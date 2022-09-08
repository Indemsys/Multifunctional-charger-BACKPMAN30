#include   "App.h"
#include   "Flash_media_driver.h"
#include   "STfs_int.h"
#include   "STfs_api.h"
#include   "STfs_tests.h"
#include   <stdlib.h>
#include   <stdio.h>

T_stfs_test_cbl  *p_test_cbl;

static uint32_t   crc_table[256];

/*-----------------------------------------------------------------------------------------------------
  Name  : CRC-32
  Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  Revert: true
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789")
  MaxLen: 268 435 455 Byte


  \param buf
  \param len

  \return unsigned int
-----------------------------------------------------------------------------------------------------*/
static void CRC32_prepare(void)
{
  uint32_t  crc;
  uint32_t  i, j;
  for (i = 0; i < 256; i++)
  {
    crc = i;
    for (j = 0; j < 8; j++) crc = crc & 1 ?(crc >> 1)^ 0xEDB88320UL : crc >> 1;

    crc_table[i] = crc;
  };
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t CRC32_start(void)
{
  return  0xFFFFFFFFUL;
}

/*-----------------------------------------------------------------------------------------------------


  \param in_crc

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t CRC32_finish(uint32_t in_crc)
{
  return in_crc ^ 0xFFFFFFFFUL;
}
/*-----------------------------------------------------------------------------------------------------


  \param in_crc
  \param buf
  \param len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t CRC32_eval(uint32_t in_crc, uint8_t *buf, uint32_t len)
{
  while (len--) in_crc = crc_table[(in_crc ^*buf++) & 0xFF] ^(in_crc >> 8);
  return in_crc;
}

/*-----------------------------------------------------------------------------------------------------
  Установить контрольную структуру для теста

  \param test_cbl
-----------------------------------------------------------------------------------------------------*/
void STfs_init_test(T_stfs_test_cbl *test_cbl)
{
  p_test_cbl = test_cbl;
}

/*-----------------------------------------------------------------------------------------------------


  \param tstmp1
  \param tstmp2
  \param p_minv
  \param p_maxv
-----------------------------------------------------------------------------------------------------*/
void STfs_test_eval_statistic(T_sys_timestump   *tstmp1, T_sys_timestump   *tstmp2, uint64_t *p_minv, uint64_t *p_maxv )
{
  uint64_t dt = Hw_timestump_diff64_us(tstmp1, tstmp2);
  if (*p_minv ==0)
  {
    *p_minv = dt;
  }
  else if (*p_minv > dt)
  {
    *p_minv = dt;
  }

  if (*p_maxv ==0)
  {
    *p_maxv = dt;
  }
  else if (*p_maxv < dt)
  {
    *p_maxv = dt;
  }
}
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void STfs_test_check(uint8_t disk_id)
{
  int32_t           res;
  T_stfs_info       pstat;
  uint32_t          free_space;

  res = STfs_check(disk_id,&pstat);
  if (res != STFS_OK)
  {
    APPLOG("STfs check error %d.", res);
    return;
  }
  STfs_free_space(disk_id,&free_space);
  APPLOG("STfs state: Media size=%d, Files count=%d, Valid aria size=%d, Invalid aria size=%d, Empty area size=%d, Free space=%d ", pstat.media_size, pstat.file_count, pstat.valid_aria_size, pstat.invalid_aria_size, pstat.empty_aria_size, free_space);

}


/*-----------------------------------------------------------------------------------------------------
   В ходе теста:
    - Открываем файл на запись
    - Записываем туда фрагменты случайного размера со случайным содержимым.
       Если фрагментация разрешена, то выполняем функцию STfs_flush после каждой записи фрагмента.
       Это приводит к созданию дополнительных дескрипторов и фрагментированию файла.
    - Закрываем файл.
    - Открываем на чтение и читаем файл случайными фрагментами и сравниваем с тем что должно было быть записанно.
       Сравнение производится по 4-байтной контрольной сумме в конце файла
    - Стираем файл если стирание разрешено.


  \param disk_id        илентификатор диска
  \param iter_number    номер итерации
  \param rand_number    случайное число
  \param en_deleting    если не 0 то созданный файл стереть при выходе из функции
  \param fragm_flag     если не 0 то вызывать функцию STfs_flush после записи каждого фрагмента

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_test_write_read_delete(uint8_t disk_id, uint32_t iter_number, uint32_t rand_number, uint8_t en_deleting, uint8_t fragm_flag)
{

  int32_t           file;
  int32_t           res;
  int32_t           left_bytes;
  uint8_t           *chunk_buf;
  uint8_t           *file_buf;
  char              file_name[STFS_FLASH_WORD_SIZE+1];
  uint32_t          file_number;
  volatile uint32_t fsize;
  volatile uint32_t chunk_size;
  uint32_t          seed;
  T_file_info       file_info;
  uint8_t           *ptr;
  uint32_t          rand_max = RAND_MAX;
  T_stfs_info       pstat;
  uint32_t          free_space;
  uint32_t          file_crc;
  int32_t           actual_read;
  uint32_t          chunk_cnt;
  uint32_t          max_estim_size = 0;
  T_sys_timestump   tstmp1;
  T_sys_timestump   tstmp2;


  chunk_buf = App_malloc(p_test_cbl->max_file_size);
  if (chunk_buf == 0)
  {
    APPLOG("Memory allocation error.");
    return STFS_ERROR;
  }
  file_buf = App_malloc(p_test_cbl->max_file_size + 4); // Буфер для файла и CRC
  if (file_buf == 0)
  {
    APPLOG("Memory allocation error.");
    App_free(chunk_buf);
    return STFS_ERROR;
  }

  CRC32_prepare();

  // Проверяем файловую систему
  res =  STfs_check(disk_id,&pstat);
  if (res != STFS_OK)
  {
    APPLOG("STfs check error %d.", res);
    goto error_exit;
  }
  STfs_free_space(disk_id,&free_space);
  APPLOG("STfs state: Media size=%d, Files count=%d, Valid aria size=%d, Invalid aria size=%d, Empty area size=%d, Free space=%d ", pstat.media_size, pstat.file_count, pstat.valid_aria_size, pstat.invalid_aria_size, pstat.empty_aria_size, free_space);

  seed = 12343 + rand_number;
  file_number = iter_number;

  srand(seed);

  // Получаем случайный размер файла
  fsize = rand();
  fsize =((uint64_t)fsize * (p_test_cbl->max_file_size - p_test_cbl->min_file_size)) / (uint64_t)rand_max + p_test_cbl->min_file_size;

  // Проверит есть ли место для нового файла
  // Предусматриваем оценочно для худшего случая:
  // - дополнительно запись p_test_cbl->max_num_chunks+8 дескрипторов
  // - запись 4 байт контрольной суммы
  // - запись дескриптора имени файла и блока с именем файла
  // - запас на блоки с размером не кратным STFS_FLASH_WORD_SIZE
  if (fragm_flag)
  {
    max_estim_size = fsize +(sizeof(T_stfs_file_descriptor) * (p_test_cbl->max_num_chunks+8))+ sizeof(file_crc)+ sizeof(T_stfs_file_descriptor)+ STFS_FLASH_WORD_SIZE + STFS_FLASH_WORD_SIZE * p_test_cbl->max_num_chunks;
  }
  else
  {
    max_estim_size = fsize +(sizeof(T_stfs_file_descriptor) * 8)+ sizeof(file_crc)+ sizeof(T_stfs_file_descriptor)+ STFS_FLASH_WORD_SIZE + STFS_FLASH_WORD_SIZE;
  }
  if (max_estim_size > pstat.empty_aria_size)
  {
    uint32_t indx;
    // Дефрагментируем файловую систему
    res = STfs_defrag(disk_id);
    if (res == STFS_OK)
    {
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      APPLOG(" Defragmentation was done successfully  ");
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }
    else
    {
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      APPLOG(" Defragmentation error %d", res);
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

      do
      {
        // Пытаемся стирать файлы и освободить место для дефрагментаии и записи файла
        if (STfs_find(disk_id,"*",&file_info) == STFS_OK)
        {
          STfs_delete(disk_id, file_info.name);
          APPLOG("File %s deleting done.", file_info.name);
        }
        else
        {
          goto error_exit;
        }
        res =  STfs_check(disk_id,&pstat);
        if (res != STFS_OK)
        {
          APPLOG("STfs check error %d.", res);
          goto error_exit;
        }

        if (STfs_defrag(disk_id) == STFS_OK)
        {
          res =  STfs_check(disk_id,&pstat);
          if (res != STFS_OK)
          {
            APPLOG("STfs check error %d.", res);
            goto error_exit;
          }
          if ((fsize + sizeof(T_stfs_file_descriptor) * 100 + sizeof(file_crc)) < pstat.empty_aria_size) break;
        }

      } while (1);
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      APPLOG(" Defragmentation was done successfully  ");
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    }

    // Проверяем целостность файловой системы
    res =  STfs_check(disk_id,&pstat);
    if (res != STFS_OK)
    {
      APPLOG("STfs check error %d.", res);
      goto error_exit;
    }
    STfs_free_space(disk_id,&free_space);
    APPLOG("STfs state: Media size=%d, Files count=%d, Valid aria size=%d, Invalid aria size=%d, Empty area size=%d, Free space=%d ", pstat.media_size, pstat.file_count, pstat.valid_aria_size, pstat.invalid_aria_size, pstat.empty_aria_size, free_space);

    uint8_t crc_err = 0;
    // Находим все валидные файлы и проверяем их целостность
    if (STfs_find_first_file(disk_id,&indx,&file_info) == STFS_OK)
    {
      do
      {
        res = STfs_open(disk_id,file_info.name, STFS_OPEN_READ,&file);
        if (res != STFS_OK)
        {
          APPLOG("File %s opening for read error = %d.",file_info.name, res);
          goto error_exit;
        }

        res = STfs_read(file, file_buf, file_info.size - sizeof(file_crc),&actual_read);
        if (res != STFS_OK)
        {
          APPLOG("File %s read error = %d.", file_info.name, res);
          goto error_exit;
        }
        file_crc = CRC32_start();
        file_crc = CRC32_eval(file_crc, file_buf, file_info.size - sizeof(file_crc));
        file_crc = CRC32_finish(file_crc);

        res = STfs_read(file, file_buf, sizeof(file_crc),&actual_read);
        if (res != STFS_OK)
        {
          APPLOG("File %s read error = %d.", file_info.name, res);
          goto error_exit;
        }

        char *str;
        if (file_crc != *(uint32_t *)file_buf)
        {
          str = "ERROR";
          crc_err = 1;
        }
        else
        {
          str = "OK";
        }
        APPLOG("File %s was found. sector = %d, version = %d, file_id = %d, Size = %d, CRC= %s", file_info.name, file_info.file_name_sector, file_info.ver, file_info.file_id, file_info.size, str);


        res = STfs_close(file);
        if (res != STFS_OK)
        {
          APPLOG("File %s close error.", file_name);
          goto error_exit;
        }

      } while (STfs_find_next_file(indx,&file_info) == STFS_OK);
    }

    if (crc_err != 0)
    {
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      APPLOG("File integrity violation detected.");
      APPLOG("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      goto error_exit;
    }

  }

  chunk_cnt = 0;
  snprintf(file_name, STFS_FLASH_WORD_SIZE, "File%d", file_number);
  Get_hw_timestump(&tstmp1);
  res = STfs_open(disk_id,file_name, STFS_OPEN_WRITE,&file);
  Get_hw_timestump(&tstmp2);
  STfs_test_eval_statistic(&tstmp1,&tstmp2, &(p_test_cbl->min_open_time),&(p_test_cbl->max_open_time));

  if (res != STFS_OK)
  {
    APPLOG("File %s opening for write error = %d.",file_name, res);
    goto error_exit;
  }
  APPLOG("File %s opened for write successfully. Size to write = %d, Estimated needed space = %d",file_name, fsize, max_estim_size);

  // Записываем в файл случайные блоки данных
  left_bytes = fsize;
  ptr = file_buf;
  file_crc = CRC32_start();
  while (left_bytes != 0)
  {
    if (chunk_cnt >= (p_test_cbl->max_num_chunks-1))
    {
      chunk_size = left_bytes;
    }
    else
    {
      chunk_size = rand();
      chunk_size =(chunk_size * (p_test_cbl->max_file_chunk - p_test_cbl->min_file_chunk)) / rand_max + p_test_cbl->min_file_chunk;
      if (chunk_size > left_bytes) chunk_size = left_bytes;
    }

    for (uint32_t i=0; i < chunk_size; i++)
    {
      chunk_buf[i] = rand();
      *ptr = chunk_buf[i];
      ptr++;
    }
    file_crc = CRC32_eval(file_crc, chunk_buf, chunk_size);

    Get_hw_timestump(&tstmp1);
    res = STfs_write(file, chunk_buf, chunk_size);
    Get_hw_timestump(&tstmp2);
    STfs_test_eval_statistic(&tstmp1,&tstmp2,&(p_test_cbl->min_write_time),&(p_test_cbl->max_write_time));

    if (res != STFS_OK)
    {
      APPLOG("File %s write error = %d. Chunk num = %d, Chunk size = %d, Left bytes = %d", file_name, res, chunk_cnt, chunk_size, left_bytes);
      res = STfs_close(file);
      if (res == STFS_OK)
      {
        APPLOG("File closed Ok");
        res= STfs_find(disk_id,file_name,&file_info);
        if (res != STFS_OK)
        {
          APPLOG("Find file %s error.");
          goto error_exit;
        }
        APPLOG("File %s was found. ID = %d, Size = %d",file_name, file_info.file_id, file_info.size);
        res = STfs_delete(disk_id, file_name);
        if (res != STFS_OK)
        {
          APPLOG("File %s delete error.", file_name);
          goto error_exit;
        }
        APPLOG("File %s deleting done.", file_name);
        APPLOG("#################################");

        goto error_exit;
      }
      else
      {
        APPLOG("File close error %d", res);
      }
      goto error_exit;
    }

    if (fragm_flag != 0)
    {
      res = STfs_flush(file);
      if (res != STFS_OK)
      {
        APPLOG("File %s flush error = %d.Chunk num = %d, Chunk size = %d, Left bytes = %d", file_name, res, chunk_cnt, chunk_size, left_bytes);
        res = STfs_close(file);
        if (res == STFS_OK)
        {
          APPLOG("File closed Ok");
        }
        else
        {
          APPLOG("File close error %d", res);
        }
        goto error_exit;
      }
    }

    left_bytes = left_bytes - chunk_size;
    chunk_cnt++;
  }
  // Записываем CRC в конце файла
  file_crc = CRC32_finish(file_crc);
  res = STfs_write(file,(uint8_t *)&file_crc, sizeof(file_crc));
  if (res != STFS_OK)
  {
    APPLOG("File %s crc write error = %d.", file_name, res);
    STfs_close(file);
    goto error_exit;
  }
  fsize += sizeof(file_crc);

  res = STfs_close(file);
  if (res != STFS_OK)
  {
    APPLOG("File %s close error.", file_name);
    goto error_exit;
  }
  APPLOG("File %s closing done. Size = %d", file_name, fsize);

  // Читаем обратно из файла случайные блоки данных

  // Сначала файл находим
  Get_hw_timestump(&tstmp1);
  res= STfs_find(disk_id,file_name,&file_info);
  Get_hw_timestump(&tstmp2);
  STfs_test_eval_statistic(&tstmp1,&tstmp2,&(p_test_cbl->min_find_time),&(p_test_cbl->max_find_time));

  if (res != STFS_OK)
  {
    APPLOG("Find file %s error.");
    goto error_exit;
  }
  APPLOG("File %s was found. ID = %d, Size = %d",file_name, file_info.file_id, file_info.size);

  if (file_info.size != fsize)
  {
    APPLOG("File size mismatch. In memory = %d, in media = %d", fsize, file_info.size);
    goto error_exit;
  }

  res = STfs_open(disk_id,file_name, STFS_OPEN_READ,&file);
  if (res != STFS_OK)
  {
    APPLOG("File %s opening for read error = %d.",file_name, res);
    goto error_exit;
  }
  APPLOG("File %s opened for read successfully.",file_name);

  // Читаем из файла блоками случайной длины
  left_bytes = fsize - sizeof(file_crc);
  ptr = file_buf;
  file_crc = CRC32_start();
  while (left_bytes != 0)
  {
    chunk_size = rand();
    chunk_size =(chunk_size * (p_test_cbl->max_file_chunk - p_test_cbl->min_file_chunk)) / RAND_MAX + p_test_cbl->min_file_chunk;
    if (chunk_size > left_bytes) chunk_size = left_bytes;
    Get_hw_timestump(&tstmp1);
    res = STfs_read(file, chunk_buf, chunk_size,&actual_read);
    Get_hw_timestump(&tstmp2);
    STfs_test_eval_statistic(&tstmp1,&tstmp2,&(p_test_cbl->min_read_time),&(p_test_cbl->max_read_time));

    if (res != STFS_OK)
    {
      APPLOG("File %s read error = %d.", file_name, res);
      goto error_exit;
    }
    file_crc = CRC32_eval(file_crc, chunk_buf, chunk_size);
    for (uint32_t i=0; i < chunk_size; i++)
    {
      if (chunk_buf[i] != *ptr)
      {
        APPLOG("File content mismatch. In memory %02X, in media %02X ",*ptr, chunk_buf[i]);
        goto error_exit;
      }
      ptr++;
    }
    left_bytes = left_bytes - chunk_size;
  }
  file_crc = CRC32_finish(file_crc);

  res = STfs_read(file, chunk_buf, sizeof(file_crc),&actual_read);
  if (res != STFS_OK)
  {
    APPLOG("File %s read error = %d.", file_name, res);
    goto error_exit;
  }

  if (*((uint32_t *)chunk_buf) != file_crc)
  {
    APPLOG("File %s CRC error.", file_name);
    goto error_exit;
  }

  APPLOG("File %s integrity check was successful.",file_name);
  res = STfs_close(file);
  if (res != STFS_OK)
  {
    APPLOG("File %s close error.", file_name);
    goto error_exit;
  }
  APPLOG("File %s reading done. Size = %d", file_name, fsize);

  // Проверяем файловую систему
  Get_hw_timestump(&tstmp1);
  res =  STfs_check(disk_id,&pstat);
  Get_hw_timestump(&tstmp2);
  STfs_test_eval_statistic(&tstmp1,&tstmp2,&(p_test_cbl->min_check_time),&(p_test_cbl->max_check_time));

  if (res != STFS_OK)
  {
    APPLOG("STfs check error %d.", res);
    goto error_exit;
  }
  STfs_free_space(disk_id,&free_space);
  APPLOG("STfs state: Media size=%d, Files count=%d, Valid aria size=%d, Invalid aria size=%d, Empty area size=%d, Free space=%d ", pstat.media_size, pstat.file_count, pstat.valid_aria_size, pstat.invalid_aria_size, pstat.empty_aria_size, free_space);

  if (en_deleting)
  {
    res = STfs_delete(disk_id, file_name);
    if (res != STFS_OK)
    {
      APPLOG("File %s delete error.", file_name);
      goto error_exit;
    }
    APPLOG("File %s deleting done.", file_name);
  }


  App_free(chunk_buf);
  App_free(file_buf);
  return STFS_OK;
error_exit:
  App_free(chunk_buf);
  App_free(file_buf);
  return STFS_ERROR;

}

