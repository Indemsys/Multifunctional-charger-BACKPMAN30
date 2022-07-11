#include "App.h"
#include "Flash_media_driver.h"
#include "STfs_int.h"
#include "STfs_api.h"

T_stfs_fcbl g_fcbls[SFFS_NFILE];

static char  g_stfs_file_name[STFS_FLASH_WORD_SIZE];



/*-----------------------------------------------------------------------------------------------------


  \param sector

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_lw_get_sector_useful_area_size(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  return ((T_device_mem_map *)p_fcbl->dev_map)[p_fcbl->sector].last_adr -((T_device_mem_map *)p_fcbl->dev_map)[p_fcbl->sector].start_adr;
}

/*-----------------------------------------------------------------------------------------------------
  Получаем параметры Flash


  \param pdev_map
  \param empty_pttrn
  \param numsec

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  STfs_lw_get_flash_mem_params(void **pdev_map, uint32_t *number_of_sectors)
{
  return FlashDriver_get_flash_mem_params(pdev_map, number_of_sectors);
}

/*-----------------------------------------------------------------------------------------------------


  \param adr
  \param buf
  \param cnt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_write_file_name_data(uint32_t adr, void *buf, uint32_t cnt)
{
  return FlashDriver_program_aligned_pages(adr,cnt,buf);
}

/*-----------------------------------------------------------------------------------------------------
  Записываем блок данных

  \param adr
  \param buf
  \param cnt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_write_data(T_stfs_fcbl *p_fcbl, void *buf, uint32_t cnt)
{
  uint32_t adr = p_fcbl->curr_addr;
  return FlashDriver_program_pages(adr,cnt,buf, p_fcbl->prog_word_buf,&p_fcbl->prog_word_cnt);
}

/*-----------------------------------------------------------------------------------------------------
  Дозапись остатка данных

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_write_data_rest(T_stfs_fcbl *p_fcbl)
{
  uint32_t adr = p_fcbl->start_addr + p_fcbl->chunk_size - STFS_FLASH_WORD_SIZE;
  // Дозаполняем буфер слова программировния значениями 0xFF
  memset(&p_fcbl->prog_word_buf[p_fcbl->prog_word_cnt], 0xFF, STFS_FLASH_WORD_SIZE - p_fcbl->prog_word_cnt);

  return FlashDriver_program_aligned_pages(adr, STFS_FLASH_WORD_SIZE, p_fcbl->prog_word_buf);
}

/*-----------------------------------------------------------------------------------------------------
   Запись данных дескриптора

  \param adr
  \param buf
  \param cnt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_write_descriptor_data(T_stfs_fcbl *p_fcbl, void *buf, uint32_t cnt)
{
  uint32_t adr = p_fcbl->end_addr - STFS_FLASH_WORD_SIZE;
  return FlashDriver_program_aligned_pages(adr,cnt,buf);
}

/*-----------------------------------------------------------------------------------------------------
  Запись в область тэгов

  \param adr
  \param buf
  \param cnt

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_tag_data(uint32_t adr, void *buf, uint32_t cnt)
{
  return FlashDriver_program_aligned_pages(adr,cnt,buf);
}


/*-----------------------------------------------------------------------------------------------------


  \param adr
  \param buf
  \param sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  STfs_lw_read_data(uint32_t source_adr, void *buf, uint32_t sz)
{
  return FlashDriver_read_data(source_adr,sz,buf);
}

/*-----------------------------------------------------------------------------------------------------
  Стирание сектора

  \param sector
  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_erase_sector(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  return FlashDriver_erase_sector(sector);
}


/*-----------------------------------------------------------------------------------------------------
  Запись дескриптороа
  В структуре p_fcbl передаются все данные текущего фрагмента данных

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_write_descriptor(T_stfs_fcbl *p_fcbl)
{
  uint32_t                        res = STFS_OK;
  int32_t                         data_size;
  int32_t                         chunk_size;
  T_stfs_file_normal_descriptor   descriptor;

  // Запишем дескриптор фрагмента с именем файла
  memset(&descriptor, 0xFF, sizeof(descriptor));
  descriptor.file_id        = p_fcbl->file_id;
  descriptor.start_addr     = p_fcbl->start_addr;
  descriptor.number         = p_fcbl->chunk_number;
  descriptor.version        = p_fcbl->version;

  data_size = p_fcbl->curr_addr - p_fcbl->start_addr;
  if ((data_size % STFS_FLASH_WORD_SIZE) != 0)
  {
    chunk_size =((data_size / STFS_FLASH_WORD_SIZE)+ 1) * STFS_FLASH_WORD_SIZE;
  }
  else
  {
    chunk_size = data_size;
  }
  descriptor.data_size      = data_size;
  descriptor.size           = chunk_size;
  p_fcbl->chunk_size        = chunk_size;

  if (chunk_size != data_size)
  {
    // Если размер данных не равен размеру фрагмента, то следовательно остался недозаписанное слово программирования
    // и необходимо его дозаписать
    res = STfs_lw_write_data_rest(p_fcbl);
  }
  if (res == STFS_OK)
  {
    res = STfs_lw_write_descriptor_data(p_fcbl,&descriptor, sizeof(descriptor));
#ifdef SFFS_ENABLE_TRACE_WRITES
    LOG_STFS_DESCRIPTOR_WRITE;
#endif

    p_fcbl->end_addr -= sizeof(T_stfs_file_descriptor); // Верхний адрес свободной области уменьшаем на размер полного дескриптора фрагмента файла
    p_fcbl->chunk_number++;
  }
  return res;
}


/*-----------------------------------------------------------------------------------------------------
  Возврат указателя на первый файловый дескриптор если сектор имеет записи файлов и не имеет статуса зарезервированого
  Иначе возвращает 0


  \param sector

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
T_stfs_file_descriptor* STfs_lw_get_first_fdescriptor_used(uint32_t sector, T_stfs_fcbl *p_fcbl, uint32_t *p_sector_type)
{
  T_stfs_sector_descriptor  sdescr;
  T_stfs_sector_descriptor *p_sdescr;
  p_sdescr = (T_stfs_sector_descriptor *)((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr;
  STfs_lw_read_data((uint32_t)p_sdescr, (uint8_t *)&sdescr, sizeof(sdescr));
  if (sdescr.u.sector_used == STSF_TAG_USED_SECTOR)
  {
    p_sdescr--;
    if (p_sector_type != 0) *p_sector_type = STFS_SECTOR_USED;
    return (T_stfs_file_descriptor *)p_sdescr;
  }
  if (p_sector_type != 0)
  {
    if (sdescr.r.sector_reserved == STSF_TAG_RESERVED_SECTOR)
    {
      *p_sector_type = STFS_SECTOR_RESERVED;
    }
    else if ((sdescr.r.sector_reserved == EMPTY_PTTRN) && (sdescr.u.sector_used == EMPTY_PTTRN))
    {
      *p_sector_type = STFS_SECTOR_EMPTY;
    }
    else
    {
      *p_sector_type = STFS_BAD_SECTOR;
    }
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Возврат указателя на первый файловый дескриптор если сектор имеет записи файлов и имеет статуса зарезервированого
  Иначе возвращает 0


  \param sector

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
T_stfs_file_descriptor* STfs_lw_get_first_fdescriptor_resv(uint32_t sector, T_stfs_fcbl *p_fcbl, uint32_t *p_sector_type)
{
  T_stfs_sector_descriptor  sdescr;
  T_stfs_sector_descriptor *p_sdescr;
  p_sdescr = (T_stfs_sector_descriptor *)((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr;
  STfs_lw_read_data((uint32_t)p_sdescr, (uint8_t *)&sdescr, sizeof(sdescr));
  if ((sdescr.r.sector_reserved == STSF_TAG_RESERVED_SECTOR) && (sdescr.u.sector_used == EMPTY_PTTRN))
  {
    p_sdescr--;
    if (p_sector_type != 0) *p_sector_type = STFS_SECTOR_RESERVED;
    return (T_stfs_file_descriptor *)p_sdescr;
  }
  if (p_sector_type != 0)
  {
    if (sdescr.u.sector_used == STSF_TAG_USED_SECTOR)
    {
      *p_sector_type = STFS_SECTOR_USED;
    }
    else if ((sdescr.r.sector_reserved == EMPTY_PTTRN) && (sdescr.u.sector_used == EMPTY_PTTRN))
    {
      *p_sector_type = STFS_SECTOR_EMPTY;
    }
    else
    {
      *p_sector_type = STFS_BAD_SECTOR;
    }
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param sector

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_get_sector_type(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  T_stfs_sector_descriptor *p_sdescr;
  T_stfs_sector_descriptor  sdescr;
  p_sdescr = (T_stfs_sector_descriptor *)((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr;
  STfs_lw_read_data((uint32_t)p_sdescr, (uint8_t *)&sdescr, sizeof(sdescr));
  if (sdescr.u.sector_used == STSF_TAG_USED_SECTOR)
  {
    return STFS_SECTOR_USED;
  }
  if (sdescr.r.sector_reserved == STSF_TAG_RESERVED_SECTOR)
  {
    return STFS_SECTOR_RESERVED;
  }
  if ((sdescr.u.sector_used == EMPTY_PTTRN) && (sdescr.r.sector_reserved == EMPTY_PTTRN))
  {
    return STFS_SECTOR_EMPTY;
  }
  return STFS_BAD_SECTOR;

}

/*-----------------------------------------------------------------------------------------------------
  Поиск свободных/неиспользуемых идентификаторов файла
  Вызывается из функции открытия файла

  \param maxID
  \param fcb

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t STfs_lw_get_freeID(T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector_type;
  uint32_t                sector;
  uint32_t                file_id = 0;

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector,p_fcbl,&sector_type);
    if (p_descriptor == 0) continue;
    if (sector_type == STFS_BAD_SECTOR) continue;
    if (sector_type == STFS_SECTOR_RESERVED) continue;

    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    while (descriptor.n.file_id != EMPTY_PTTRN)
    {
      if ((descriptor.n.file_id > file_id) && (descriptor.n.number == 0) && (descriptor.e.deleted == EMPTY_PTTRN))
      {
        file_id = descriptor.n.file_id;
      }
      p_descriptor--;
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    }
  }
  file_id++;
  return file_id;
}


/*-----------------------------------------------------------------------------------------------------
  Записываем в сектор тэг используемого сектора

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_write_used_sector_tag(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  uint32_t addr =((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr;
  T_stfs_used_sector_descriptor  us_descriptor;
  memset(&us_descriptor,0xFF, sizeof(us_descriptor));
  us_descriptor.sector_used = STSF_TAG_USED_SECTOR;
  return STfs_lw_tag_data(addr,&us_descriptor, sizeof(us_descriptor));
}

/*-----------------------------------------------------------------------------------------------------
  Записываем в сектор тэг зарезервированного сектора

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_write_reserved_sector_tag(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  uint32_t addr =((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr + STFS_FLASH_WORD_SIZE;
  T_stfs_reserved_sector_descriptor rs_descriptor;
  memset(&rs_descriptor, 0XFF, sizeof(rs_descriptor));
  rs_descriptor.sector_reserved = STSF_TAG_RESERVED_SECTOR;
  return STfs_lw_tag_data(addr,&rs_descriptor, sizeof(rs_descriptor));
}


/*-----------------------------------------------------------------------------------------------------
  Отметить фрагмен как стертый записью специальной отметки STUMP_CHUNK_ERASED  в поле descriptor.stamp_of_erased_file

  \param p_descriptor
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_write_deleted_chunk_tag(T_stfs_file_descriptor *p_src_descriptor, uint32_t dest_addr)
{
  memset(&p_src_descriptor->e, 0xFF, sizeof(T_stfs_file_erase_descriptor));
  p_src_descriptor->e.deleted = STSF_TAG_CHUNK_DELETED;
  return STfs_lw_tag_data(dest_addr, (void *)&p_src_descriptor->e, sizeof(T_stfs_file_erase_descriptor));
}

/*-----------------------------------------------------------------------------------------------------
  Запрашиваем количество свободного пространства в секторе доступного для записи новых фрагментов файлов


  \param sector
  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_obtain_free_space_limits(uint32_t sector,  T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                chunks_size_sum      = 0;
  uint32_t                descriptors_size_sum = 0;
  uint32_t                sector_type;
  uint32_t                useful_sector_size;

  // Получаем размер от нижнего адреса до верхнего адреса сектора исключая дескрипторы сектора
  useful_sector_size = _STfs_lw_get_sector_useful_area_size(sector, p_fcbl);
  // Позиционируемся на дескриптор первого фрагмента
  p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
  if (sector_type == STFS_SECTOR_EMPTY)
  {
    p_fcbl->end_addr =((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr;
    p_fcbl->start_addr =((T_device_mem_map *)p_fcbl->dev_map)[sector].start_adr;
    p_fcbl->curr_addr  = p_fcbl->start_addr;
    // Возвращаем размер полезного пространства сектора предназначенного для размещения дескрипторов и данных
    // за вычетом пустого конечного дескриптора цепочки дескрипторов
    return useful_sector_size - sizeof(descriptor);
  }
  if (sector_type == STFS_BAD_SECTOR)
  {
    return 0;
  }
  if (p_descriptor == 0) return 0;

  // Проходим по всем дескрипторам пока не найдем чистый

  STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
  while (descriptor.n.file_id != EMPTY_PTTRN)
  {
    chunks_size_sum      += descriptor.n.size;
    descriptors_size_sum += sizeof(descriptor);
    p_descriptor--;
    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
  }
  p_fcbl->end_addr   =((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr - descriptors_size_sum;
  p_fcbl->start_addr =((T_device_mem_map *)p_fcbl->dev_map)[sector].start_adr + chunks_size_sum;
  p_fcbl->curr_addr  = p_fcbl->start_addr;

  // Возвращаем размер полезного пространства сектора предназначенного для размещения дескрипторов и данных
  // за вычетом пустого конечного дескриптора цепочки дексрипторов
  useful_sector_size = useful_sector_size  - chunks_size_sum - descriptors_size_sum - sizeof(descriptor);


  return useful_sector_size;
}

/*-----------------------------------------------------------------------------------------------------
  Проверяем есть ли другие файлы открытые на запись в данном секторе
  Возвращает SFFS_TRUE если открытые на запись  файлы в заданном секторе обнаружены

  \param sector
  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_is_sector_used_for_write(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  uint32_t i;

  for (i = 0; i < SFFS_NFILE; i++)
  {
    if (&g_fcbls[i] == p_fcbl)
    {
      continue;
    }
    if ((g_fcbls[i].status & STFS_OPEN_TO_WRITE) && (g_fcbls[i].sector == sector))
    {
      return (STFS_TRUE);
    }
  }
  return (STFS_FALSE);
}

/*-----------------------------------------------------------------------------------------------------
  Найти дескриптор заданного фрагмента заданного файла

  Если дескриптор найден, то:
   - записываются в управляющую структуру p_fcbl адреса начала и конеца фрагмента данных
   - увеличивается на 1 номер фрагмента в структуре p_fcbl
   - возвращается SFFS_TRUE

  Вызывается из функций API Open и Read

  \param p_fcbl

  \return uint32_t  возвращает SFFS_TRUE если фрагмент найден
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_get_file_chunk_for_read(T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                i;
  uint32_t                sector;
  uint32_t                sector_type;

  sector = p_fcbl->sector;

  // Ищем фрагменты файла с заданным идентификатором файла
  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);

    if (p_descriptor != 0)
    {
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        if ((descriptor.n.file_id == p_fcbl->file_id) &&  (descriptor.n.number == p_fcbl->chunk_number) && (descriptor.e.deleted == EMPTY_PTTRN))
        {
          // Возвращаем номер сектора, смещения начала и конца фрагмента и номер слудующей записи
          p_fcbl->sector      = sector;
          p_fcbl->start_addr  = descriptor.n.start_addr;
          p_fcbl->curr_addr   = p_fcbl->start_addr;
          p_fcbl->end_addr    = p_fcbl->start_addr + descriptor.n.data_size;
          p_fcbl->chunk_number++;
          return (STFS_TRUE);
        }
        // Дескриптор искомого фрагмента искомого файла еще не найден. Продолжаем читать дескрипторы
        p_descriptor--;
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }

    }
    // Дескриптор искомого фрагмента искомого файла еще не найден.
    // Перемещаемся на следующий сектор по кольцу
    sector++;
    if (sector == p_fcbl->number_of_sectors) sector = 0;
  }
  return (STFS_FALSE);
}



/*-----------------------------------------------------------------------------------------------------
  Ищем файл по заданному имени
  Если файл найден возвращаем STFS_OK и в структуру p_fcbl записывается номер сектора и идентификатор файла


  Вызывается из:
    Sffs_delete
    Sffs_open
    Sffs_rename


  \param fname
  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_find_file_by_name(const char *fname, T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor   descriptor;
  T_stfs_file_descriptor  *p_descriptor;
  uint32_t                 sector;
  uint32_t                 sector_type;
  uint32_t                 max_fid = 0;

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    // Получаем указатель на первый файловый дескриптор
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
    if (sector_type == STFS_BAD_SECTOR) continue;

    if (p_descriptor != 0)
    {
      // Читаем дескриптор файла
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));

      // Проверяем и читаем дескрипторы пока на наткнемся на чистый блок, после чего переходим на следующий сектор
      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        if ((descriptor.n.number == 0) && (descriptor.e.deleted == EMPTY_PTTRN))
        {
          // Это фрагмент с именем файла
          STfs_lw_read_data(descriptor.n.start_addr, (uint8_t *)g_stfs_file_name,  STFS_FLASH_WORD_SIZE);

          if (strcmp(fname,g_stfs_file_name) == 0)
          {
            p_fcbl->file_id  = descriptor.n.file_id;
            p_fcbl->version  = descriptor.n.version;
            p_fcbl->sector   = sector;
            return STFS_OK; // Имя файла найдено
          }
          if (descriptor.n.file_id > max_fid) max_fid = descriptor.n.file_id; // Отслеживаем максимальный fid
        }
        // Переходим на следующий дескриптор
        p_descriptor--;
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }
    }
  }
  return STFS_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Найти и занять пустой сектор или незанятый записью файла сектор в котором есть еще место для записи файла
  Функция используется при записи в открытый файл данных или в установке позиции в файле открытом на чтение из API

  \param fcb

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_alloc_next_sector(T_stfs_fcbl *p_fcbl, uint32_t needed_space)
{
  uint32_t   i;
  uint32_t   sector;
  uint32_t   sector_type;
  uint32_t   res;

  // Начинаем со следующего сектора от текущего
  sector = p_fcbl->sector + 1;
  if (sector >= p_fcbl->number_of_sectors) sector = 0;

  // Проходим по всем сектора за исключением указанного в p_fcbl
  for (i = 0; i < (p_fcbl->number_of_sectors - 1); i++)
  {
    sector_type = STfs_get_sector_type(sector, p_fcbl);
    if (sector_type == STFS_SECTOR_EMPTY)
    {
      // Нашли чистый сектор
      // Отметим его как занятый
      p_fcbl->sector = sector;
      STfs_lw_obtain_free_space_limits(sector, p_fcbl);
      res = STfs_write_used_sector_tag(sector, p_fcbl);
      if (res != STFS_OK)
      {
        return STFS_SECTOR_ALLOC_ERROR;
      }
      return STFS_OK;
    }
    // Проверяем используемые сектора
    else if (sector_type == STFS_SECTOR_USED)
    {
      // Проверяем нет ли файлов уже ведущих запись в этот сектор
      if (STfs_lw_is_sector_used_for_write(sector, p_fcbl) == STFS_FALSE)
      {
        if (STfs_lw_obtain_free_space_limits(sector, p_fcbl) >= needed_space)
        {
          p_fcbl->sector = sector;
          return STFS_OK;
        }
      }
    }
    sector++;
    if (sector >= p_fcbl->number_of_sectors) sector = 0;
  }
  return STFS_SECTOR_ALLOC_ERROR;
}


/*-----------------------------------------------------------------------------------------------------
  Проверка все ли фрагметы файлов в используемом секторе обозначены как стертые

  \param block
  \param fcb

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_is_used_sector_invalid(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector_type;


  p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
  if (p_descriptor == 0) return STFS_FALSE;

  // Читаем первый дескриптор
  STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));

  // Ищем пока есть используемые дескриптроы и пока не найдем дескриптор нестертого файла
  while (descriptor.n.file_id != EMPTY_PTTRN)
  {
    if (descriptor.e.deleted != STSF_TAG_CHUNK_DELETED)
    {
      return STFS_FALSE;
    }
    p_descriptor--;
    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
  }
  return STFS_TRUE;
}

/*-----------------------------------------------------------------------------------------------------
  Проверка все ли фрагметы файлов в зарезервированном секторе обозначены как стертые

  \param block
  \param fcb

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_lw_is_resv_sector_invalid(uint32_t sector, T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector_type;


  p_descriptor = STfs_lw_get_first_fdescriptor_resv(sector, p_fcbl,&sector_type);
  if (p_descriptor == 0) return STFS_FALSE;

  // Читаем первый дескриптор
  STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));

  // Ищем пока есть используемые дескриптроы и пока не найдем дескриптор нестертого файла
  while (descriptor.n.file_id != EMPTY_PTTRN)
  {
    if (descriptor.e.deleted != STSF_TAG_CHUNK_DELETED)
    {
      return STFS_FALSE;
    }
    p_descriptor--;
    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
  }
  return STFS_TRUE;
}


/*-----------------------------------------------------------------------------------------------------
  Создаем файл с именем fname
  Одновременно функция закрывает для использования полностью занятые сектора где уже нельзя создать файл
  Функция вызывается при переименовании файла и при открытии файла на запись

  \param fname
  \param fcb

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_lw_create(const char *fname, T_stfs_fcbl *p_fcbl)
{
  uint32_t                         i;
  uint32_t                         sector;
  uint32_t                         sector_type;
  uint8_t                          sector_found = 0;
  uint32_t                         free_space;
  uint32_t                         res;

  if (strlen(fname) > (STFS_FLASH_WORD_SIZE - 1))
  {
    return STFS_BAD_FILE_NAME;
  }

  sector = 0;
  // Проходим в поиске доступных для записи секторов. Предпочтение отдаем уже используемым секторам
  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    uint32_t st = STfs_get_sector_type(i, p_fcbl);
    if (st == STFS_SECTOR_USED)
    {
      // Проверяем используемые сектора
      // Если в текущем секторе есть открытые на запись файлы, то перейти к следующему сектору
      if (STfs_lw_is_sector_used_for_write(i, p_fcbl) == STFS_TRUE) continue;

      p_fcbl->sector = i;
      free_space = STfs_lw_obtain_free_space_limits(i, p_fcbl);
      // Если места меньше чем размер дескриптора и блока с названием файла, то пропускаем этот сектор
      if (free_space < (sizeof(T_stfs_file_descriptor)+ STFS_FLASH_WORD_SIZE)) continue;
      sector       = i;
      sector_found = 1;
      sector_type  = st;
      break;
    }

    if (st == STFS_SECTOR_EMPTY)
    {
      // Сектор пуст и доступен, но поиск здесь не прерываем чтобы попытаться найти уже используемый сектор
      if (sector_found == 0)
      {
        sector         = i;
        sector_found   = 1;
        sector_type    = st;
        p_fcbl->sector = i;
        STfs_lw_obtain_free_space_limits(i, p_fcbl);
      }
    }
  }
  if (sector_found == 0)
  {
    return STFS_ERROR;
  }
  // Снова вызываем определение границ поскольку содержимое p_fcbl могло быть изменено в предыдцщем цикле поиска
  STfs_lw_obtain_free_space_limits(sector, p_fcbl);
  p_fcbl->chunk_number = 0;
  p_fcbl->sector       = sector;
  memset(g_stfs_file_name, 0, STFS_FLASH_WORD_SIZE);
  strcpy(g_stfs_file_name, fname);

  // Запишем фрагмент с именем файла
  res = STfs_lw_write_file_name_data(p_fcbl->start_addr, g_stfs_file_name, STFS_FLASH_WORD_SIZE);
  if (res != STFS_OK)
  {
    return res;
  }

  p_fcbl->curr_addr  += STFS_FLASH_WORD_SIZE;

  // Запишем дескриптор фрагмента с именем файла
  res = STfs_lw_write_descriptor(p_fcbl);
  if (res != STFS_OK)
  {
    return res;
  }

  p_fcbl->start_addr += STFS_FLASH_WORD_SIZE;
  p_fcbl->curr_addr   = p_fcbl->start_addr;

  // Если сегмент FLASH еще не отмечен как используемый, то записать отметку использования сектора
  if (sector_type == STFS_SECTOR_EMPTY)
  {
    return STfs_write_used_sector_tag(sector, p_fcbl);
  }
  return STFS_OK;
}



