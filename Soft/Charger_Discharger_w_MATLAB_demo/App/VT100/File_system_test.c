#include "App.h"


#define STRINGS_SZ 64


typedef struct
{
    int    fd;
    uint32_t open_t;
    uint32_t rw_t;
    uint32_t close_t;
}
T_file_stat_rec;


#define MAX_FILE_BUF_SIZE  (32768)
#define MAX_FILE_BUF       (1024*1024*1024)

uint8_t *ptr1;
uint8_t *ptr2;

static  char              *fname_tmpl;
static  char              *filename;
static  char              *str;
static  T_sys_timestump    t1, t2;




typedef struct
{
    unsigned int  files_cnt;
    unsigned int  file_sz;
    unsigned char write_test;
    unsigned char info;

} T_fs_test;


/*-------------------------------------------------------------------------------------------------------------
 Последовательно записываем количество файлов заданное в cbl->files_cnt  размером заданным в cbl->file_sz
-------------------------------------------------------------------------------------------------------------*/
static int   FS_test_write_N_files(T_fs_test *test)
{
  FX_FILE              f;
  uint32_t             res;

  T_file_stat_rec     *ftm;

  int                  i,  num;
  uint8_t              *bptr;

  uint32_t max_open_t  = 0;
  uint32_t min_open_t  = 0xFFFFFFFF;
  uint32_t max_rw_t    = 0;
  uint32_t min_rw_t    = 0xFFFFFFFF;
  uint32_t max_close_t = 0;
  uint32_t min_close_t = 0xFFFFFFFF;

  float    avr_open_t = 0;
  float    avr_rw_t = 0;
  float    avr_close_t = 0;

  uint8_t  *file_buf;
  uint32_t  file_buf_size;

  GET_MCBL;

  if (test->file_sz > MAX_FILE_BUF_SIZE)
  {
    file_buf_size = MAX_FILE_BUF_SIZE;
  }
  else
  {
    file_buf_size = test->file_sz;
  }

  // Выделяем память для массивов
  file_buf = App_malloc(file_buf_size);
  if (file_buf == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }

  // Заполняем буфер случайными числами
  num = 0;
  srand(num);
  bptr = (uint8_t *)file_buf;
  for (i = 0; i < file_buf_size; i++)
  {
    *bptr = rand();
    bptr++;
  }


  ftm = (T_file_stat_rec *)App_malloc(test->files_cnt * sizeof(T_file_stat_rec));
  if (ftm == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for array of statistic!\r\n");
    goto exit_;
  }

  str = (char *)App_malloc(STRINGS_SZ * 3);
  if (ftm == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  fname_tmpl =&str[STRINGS_SZ];
  filename   =&str[STRINGS_SZ * 2];

  strcpy(fname_tmpl, "testf");

  // Создаем и записываем количество файлов заданное переменной filecount
  MPRINTF("-------------------- Writing -----------------\n\r");
  for (num = 0; num < test->files_cnt; num++)
  {


    sprintf(filename, "%s%d.BIN", fname_tmpl, num);


    Get_hw_timestump(&t1);
    res = Recreate_file_for_write(&f, (CHAR *)filename);
    Get_hw_timestump(&t2);
    ftm[num].open_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("File %s open to write error %d\n\r", filename, res);
      continue;
    }

    Get_hw_timestump(&t1);
    {
      uint32_t sz = test->file_sz;
      uint32_t wr_buf_sz = file_buf_size;
      do
      {
        res = fx_file_write(&f, file_buf, wr_buf_sz);
        if (res != FX_SUCCESS) break;
        sz = sz - wr_buf_sz;
        if (sz < wr_buf_sz) wr_buf_sz = sz;
      } while (sz > 0);
    }
    Get_hw_timestump(&t2);
    ftm[num].rw_t = Hw_timestump_diff32_us(&t1,&t2);

    Get_hw_timestump(&t1);
    fx_file_close(&f);
    Get_hw_timestump(&t2);
    ftm[num].close_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("%s Write error. Size = %06d, writen = %d\n\r", filename, test->file_sz, res);
    }
    else
    {
      MPRINTF("%s %06d %08d %08d %08d %.0f Kb/s\n\r", filename, test->file_sz, ftm[num].open_t, ftm[num].rw_t, ftm[num].close_t,  test->file_sz * 1000000.0 / ((float)ftm[num].rw_t * 1024.0));
    }
  }


  fx_media_flush(&fat_fs_media);
  // Подсчет статистики
  for (num = 0; num < test->files_cnt; num++)
  {

    if (ftm[num].open_t > max_open_t) max_open_t = ftm[num].open_t;
    if (ftm[num].open_t < min_open_t) min_open_t = ftm[num].open_t;
    if (ftm[num].rw_t > max_rw_t) max_rw_t = ftm[num].rw_t;
    if (ftm[num].rw_t < min_rw_t) min_rw_t = ftm[num].rw_t;
    if (ftm[num].close_t > max_close_t) max_close_t = ftm[num].close_t;
    if (ftm[num].close_t < min_close_t) min_close_t = ftm[num].close_t;

    avr_open_t  += ftm[num].open_t;
    avr_rw_t    += ftm[num].rw_t;
    avr_close_t += ftm[num].close_t;
  }

  MPRINTF("Statistic for write operations.\n\r");
  MPRINTF("Open max = %08d us Open min = %08d us, Open aver = %.0f us\n\r", max_open_t, min_open_t,  (float)avr_open_t / (float)test->files_cnt);
  MPRINTF("Writ max = %08d us Writ min = %08d us, Writ aver = %.0f us\n\r", max_rw_t, min_rw_t,  (float)avr_rw_t / (float)test->files_cnt);
  MPRINTF("Clos max = %08d us Clos min = %08d us, Clos aver = %.0f us\n\r", max_close_t, min_close_t,  (float)avr_close_t / (float)test->files_cnt);
  {
    float t =(float)avr_rw_t / (float)test->files_cnt;
    MPRINTF("Averaged write speed  = %.0f Kbyte/s\n\r", test->file_sz * 1000000.0 / (t * 1024.0));
  }

exit_:
  if (file_buf)  App_free(file_buf);
  if (ftm)   App_free(ftm);
  if (str)   App_free(str);
  return 0;
}

/*-------------------------------------------------------------------------------------------------------------
 Последовательно читаем количество файлов заданное в cbl->files_cnt  размером заданным в cbl->file_sz
-------------------------------------------------------------------------------------------------------------*/
static int   FS_test_read_N_files(T_fs_test *test)
{
  FX_FILE              f;
  uint32_t             res;
  ULONG                actual_size;

  T_file_stat_rec     *ftm;

  int                  err_cnt;
  int                  i,  num;
  uint8_t              *bptr;

  uint32_t max_open_t  = 0;
  uint32_t min_open_t  = 0xFFFFFFFF;
  uint32_t max_rw_t    = 0;
  uint32_t min_rw_t    = 0xFFFFFFFF;
  uint32_t max_close_t = 0;
  uint32_t min_close_t = 0xFFFFFFFF;

  float    avr_open_t = 0;
  float    avr_rw_t = 0;
  float    avr_close_t = 0;

  uint8_t  *file_buf;
  uint8_t  *file_buf2;
  uint32_t  file_buf_size;

  uint32_t  td;
  GET_MCBL;

  if (test->file_sz > MAX_FILE_BUF_SIZE)
  {
    file_buf_size = MAX_FILE_BUF_SIZE;
  }
  else
  {
    file_buf_size = test->file_sz;
  }

  // Выделяем память для массивов
  file_buf = App_malloc(file_buf_size);
  if (file_buf == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }
  file_buf2 = App_malloc(file_buf_size);
  if (file_buf2 == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for file second buffer!\r\n");
    goto exit_;
  }

  // Заполняем буфер случайными числами
  num = 0;
  srand(num);
  bptr = (uint8_t *)file_buf;
  for (i = 0; i < file_buf_size; i++)
  {
    *bptr = rand();
    bptr++;
  }


  ftm = (T_file_stat_rec *)App_malloc(test->files_cnt * sizeof(T_file_stat_rec));
  if (ftm == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for array of statistic!\r\n");
    goto exit_;
  }

  str = (char *)App_malloc(STRINGS_SZ * 3);
  if (ftm == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  fname_tmpl =&str[STRINGS_SZ];
  filename   =&str[STRINGS_SZ * 2];


  strcpy(fname_tmpl, "testf");

  // Читаем количество файлов заданное переменной filecount
  MPRINTF("-------------------- Reading -----------------\n\r");
  for (num = 0; num < test->files_cnt; num++)
  {

    sprintf(filename, "%s%d.BIN", fname_tmpl, num);

    Get_hw_timestump(&t1);
    res = fx_file_open(&fat_fs_media,&f, filename,(FX_OPEN_FOR_READ));
    Get_hw_timestump(&t2);
    ftm[num].open_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("File %s open to read error!\n\r", filename);
      break;
    }

    td = 0;
    {
      uint32_t sz = test->file_sz;
      uint32_t rd_buf_sz = file_buf_size;
      do
      {
        Get_hw_timestump(&t1);
        res = fx_file_read(&f,file_buf2, rd_buf_sz,&actual_size);
        Get_hw_timestump(&t2);
        td += Hw_timestump_diff32_us(&t1,&t2);

        if (res != FX_SUCCESS)
        {
          MPRINTF("File %s read error %d!\n\r", filename,res);
          break;
        }
        if  (actual_size != rd_buf_sz)
        {
          MPRINTF("File %s read insufficient data. Actual size = %d but must be %d!\n\r", filename,actual_size, rd_buf_sz);
          break;
        }

        // Сравниваем эталонные и прочитанные данные
        err_cnt = 0;
        for (i = 0; i < rd_buf_sz; i++)
        {
          uint8_t b,b_e;
          b = file_buf2[i];
          b_e = file_buf[i];
          if (b != b_e)
          {
            MPRINTF("File %s has data error at offset %08X (read -> %02X ,must be -> %02X)\n\r", filename, i,b, b_e);
            err_cnt++;
            if (err_cnt > 32) break;
          }
          bptr++;
        }
        if (err_cnt > 0) break;

        sz = sz - rd_buf_sz;
        if (sz < rd_buf_sz) rd_buf_sz = sz;

      } while (sz > 0);
    }
    ftm[num].rw_t = td;

    Get_hw_timestump(&t1);
    fx_file_close(&f);
    Get_hw_timestump(&t2);
    ftm[num].close_t = Hw_timestump_diff32_us(&t1,&t2);

    MPRINTF("%s %06d %08d %08d %08d %.0f Kb/s\n\r", filename, test->file_sz, ftm[num].open_t, ftm[num].rw_t, ftm[num].close_t,  test->file_sz * 1000000.0 / ((float)ftm[num].rw_t * 1024.0));

  }

  max_open_t  = 0;
  min_open_t  = 0xFFFFFFFF;
  max_rw_t    = 0;
  min_rw_t    = 0xFFFFFFFF;
  max_close_t = 0;
  min_close_t = 0xFFFFFFFF;

  avr_open_t = 0;
  avr_rw_t = 0;
  avr_close_t = 0;

  // Подсчет статистики
  for (num = 0; num < test->files_cnt; num++)
  {

    if (ftm[num].open_t > max_open_t) max_open_t = ftm[num].open_t;
    if (ftm[num].open_t < min_open_t) min_open_t = ftm[num].open_t;
    if (ftm[num].rw_t > max_rw_t) max_rw_t = ftm[num].rw_t;
    if (ftm[num].rw_t < min_rw_t) min_rw_t = ftm[num].rw_t;
    if (ftm[num].close_t > max_close_t) max_close_t = ftm[num].close_t;
    if (ftm[num].close_t < min_close_t) min_close_t = ftm[num].close_t;

    avr_open_t  += ftm[num].open_t;
    avr_rw_t    += ftm[num].rw_t;
    avr_close_t += ftm[num].close_t;
  }

  MPRINTF("Statistic for read operations.\n\r");
  MPRINTF("Open max = %08d us Open min = %08d us, Open aver = %.0f us\n\r", max_open_t, min_open_t,  (float)avr_open_t / (float)test->files_cnt);
  MPRINTF("Read max = %08d us Read min = %08d us, Read aver = %.0f us\n\r", max_rw_t, min_rw_t,  (float)avr_rw_t / (float)test->files_cnt);
  MPRINTF("Clos max = %08d us Clos min = %08d us, Clos aver = %.0f us\n\r", max_close_t, min_close_t,  (float)avr_close_t / (float)test->files_cnt);
  {
    float t =(float)avr_rw_t / (float)test->files_cnt;
    MPRINTF("Averaged read speed  = %.0f Kbyte/s\n\r", test->file_sz * 1000000.0 / (t * 1024.0));
  }
  fx_media_flush(&fat_fs_media);

exit_:
  if (file_buf)  App_free(file_buf);
  if (file_buf2) App_free(file_buf2);
  if (ftm)   App_free(ftm);
  if (str)   App_free(str);
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param test

  \return int
-----------------------------------------------------------------------------------------------------*/
static int   FS_test_delete_N_files(T_fs_test *test)
{
  uint32_t             res;

  T_file_stat_rec     *ftm;

  int                  num;

  uint32_t max_open_t  = 0;
  uint32_t min_open_t  = 0xFFFFFFFF;
  float    avr_open_t = 0;


  GET_MCBL;

  ftm = (T_file_stat_rec *)App_malloc(test->files_cnt * sizeof(T_file_stat_rec));
  if (ftm == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for array of statistic!\r\n");
    goto exit_;
  }

  str = (char *)App_malloc(STRINGS_SZ * 3);
  if (ftm == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  fname_tmpl =&str[STRINGS_SZ];
  filename   =&str[STRINGS_SZ * 2];

  strcpy(fname_tmpl, "testf");

  // Удаляем количество файлов заданное переменной filecount
  MPRINTF("-------------------- Deleting -----------------\n\r");

  for (num = 0; num < test->files_cnt; num++)
  {
    sprintf(filename, "%s%d.BIN", fname_tmpl, num);

    Get_hw_timestump(&t1);
    res = fx_file_delete(&fat_fs_media,filename);
    Get_hw_timestump(&t2);
    ftm[num].open_t = Hw_timestump_diff32_us(&t1,&t2);

    MPRINTF("%s %08d us. Result=%d\n\r",filename, ftm[num].open_t, res);

  }

  max_open_t  = 0;
  min_open_t  = 0xFFFFFFFF;

  avr_open_t = 0;

  // Подсчет статистики
  for (num = 0; num < test->files_cnt; num++)
  {

    if (ftm[num].open_t > max_open_t) max_open_t = ftm[num].open_t;
    if (ftm[num].open_t < min_open_t) min_open_t = ftm[num].open_t;

    avr_open_t  += ftm[num].open_t;
  }

  MPRINTF("Statistic for delete operations.\n\r");
  MPRINTF("Delet. max = %08d us Delet. min = %08d us, Delet. aver = %.0f us\n\r", max_open_t, min_open_t,  (float)avr_open_t / (float)test->files_cnt);

exit_:
  if (ftm)   App_free(ftm);
  if (str)   App_free(str);
  return 0;
}

/*------------------------------------------------------------------------------
  Записываем файл, проверяем его содержание, стираем

 \param cbl

 \return int
 ------------------------------------------------------------------------------*/
static int   FS_test2(T_fs_test *test)
{
  FX_FILE              f;
  uint32_t             res;
  ULONG                actual_size;

  int                  i;
  int                  num;

  unsigned int         open_w_t;  // Время открытия файла на запись
  unsigned int         write_t;   // Время записи файла
  unsigned int         close_w_t; // Время закрытия файла на запись

  unsigned int         open_r_t;  // Время открытия файла на запись
  unsigned int         read_t;    // Время записи файла
  unsigned int         close_r_t; // Время закрытия файла на запись

  unsigned int         delete_t;  // Время удаления файла

  uint8_t              *file_buf;
  uint8_t              *file_buf2;

  GET_MCBL;



  // Выделяем память для массивов
  file_buf = App_malloc(test->file_sz);
  if (file_buf == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }
  file_buf2 = App_malloc(test->file_sz);
  if (file_buf2 == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for file second buffer!\r\n");
    goto exit_;
  }

  str = (char *)App_malloc(STRINGS_SZ * 3);
  if (str == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  fname_tmpl =&str[STRINGS_SZ];
  filename   =&str[STRINGS_SZ * 2];

  strcpy(fname_tmpl, "F_");

  MPRINTF("\r\n--------- File System test 2 ---------\r\n");


  for (num = 0; num < test->files_cnt; num++)
  {

    srand(num);

    // Создаем имя файла
    sprintf(filename, "%07d.BIN", num);



    // Создаем файл
    Get_hw_timestump(&t1);
    res = Recreate_file_for_write(&f, (CHAR *)filename);
    Get_hw_timestump(&t2);
    open_w_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nFile opening for write error %d!\r\n", res);
      goto exit_;
    }
    // Заполняем буфер случайными числами
    for (i = 0; i < test->file_sz; i++)
    {
      file_buf[i] = rand();
    }


    // Записываем файл
    Get_hw_timestump(&t1);
    res = fx_file_write(&f, file_buf, test->file_sz);
    Get_hw_timestump(&t2);
    write_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nWrite size error (%d)!\r\n", res);
      goto exit_;
    }

    // Закрываем файл
    Get_hw_timestump(&t1);
    fx_file_close(&f);
    Get_hw_timestump(&t2);
    close_w_t = Hw_timestump_diff32_us(&t1,&t2);

    // Открываем файл на чтение
    Get_hw_timestump(&t1);
    res = fx_file_open(&fat_fs_media,&f, filename,(FX_OPEN_FOR_READ));
    Get_hw_timestump(&t2);
    open_r_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nFile opening for read error!\r\n");
      goto exit_;
    }


    // Читаем файл
    Get_hw_timestump(&t1);
    res = fx_file_read(&f,file_buf2, test->file_sz,&actual_size);
    Get_hw_timestump(&t2);
    read_t = Hw_timestump_diff32_us(&t1,&t2);

    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nRead size error (%d)!\r\n", res);
      goto exit_;
    }
    if (actual_size != test->file_sz)
    {
      MPRINTF("File %s read insufficient data. Actual size = %d but must be %d!\n\r", filename,actual_size, test->file_sz);
      goto exit_;
    }


    // Закрываем файл
    Get_hw_timestump(&t1);
    fx_file_close(&f);
    Get_hw_timestump(&t2);
    close_r_t = Hw_timestump_diff32_us(&t1,&t2);


    Get_hw_timestump(&t1);
    res = fx_file_delete(&fat_fs_media,filename);
    Get_hw_timestump(&t2);
    delete_t = Hw_timestump_diff32_us(&t1,&t2);

    fx_media_flush(&fat_fs_media);

    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nDeleting  error (%d)!\r\n", res);
      goto exit_;
    }


    // Проверяем прочитанные данные
    for (i = 0; i < test->file_sz; i++)
    {
      if (file_buf[i] != file_buf2[i])
      {
        MPRINTF("\r\nFile error %02X = %02X at %d\r\n", file_buf[i], file_buf2[i], i);
        goto exit_;
      }
    }
    MPRINTF("%s o:%07d w:%07d c:%07d o:%07d r:%07d c:%07d d:%07d\r\n",
      filename,
      open_w_t,
      write_t,
      close_w_t,
      open_r_t,
      read_t,
      close_r_t,
      delete_t
      );

  }

exit_:
  if (file_buf)  App_free(file_buf);
  if (file_buf2) App_free(file_buf2);
  if (str)   App_free(str);
  return 0;
}

/*------------------------------------------------------------------------------
 Открываем файл и записываем туда непрерывно блоки
 количество блоков задано в cbl->files_cnt
 размер блоков задан cbl->file_sz
 Повторяем для второго файла

 \param cbl

 \return int
 ------------------------------------------------------------------------------*/
static int   FS_test3(T_fs_test *test)
{
  FX_FILE              f;
  uint32_t             res;


  int                  i,k;
  int                  num;

  unsigned int         write_t;   // Время записи файла

  uint8_t              *file_buf;


  GET_MCBL;


  // Выделяем память для массивов
  file_buf = App_malloc(test->file_sz);
  if (file_buf == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for file first buffer!\r\n");
    goto exit_;
  }

  str = (char *)App_malloc(STRINGS_SZ * 3);
  if (str == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for strings!\r\n");
    goto exit_;
  }
  fname_tmpl =&str[STRINGS_SZ];
  filename   =&str[STRINGS_SZ * 2];

  strcpy(fname_tmpl, "F_");

  MPRINTF("\r\n---------  File System test 3 ---------\r\n");

  for (num = 0; num < 2; num++)
  {
    // Создаем имя файла
    sprintf(filename, "%07d.BIN", num);
    res = Recreate_file_for_write(&f, (CHAR *)filename);
    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nFile %s opening for write error %d!\r\n", filename, res);
      goto exit_;
    }
    else
    {
      MPRINTF("\r\nStarted writing to file  %s.\r\n", filename);
    }

    srand(0);

    for (i = 0; i < test->files_cnt; i++)
    {
      // Заполняем буфер случайными числами
      for (k = 0; k < test->file_sz; k++)
      {
        file_buf[k] = rand();
      }

      // Записываем файл
      Get_hw_timestump(&t1);
      res = fx_file_write(&f, file_buf, test->file_sz);
      Get_hw_timestump(&t2);
      write_t = Hw_timestump_diff32_us(&t1,&t2);
      if (res != FX_SUCCESS)
      {
        MPRINTF("\r\nWrite size error (%d)!\r\n", res);
        goto exit_;
      }
      MPRINTF("%07d %07d us\r\n", i, write_t);

    }
    fx_file_close(&f);
    fx_media_flush(&fat_fs_media);

  }


exit_:
  if (file_buf)  App_free(file_buf);
  if (str)   App_free(str);
  return 0;

}

/*-----------------------------------------------------------------------------------------------------


  \param test

  \return int
-----------------------------------------------------------------------------------------------------*/
static int   FS_test_file_list(T_fs_test *test)
{
  UINT res;
  FX_FILE          f;
  CHAR            *entry_name;
  UINT            attributes;
  ULONG           size;
  UINT            year;
  UINT            month;
  UINT            day;
  UINT            hour;
  UINT            minute;
  UINT            second;


  GET_MCBL;

  entry_name = App_malloc_pending(FX_MAX_LONG_NAME_LEN, 10);
  if  (entry_name == NULL) goto exit_;

  MPRINTF("\r\n--------- File list ---------\r\n");

  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  while (res == FX_SUCCESS)
  {
    res = fx_file_open(&fat_fs_media,&f, entry_name,(FX_OPEN_FOR_READ));
    if (res == FX_SUCCESS)
    {
      MPRINTF("%-16s %08X %10d %04d-%02d-%02d %02d:%02d:%02d  %10lld\r\n",entry_name, attributes, size, year, month, day, hour, minute, second, f.fx_file_current_file_size);
      fx_file_close(&f);
    }
    else
    {
      MPRINTF("%-16s %08X %10d %04d-%02d-%02d %02d:%02d:%02d\r\n",entry_name, attributes, size, year, month, day, hour, minute, second);
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

exit_:
  App_free(entry_name);
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param test

  \return int
-----------------------------------------------------------------------------------------------------*/
static int   FS_test_format_disk(T_fs_test *test)
{
  uint32_t res;
  uint32_t dt;
  T_file_system_init_results *fsres;

  GET_MCBL;


  MPRINTF("\r\n--------- Format disk ---------\r\n");


  Get_hw_timestump(&t1);
  res = FS_format();
  Get_hw_timestump(&t2);
  dt = Hw_timestump_diff32_us(&t1,&t2);

  if (res == TX_SUCCESS)
  {
    MPRINTF("Format done. Duration = %d us\r\n",dt);
  }
  else
  {
    MPRINTF("Format error %04X. Duration = %d us\r\n",res, dt);
  }

  Init_exFAT();
  fsres =  Get_FS_init_res();

  MPRINTF("\r\nFile system reinitialized. Media open result:%d, Check result:%d, Detected errors=%d,  \r\n", fsres->fx_media_open_result, fsres->fs_check_error_code, fsres->fs_detected_errors);
  return 0;
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static unsigned int  Print_MFS_test_header(T_fs_test *p)
{
  UINT res;
  ULONG64 media_available_bytes;
  ULONG64 media_total_bytes;
  char    *str;

  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  FileX System Test ===\n\r");
  MPRINTF("Press 'W'- write files, '2'- write, read, delete file, '3'- write to big file by blocks\n\r");
  MPRINTF("      'R'- read  files, 'L'- show file list, 'D'- delete files, 'F'- format disk\n\r");
  MPRINTF("Files <C>nt= %d, File s<Z>. = %d\n\r", p->files_cnt, p->file_sz);
  MPRINTF("----------------------------------------------------------------------\n\r");

  media_total_bytes = Get_media_total_size();
  res = fx_media_extended_space_available(&fat_fs_media,&media_available_bytes);
  if (res == FX_SUCCESS)
  {
    MPRINTF("Media total bytes %lld, Media available bytes %lld, Occupied %lld bytes\n\r", media_total_bytes, media_available_bytes, media_total_bytes - media_available_bytes);
    if (fx_directory_default_get(&fat_fs_media,&str) == FX_SUCCESS)
    {
      if (*str != FX_NULL)
      {
        MPRINTF("Default path:  %s\n\r", str);
      }
      else
      {
        MPRINTF("Default path is root\n\r");
      }
    }
  }
  else
  {
    MPRINTF("Media total bytes %lld\n\r", media_total_bytes);
  }
  MPRINTF("----------------------------------------------------------------------\n\r");
  return 8;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void FileX_test(void)
{
  uint8_t             b;
  uint8_t             row;

  GET_MCBL;


  T_fs_test cbl;

  cbl.file_sz     = 8192;
  cbl.files_cnt   = 1000;
  row = Print_MFS_test_header(&cbl);


  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(1000)) == RES_OK)
    {
      if (b > 0)
      {
        switch (b)
        {
        case 'C':
        case 'c':
          Print_MFS_test_header(&cbl);
          Edit_uinteger_val(row + 1,&cbl.files_cnt, 1, 1000000000);
          Print_MFS_test_header(&cbl);
          break;
        case 'Z':
        case 'z':
          Print_MFS_test_header(&cbl);
          Edit_uinteger_val(row + 1,&cbl.file_sz, 1, MAX_FILE_BUF);
          Print_MFS_test_header(&cbl);
          break;
        case 'W':
        case 'w':
          cbl.info = 0;
          FS_test_write_N_files(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case 'R':
        case 'r':
          cbl.info = 0;
          FS_test_read_N_files(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case '2':
          FS_test2(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case '3':
          FS_test3(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case 'D':
        case 'd':
          FS_test_delete_N_files(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case 'F':
        case 'f':
          FS_test_format_disk(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case 'L':
        case 'l':
          FS_test_file_list(&cbl);
          MPRINTF("\n\r\n\r");
          break;
        case VT100_ESC:

          return;
        default:
          Print_MFS_test_header(&cbl);
          break;
        }
      }
    }
  }while (1);
}



#define EDIT_ROW_POS         10
#define TEST_FILE_NAME_LEN   64
typedef struct
{
    uint32_t            iter_num;
    uint32_t            curr_iter;
    uint32_t            file_size;      // Размер файла
    uint32_t            block_size;  // Размер записываемого блока
    uint32_t            stat_block_sz;  // Количество записанных данных после которого выводится статистика
    char                filename[TEST_FILE_NAME_LEN];

} T_f_wr_test;

typedef struct
{
    uint32_t            min_bl_time;
    uint32_t            max_bl_time;
    uint32_t            aver_bl_time;
    float               aver_bl_speed;
} T_f_test_results;


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
void Print_File_write_test_header(T_f_wr_test *p_fwrt)
{
  GET_MCBL;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  File read/write performance test ===\n\r");
  MPRINTF(" 'W' - write test, 'R' - read test, 'P' - print last results,  ESC - exit\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");
  MPRINTF("File <S>ize = %d, wr.<B>lock size = %d, s<T>at.block size= %d\n\r",  p_fwrt->file_size, p_fwrt->block_size, p_fwrt->stat_block_sz);
  MPRINTF("<I>teratin bumber = %d, File <N>ame = %s\n\r",p_fwrt->iter_num, p_fwrt->filename);
  MPRINTF("----------------------------------------------------------------------\n\r");
}

/*-----------------------------------------------------------------------------------------------------


  \param p_fwrt
-----------------------------------------------------------------------------------------------------*/
void File_write_perfomance_test(T_f_wr_test *p_fwrt, T_f_test_results *p_result)
{
  FX_FILE            f;
  uint32_t           tot_min_bl_wr_time      = 0xFFFFFFFF;
  uint32_t           tot_max_bl_wr_time      = 0;
  uint32_t           tot_aver_bl_wr_time_acc = 0;
  uint32_t           tot_aver_bl_wr_time_cnt = 0;

  uint32_t           min_bl_wr_time  = 0xFFFFFFFF;
  uint32_t           max_bl_wr_time  = 0;
  uint32_t           aver_bl_wr_time = 0;
  uint32_t           written_data    = 0;
  uint32_t           tacc            = 0;
  uint32_t           bl_cnt          = 0;

  uint8_t            *data_bl;
  uint32_t           res;
  T_sys_timestump    t1, t2;
  uint32_t           stat_acc = 0;

  GET_MCBL;

  // Гармонизируем установки
  if (p_fwrt->file_size < p_fwrt->block_size)
  {
    p_fwrt->block_size = p_fwrt->file_size;
  }
  if (p_fwrt->stat_block_sz > p_fwrt->file_size)
  {
    p_fwrt->stat_block_sz = p_fwrt->file_size;
  }


  // Выделяем память для массивов
  data_bl = App_malloc(p_fwrt->block_size);
  if (data_bl == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for data block!\r\n");
    goto exit_;
  }

  // Заполняем блок данных случайными данными
  srand(0);
  for (uint32_t i = 0; i < p_fwrt->block_size; i++)
  {
    data_bl[i] = rand();
  }


  res = Recreate_file_for_write(&f, (CHAR *)p_fwrt->filename);
  if (res != FX_SUCCESS)
  {
    MPRINTF("\r\nUnnable create file %s! Error %d.\r\n", p_fwrt->filename, res);
    goto exit_;
  }


  // Начинаем цикл
  MPRINTF("\r\n\r\n");

  do
  {
    uint32_t blsz;
    uint32_t tdiff;

    blsz = p_fwrt->block_size;
    if ((p_fwrt->file_size - written_data) < blsz)
    {
      blsz = p_fwrt->file_size - written_data;
    }

    Get_hw_timestump(&t1);
    res = fx_file_write(&f, data_bl, blsz);
    Get_hw_timestump(&t2);
    if (res != FX_SUCCESS)
    {
      MPRINTF("\r\nBlock write error %d!\r\n",res);
      break;
    }
    tdiff =  Hw_timestump_diff32_us(&t1,&t2);
    if (tdiff <  min_bl_wr_time)  min_bl_wr_time = tdiff;
    if (tdiff >  max_bl_wr_time)  max_bl_wr_time = tdiff;
    tacc += tdiff;
    bl_cnt++;
    written_data += blsz;
    stat_acc += blsz;

    if (stat_acc >= p_fwrt->stat_block_sz)
    {

      // Выводим статистику

      aver_bl_wr_time = tacc / bl_cnt;

      MPRINTF("iter=%03d written=%10d  min=%06d us  max=%06d us  aver=%06d us aver_speed=%9.0f Byte/s\r\n",p_fwrt->curr_iter,  written_data, min_bl_wr_time, max_bl_wr_time, aver_bl_wr_time, (float)p_fwrt->block_size * 1000000.0f / (float)aver_bl_wr_time);

      if (min_bl_wr_time <  tot_min_bl_wr_time)  tot_min_bl_wr_time = min_bl_wr_time;
      if (max_bl_wr_time >  tot_max_bl_wr_time)  tot_max_bl_wr_time = max_bl_wr_time;
      tot_aver_bl_wr_time_cnt++;
      tot_aver_bl_wr_time_acc += aver_bl_wr_time;

      tacc            = 0;
      bl_cnt          = 0;
      min_bl_wr_time  = 0xFFFFFFFF;
      max_bl_wr_time  = 0;
      aver_bl_wr_time = 0;
      stat_acc = 0;
    }

    if (written_data >= p_fwrt->file_size)
    {
      uint32_t tot_aver = tot_aver_bl_wr_time_acc / tot_aver_bl_wr_time_cnt;
      float    aver_speed = (float)p_fwrt->block_size * 1000000.0f / (float)tot_aver;
      MPRINTF("\r\n\r\nEnd.iter=%03d written=%10d tot_min=%d tot_max=%d tot_aver=%d tot_aver_speed=%9.0f Byte/s\r\n\r\n",p_fwrt->curr_iter, written_data, tot_min_bl_wr_time, tot_max_bl_wr_time, tot_aver, aver_speed);
      p_result->aver_bl_time = tot_aver;
      p_result->max_bl_time = tot_max_bl_wr_time;
      p_result->min_bl_time = tot_min_bl_wr_time;
      p_result->aver_bl_speed = aver_speed;
      break;
    }

  } while (1);


  fx_file_close(&f);
  fx_media_flush(&fat_fs_media);

exit_:
  if (data_bl)  App_free(data_bl);

  return;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_fwrt
-----------------------------------------------------------------------------------------------------*/
void File_read_perfomance_test(T_f_wr_test *p_fwrt, T_f_test_results *p_result)
{
  FX_FILE            f;
  uint32_t           tot_min_bl_rd_time      = 0xFFFFFFFF;
  uint32_t           tot_max_bl_rd_time      = 0;
  uint32_t           tot_aver_bl_rd_time_acc = 0;
  uint32_t           tot_aver_bl_rd_time_cnt = 0;

  uint32_t           min_bl_rd_time  = 0xFFFFFFFF;
  uint32_t           max_bl_rd_time  = 0;
  uint32_t           aver_bl_rd_time = 0;
  uint32_t           read_data    = 0;
  uint32_t           tacc            = 0;
  uint32_t           bl_cnt          = 0;

  uint8_t            *data_bl;
  uint32_t           res;
  T_sys_timestump    t1, t2;
  uint32_t           stat_acc = 0;

  GET_MCBL;

  // Открываем файл на запись
  res = fx_file_open(&fat_fs_media,&f, p_fwrt->filename,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS)
  {
    MPRINTF("\r\nUnnable to open file %s! Error %d.\r\n", p_fwrt->filename, res);
    goto exit_;
  }

  p_fwrt->file_size = f.fx_file_current_file_size;


  // Гармонизируем установки
  if (p_fwrt->file_size < p_fwrt->block_size)
  {
    p_fwrt->block_size = p_fwrt->file_size;
  }
  if (p_fwrt->stat_block_sz > p_fwrt->file_size)
  {
    p_fwrt->stat_block_sz = p_fwrt->file_size;
  }


  // Выделяем память для массивов
  data_bl = App_malloc(p_fwrt->block_size);
  if (data_bl == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for data block!\r\n");
    goto exit_;
  }




  // Начинаем цикл
  MPRINTF("\r\n\r\n");

  do
  {
    uint32_t blsz;
    uint32_t tdiff;
    ULONG    actual_size;

    blsz = p_fwrt->block_size;
    if ((p_fwrt->file_size - read_data) < blsz)
    {
      blsz = p_fwrt->file_size - read_data;
    }

    Get_hw_timestump(&t1);
    res = fx_file_read(&f, data_bl, blsz,&actual_size);
    Get_hw_timestump(&t2);
    if ((res != FX_SUCCESS) || (actual_size != blsz))
    {
      MPRINTF("\r\nBlock read error %d!\r\n",res);
      break;
    }
    tdiff =  Hw_timestump_diff32_us(&t1,&t2);
    if (tdiff <  min_bl_rd_time)  min_bl_rd_time = tdiff;
    if (tdiff >  max_bl_rd_time)  max_bl_rd_time = tdiff;
    tacc += tdiff;
    bl_cnt++;
    read_data += blsz;
    stat_acc += blsz;

    if (stat_acc >= p_fwrt->stat_block_sz)
    {

      // Выводим статистику

      aver_bl_rd_time = tacc / bl_cnt;

      MPRINTF("iter=%03d read=%10d  min=%06d us  max=%06d us  aver=%06d us aver_speed=%9.0f Byte/s\r\n",p_fwrt->curr_iter,  read_data, min_bl_rd_time, max_bl_rd_time, aver_bl_rd_time, (float)p_fwrt->block_size * 1000000.0f / (float)aver_bl_rd_time);

      if (min_bl_rd_time <  tot_min_bl_rd_time)  tot_min_bl_rd_time = min_bl_rd_time;
      if (max_bl_rd_time >  tot_max_bl_rd_time)  tot_max_bl_rd_time = max_bl_rd_time;
      tot_aver_bl_rd_time_cnt++;
      tot_aver_bl_rd_time_acc += aver_bl_rd_time;

      tacc            = 0;
      bl_cnt          = 0;
      min_bl_rd_time  = 0xFFFFFFFF;
      max_bl_rd_time  = 0;
      aver_bl_rd_time = 0;
      stat_acc = 0;
    }

    if (read_data >= p_fwrt->file_size)
    {
      uint32_t tot_aver = tot_aver_bl_rd_time_acc / tot_aver_bl_rd_time_cnt;
      float    aver_speed = (float)p_fwrt->block_size * 1000000.0f / (float)tot_aver;
      MPRINTF("\r\n\r\nEnd.iter=%03d read=%10d tot_min=%d tot_max=%d tot_aver=%d tot_aver_speed=%9.0f Byte/s\r\n\r\n",p_fwrt->curr_iter, read_data, tot_min_bl_rd_time, tot_max_bl_rd_time, tot_aver, aver_speed);
      p_result->aver_bl_time = tot_aver;
      p_result->max_bl_time = tot_max_bl_rd_time;
      p_result->min_bl_time = tot_min_bl_rd_time;
      p_result->aver_bl_speed = aver_speed;
      break;
    }

  } while (1);


  fx_file_close(&f);
  fx_media_flush(&fat_fs_media);

exit_:
  if (data_bl)  App_free(data_bl);

  return;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void File_Read_Write_test(void)
{
  uint8_t             b;
  T_f_wr_test         fwrt;
  T_f_test_results *res_arr;

  GET_MCBL;

  fwrt.file_size     = 100000000;
  fwrt.block_size    = 8192;
  fwrt.stat_block_sz = 5000000;
  fwrt.iter_num      = 1;
  strcpy(fwrt.filename, "T100M.bin");


  // Выделяем память для массива результатов
  res_arr = App_malloc(sizeof(T_f_test_results) * fwrt.iter_num);
  if (res_arr == NULL)
  {
    MPRINTF("\r\nUnnable allocate memory for iteration results array!\r\n");
    return;
  }

  Print_File_write_test_header(&fwrt);

  do
  {
    if (WAIT_CHAR(&b,  ms_to_ticks(1000)) == RES_OK)
    {
      if (b > 0)
      {
        switch (b)
        {
        case 'W':
        case 'w':
          for (uint32_t i=0; i < fwrt.iter_num; i++)
          {
            fwrt.curr_iter = i;
            File_write_perfomance_test(&fwrt,&res_arr[i]);
          }
          break;
        case 'R':
        case 'r':
          for (uint32_t i=0; i < fwrt.iter_num; i++)
          {
            fwrt.curr_iter = i;
            File_read_perfomance_test(&fwrt,&res_arr[i]);
          }
          break;
        case 'P':
        case 'p':
          // Печать результатов
          MPRINTF("\r\nResults of %d iterations\r\n", fwrt.iter_num);
          for (uint32_t i=0; i < fwrt.iter_num; i++)
          {
            MPRINTF("iter=%03d tot_min=%d tot_max=%d tot_aver=%d tot_aver_speed=%9.0f Byte/s\r\n",i,res_arr[i].min_bl_time, res_arr[i].max_bl_time, res_arr[i].aver_bl_time, res_arr[i].aver_bl_speed);
          }
          break;
        case 'S':
        case 's':
          Print_File_write_test_header(&fwrt);
          Edit_uinteger_val(EDIT_ROW_POS,&fwrt.file_size, 1, 0x7FFFFFFF);
          Print_File_write_test_header(&fwrt);
          break;
        case 'B':
        case 'b':
          Print_File_write_test_header(&fwrt);
          Edit_uinteger_val(EDIT_ROW_POS,&fwrt.block_size, 1, 0x100000);
          Print_File_write_test_header(&fwrt);
          break;
        case 'T':
        case 't':
          Print_File_write_test_header(&fwrt);
          Edit_uinteger_val(EDIT_ROW_POS,&fwrt.stat_block_sz, 1, 0x7FFFFFFF);
          Print_File_write_test_header(&fwrt);
          break;
        case 'I':
        case 'i':
          Print_File_write_test_header(&fwrt);
          Edit_uinteger_val(EDIT_ROW_POS,&fwrt.iter_num, 1, 1000);
          App_free(res_arr);
          // Выделяем память для массива результатов
          res_arr = App_malloc(sizeof(T_f_test_results) * fwrt.iter_num);
          if (res_arr == NULL)
          {
            MPRINTF("\r\nUnnable allocate memory for iteration results array!\r\n");
            return;
          }
          Print_File_write_test_header(&fwrt);
          break;
        case 'N':
        case 'n':
          {
            char tmp_buf[TEST_FILE_NAME_LEN];
            Print_File_write_test_header(&fwrt);
            if (Edit_string_in_pos(tmp_buf, TEST_FILE_NAME_LEN,EDIT_ROW_POS, fwrt.filename) == RES_OK)
            {
              strcpy(fwrt.filename, tmp_buf);
            }
            Print_File_write_test_header(&fwrt);
          }
          break;
        case VT100_ESC:
          return;
        default:
          Print_File_write_test_header(&fwrt);
          break;
        }
      }
    }
  }while (1);
}

