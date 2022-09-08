#include "App.h"
#include "Flash_media_driver.h"
#include "STfs_int.h"
#include "STfs_api.h"


extern T_stfs_fcbl             g_fcbls[SFFS_NFILE];

static uint8_t                 defragmentation_buf[STFS_TMPBUFLEN];
static uint8_t                 defrag_active;
T_stfs_fcbl                    g_write_fcbl;

/*-----------------------------------------------------------------------------------------------------
  Ишем индекс неиспользуемой управляющей структуры

  \return int32_t возвращает отрицательное значение если управляющая структура не найдена
-----------------------------------------------------------------------------------------------------*/
static int32_t _STfs_find_free_fcbl(void)
{
  T_stfs_fcbl  *p_fcbl;
  uint32_t      i;

  p_fcbl =&g_fcbls[0];
  for (i = 0; i < SFFS_NFILE; i++)
  {
    if (!(p_fcbl->status & (STFS_OPEN_TO_READ | STFS_OPEN_TO_WRITE)))
    {
      p_fcbl->status = 0;
      p_fcbl->p_thread_id = (uint32_t)tx_thread_identify();
      return (i);
    }
    p_fcbl++;
  }
  return (STFS_EOF);
}

/*-----------------------------------------------------------------------------------------------------
  Получаем параметры  Flash носителя

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_set_params(T_stfs_fcbl *p_fcbl)
{
  switch (p_fcbl->drive_id)
  {
  case INT_FLASH_BANK2:
    // Получаем параметры из модуля обслуживания Data Flash
    if (STfs_lw_get_flash_mem_params(&p_fcbl->dev_map,&p_fcbl->number_of_sectors) == 0) return STFS_TRUE;
    return (STFS_FALSE);
  default:
    return (STFS_FALSE);
  }
}


/*-----------------------------------------------------------------------------------------------------
  Отмечаем все дескрипторы фрагментов файла во всех секторах как удаленные записывая тэг в поле deleted дескрипторов
  Если в секторе не остается дескрипторов не обозначенных как удаленные, то стираем весь сектор
  Если del_temp = SFFS_TRUE то стираем только в зарезервированных секторах

  \param p_fcbl
  \param del_temp - 1 -
-----------------------------------------------------------------------------------------------------*/
static int32_t _STfs_delete_file_on_defragmentation(T_stfs_fcbl *p_fcbl, uint32_t del_reserved_only)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector;
  uint32_t                sector_type;
  uint32_t                res;

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    if (del_reserved_only == STFS_FALSE)
    {
      p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
      if ((sector_type == STFS_BAD_SECTOR) ||  (sector_type == STFS_SECTOR_EMPTY) || (sector_type == STFS_SECTOR_RESERVED)) continue;
    }
    else
    {
      p_descriptor = STfs_lw_get_first_fdescriptor_resv(sector, p_fcbl,&sector_type);
      if ((sector_type == STFS_BAD_SECTOR) ||  (sector_type == STFS_SECTOR_EMPTY) || (sector_type == STFS_SECTOR_USED)) continue;
    }

    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    while (descriptor.n.file_id != EMPTY_PTTRN)
    {
      if ((descriptor.n.file_id == p_fcbl->file_id) && (descriptor.e.deleted == EMPTY_PTTRN))
      {
        res = STfs_write_deleted_chunk_tag(&descriptor, (uint32_t)p_descriptor);
        if (res != STFS_OK) return STFS_CHUNK_DELETE_ERROR;
      }
      p_descriptor--;
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    }
    // Если все файлы в секторе отмечены как стертые, то автоматически стираем весь сектор
    if (del_reserved_only == STFS_FALSE)
    {
      if (STfs_lw_is_used_sector_invalid(sector, p_fcbl) == STFS_TRUE)
      {
        res = STfs_lw_erase_sector(sector, p_fcbl);
        if (res != STFS_OK)
        {
          return  res;
        }
      }
    }
    else
    {
      if (STfs_lw_is_resv_sector_invalid(sector, p_fcbl) == STFS_TRUE)
      {
        res = STfs_lw_erase_sector(sector, p_fcbl);
        if (res != STFS_OK)
        {
          return  res;
        }
      }
    }



  }
  return  STFS_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Удаление файла
  Удаление заключается в установке значения обратному empty_pttrn в поле fid во всех дескрипторах относящихся у файлу
  Вызывается при удалении файла и при создании нового из API

  \param p_fcbl

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t _STfs_delete_file_and_erase_sectors(T_stfs_fcbl *p_fcbl, uint8_t en_sector_erase)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                i;
  uint32_t                sector;
  uint32_t                sector_type;
  uint32_t                res;

  sector = p_fcbl->sector;
  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    // Читаем дескриптор сектора
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);

    if (p_descriptor != 0)
    {
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));

      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        if ((descriptor.n.file_id == p_fcbl->file_id) && (descriptor.e.deleted == EMPTY_PTTRN))
        {
          // Отметить фрагмент файла как стертый
          STfs_write_deleted_chunk_tag(&descriptor, (uint32_t)p_descriptor);
        }
        // Переход к следующему дескриптору
        p_descriptor--;
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }
      if (en_sector_erase)
      {
        if (STfs_lw_is_used_sector_invalid(sector, p_fcbl) == STFS_TRUE)
        {
          res= STfs_lw_erase_sector(sector, p_fcbl);
          if (res != STFS_OK)
          {
            return res;
          }
        }
      }
    }
    sector++;
    if (sector == p_fcbl->number_of_sectors) sector = 0;
  }
  return STFS_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Поиск пустого(стертого) сектора не занятого файлами
  Если сектор найден он помечается как временно занятый

  \param fb
  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_search_empty_sector_and_mark_as_reserved(T_stfs_fcbl *p_resv_cbl, T_stfs_fcbl *p_fcbl)
{
  uint32_t    sector;
  uint32_t    sector_type;
  uint32_t    res;

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    sector_type = STfs_get_sector_type(sector, p_fcbl);
    if (sector_type == STFS_SECTOR_EMPTY)
    {
      res = STfs_write_reserved_sector_tag(sector, p_fcbl);

      p_resv_cbl->sector     = sector;
      p_resv_cbl->end_addr   =((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr;
      p_resv_cbl->start_addr =((T_device_mem_map *)p_fcbl->dev_map)[sector].start_adr;
      p_resv_cbl->curr_addr = p_resv_cbl->start_addr;
      return res;
    }
  }
  return STFS_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Ищем нестертый файл с минимальным file_id большим чем в fcb->file_id
  Возвращаем SFFS_TRUE если файл найден
  В p_fcbl возвращается номер сектора файла, его fid и в p_fcbl->bot_offs смещение фрагмента содержащего имя файла

  Вызывается из процедуру дефрагментрирования при поиску актуальных файлов для переноса

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_get_next_file_on_defragmentation(T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector;
  uint32_t                file_id;
  uint32_t                next_file_id;
  uint32_t                sector_type;

  file_id      = p_fcbl->file_id;
  next_file_id = EMPTY_PTTRN;
  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);

    // Пропускаем пустые и зарезервированные сектора
    if ((sector_type != STFS_SECTOR_USED) || (p_descriptor == 0)) continue;

    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    while (descriptor.n.file_id != EMPTY_PTTRN)
    {
      if ((descriptor.n.file_id > file_id) && (descriptor.n.file_id < next_file_id) && (descriptor.e.deleted == EMPTY_PTTRN))
      {
        next_file_id       = descriptor.n.file_id;
        p_fcbl->sector     = sector;                               // Запомним на каком секторе остановились
        p_fcbl->start_addr = descriptor.n.start_addr;              // Запомним начальный адрес фрагмента файла
        p_fcbl->curr_addr  = p_fcbl->start_addr;
        p_fcbl->end_addr   = descriptor.n.start_addr + descriptor.n.data_size; // Запомним конечный адрес фрагмента файла
        p_fcbl->version    = descriptor.n.version;
        if (next_file_id == file_id + 1)
        {
          // Если найден непосредственно следующий номер файла, то сразу выходим
          p_fcbl->file_id       = next_file_id;
          p_fcbl->chunk_number  = 0;
          return STFS_TRUE;
        }
      }
      p_descriptor--;
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    }
  }
  if (next_file_id == EMPTY_PTTRN) return STFS_FALSE; // Файл не найден
  p_fcbl->file_id  = next_file_id;
  p_fcbl->chunk_number   = 0;
  return STFS_TRUE;
}

/*-----------------------------------------------------------------------------------------------------
  Поиск фрагмента файла по его идентификационному номеру file_id и номеру фрагмента среди неудаленных файлов

  Вызывается при чтении во временный буфер во время дефрагментации

  \param p_fcbl

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_search_chunk_by_fid_and_num(T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                i;
  uint32_t                sector;
  uint32_t                sector_type;

  sector = p_fcbl->sector;
  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);

    // Пропускаем пустые и зарезервированные сектора
    if ((sector_type != STFS_SECTOR_USED) || (p_descriptor == 0))
    {
      sector++;
      if (sector == p_fcbl->number_of_sectors) sector = 0;
      continue;
    }

    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    while (descriptor.n.file_id != EMPTY_PTTRN)
    {
      if ((descriptor.n.file_id == p_fcbl->file_id) &&  (descriptor.n.number == p_fcbl->chunk_number) && (descriptor.e.deleted == EMPTY_PTTRN))
      {
        p_fcbl->sector     = sector;                               // Запомним на каком секторе остановились
        p_fcbl->start_addr = descriptor.n.start_addr;              // Запомним начальный адрес фрагмента файла
        p_fcbl->curr_addr  = p_fcbl->start_addr;
        p_fcbl->end_addr   = descriptor.n.start_addr + descriptor.n.data_size; // Запомним конечный адрес фрагмента файла
        p_fcbl->chunk_number++;
        return STFS_OK;
      }
      p_descriptor--;
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
    }
    sector++;
    if (sector == p_fcbl->number_of_sectors) sector = 0;
  }
  return STFS_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Копирование файла в зарезервированный сектор или сектора
  Если файл фрагментирован, то после копирования он дефрагментируется.

  \param p_defrag_cbl  - контрольный блок файла приемника
  \param p_fcbl        - контрольный блок файла источника

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_copy_file_on_defragmetation(T_stfs_fcbl *read_fcbl)
{
  uint32_t              res;
  int32_t               sz;
  int32_t               bytes_read;
  int32_t               left_bytes;

  // Проверка размера первого фрагмента файла. Он должен быть фиксированного размера и содержать имя файла.
  if ((read_fcbl->end_addr - read_fcbl->start_addr) != STFS_FLASH_WORD_SIZE)
  {
    return STFS_ERROR;
  }

  if (defrag_active == 0)
  {
    // Если это начало процесса дефрагметации, то инициализируем вспомогательный контрольный блок
    memset(&g_write_fcbl, 0, sizeof(g_write_fcbl));
    res = _STfs_search_empty_sector_and_mark_as_reserved(&g_write_fcbl, read_fcbl);
    if (res != STFS_OK)
    {
      return STFS_ERROR;
    }
    defrag_active = 1;
  }

  // Проверяем хватит ли в зарезервированном секторе места для данных и ищем новый сектор если места нет
  if ((g_write_fcbl.end_addr - g_write_fcbl.start_addr) <= (2 * sizeof(T_stfs_file_descriptor)+STFS_FLASH_WORD_SIZE))
  {
    res = _STfs_search_empty_sector_and_mark_as_reserved(&g_write_fcbl, read_fcbl);
    if (res != STFS_OK)
    {
      return STFS_ERROR;
    }
  }

  // Первым копируем фрагмент с именем файла
  STfs_lw_read_data(read_fcbl->start_addr, defragmentation_buf, STFS_FLASH_WORD_SIZE);
  STfs_lw_write_file_name_data(g_write_fcbl.start_addr, defragmentation_buf, STFS_FLASH_WORD_SIZE);
  g_write_fcbl.curr_addr   += STFS_FLASH_WORD_SIZE;

  // Записываем дескриптор фрагмента с именем файла
  g_write_fcbl.file_id      = read_fcbl->file_id;
  g_write_fcbl.version      = read_fcbl->version + 1; // Даем фрагменту следующую по счету версию
  g_write_fcbl.chunk_number = 0;
  res = STfs_lw_write_descriptor(&g_write_fcbl);
  if (res != STFS_OK)
  {
    return STFS_ERROR;
  }
  g_write_fcbl.start_addr   = g_write_fcbl.curr_addr;

  // .................. Начинаем копирование содержимого файла ..................

  read_fcbl->curr_addr    = 0;
  read_fcbl->end_addr     = 0;
  read_fcbl->chunk_number = 1;

  do
  {
    // ......... Читаем данные во временный буфер .........
    bytes_read              = 0;
    do
    {
      if ((read_fcbl->end_addr - read_fcbl->curr_addr) == 0)
      {
        // Искать фрагмент файла
        if (_STfs_search_chunk_by_fid_and_num(read_fcbl) != STFS_OK) break;
      }
      sz = read_fcbl->end_addr - read_fcbl->curr_addr;
      if (sz > (STFS_TMPBUFLEN - bytes_read)) sz = STFS_TMPBUFLEN - bytes_read;

      STfs_lw_read_data(read_fcbl->curr_addr, defragmentation_buf + bytes_read, sz);
      read_fcbl->curr_addr   += sz; // Смещаем адрес чтения
      bytes_read             += sz; // Ведем счетчик прочитанных байт

    }while (bytes_read <  STFS_TMPBUFLEN);


    if (bytes_read == 0) break; // Если данных нет то выходим из цикла копирования

    // ......... Перенос данных из временного буфера в зарезервированный сектор .........

    left_bytes = bytes_read;
    uint8_t *ptr = defragmentation_buf;
    do
    {
      // Определяем сколько места доступно для записи данных в текущем зарезервированном секторе
      // с учетом необходимого места для дескриптора фрагмента и пустого дескриптора заканчивающего цепочку дескрипторов
      if (g_write_fcbl.end_addr > (g_write_fcbl.curr_addr + 2 * sizeof(T_stfs_file_descriptor)))
      {
        sz = g_write_fcbl.end_addr -(g_write_fcbl.curr_addr + 2 * sizeof(T_stfs_file_descriptor));
        if (sz > left_bytes) sz = left_bytes;
        // Записываем данные в новый файл
        res = STfs_lw_write_data(&g_write_fcbl, ptr, sz);
        if (res != STFS_OK)
        {
          return STFS_ERROR;
        }
        g_write_fcbl.curr_addr += sz;
        left_bytes             -= sz;
        ptr                    += sz;
      }

      // Здесь могли быть записаны все данные и тогда left_bytes = 0,
      // либо не все, поскольку все не разместились в текущем секторе, и тогда записываем дескриптор фрагмента и резервируем следующий сектор
      if (left_bytes > 0)
      {
        // Записываем дескриптор фрагмента файла и ищем следующий свободный сектор
        if (g_write_fcbl.start_addr   != g_write_fcbl.curr_addr)
        {
          res = STfs_lw_write_descriptor(&g_write_fcbl);
          if (res != STFS_OK)
          {
            return STFS_ERROR;
          }
        }

        res = _STfs_search_empty_sector_and_mark_as_reserved(&g_write_fcbl, read_fcbl);
        if (res != STFS_OK)
        {
          return STFS_ERROR;
        }
      }
    }while (left_bytes != 0);

  }while (1);


  // Записываем заключительный дескриптор фрагмента файла
  res = STfs_lw_write_descriptor(&g_write_fcbl);
  if (res != STFS_OK)
  {
    return STFS_ERROR;
  }
  g_write_fcbl.start_addr += g_write_fcbl.chunk_size;
  g_write_fcbl.curr_addr   = g_write_fcbl.start_addr;

  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Находим позицию sym в строке
  Счет позиций начинается с 0

  \param sp
  \param sym

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t _STfs_find_sym_pos_in_string(const char *str, const char sym)
{
  int32_t i;

  for (i = 0; *str != 0; i++)
  {
    if (*str == sym)
    {
      return (i);
    }
    str++;
  }
  return (-1);
}

/*-----------------------------------------------------------------------------------------------------
  Получение размера файла
  Вызывается из
   - STfs_len
   - Sffs_find
   - Sffs_find_first_file
   - Sffs_find_next_file

  \param p_fcbl
  \param set_fidx

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_get_file_size(T_stfs_fcbl *p_fcbl, uint32_t set_chunk_number)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector;
  uint32_t                chunk_number;
  uint32_t                i;
  uint32_t                file_size;
  uint32_t                sector_type;

  chunk_number = 0;
  file_size    = 0;
  sector       = p_fcbl->sector;

  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);

    if (p_descriptor != 0)
    {
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        if ((descriptor.n.file_id == p_fcbl->file_id) && (descriptor.n.number != 0) && (descriptor.e.deleted == EMPTY_PTTRN))
        {
          file_size += descriptor.n.data_size;
          if (descriptor.n.number > chunk_number) chunk_number = descriptor.n.number;
        }
        p_descriptor--;
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }
    }
    sector++;
    if (sector == p_fcbl->number_of_sectors) sector = 0;
  }

  if (set_chunk_number == STFS_TRUE)
  {
    chunk_number++;
    p_fcbl->chunk_number = chunk_number;
  }
  return (file_size);
}

/*-----------------------------------------------------------------------------------------------------
  Поиск первого обнаруженного файла с идентификатором большим чем указано в  info->file_id
  и возврат в структуре info имени файла и его идентификатора
  Вызывается из:
   - Sffs_find
   - STfs_find_first_file
   - STfs_find_next_file

  \param info
  \param p_fcbl

  \return char*
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_find_next_file_by_id(T_file_info *info, T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector;
  uint32_t                sector_type;
  char                    *fname_adr   = NULL;
  uint32_t                last_file_id = EMPTY_PTTRN;

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
    if (sector_type != STFS_SECTOR_USED) continue;
    if (p_descriptor != 0)
    {
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        if ((descriptor.n.file_id > info->file_id) && (descriptor.n.number == 0) && (descriptor.e.deleted == EMPTY_PTTRN))
        {
          if (last_file_id == EMPTY_PTTRN)
          {
            last_file_id           = descriptor.n.file_id;
            info->ver              = descriptor.n.version;
            info->file_name_sector = sector;
            fname_adr              = (char *)descriptor.n.start_addr;
          }
          else if (descriptor.n.file_id < last_file_id)
          {
            last_file_id            = descriptor.n.file_id;
            info->ver               = descriptor.n.version;
            info->file_name_sector  = sector;
            fname_adr               = (char *)descriptor.n.start_addr;
          }
        }
        p_descriptor--; // Перемещаем указатель на следующий дескриптор
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }
    }
  }
  if (last_file_id != EMPTY_PTTRN)
  {
    info->file_id = last_file_id;
    STfs_lw_read_data((uint32_t)fname_adr, info->name, STFS_FLASH_WORD_SIZE);
    info->name[STFS_FLASH_WORD_SIZE-1] = 0;
    return STFS_OK;
  }
  return STFS_ERROR;
}



/*-----------------------------------------------------------------------------------------------------
  Переименование файла
  При стирании у дескриптора фрагмента с названием старого файла выставляется признак удаленного
  и записывается фрагмент с новым названием файла и дескриптор нового фрагмента с тем же file_id какой был у старого файла

  Структура p_fcbl указавает на старый файл

  Вызывается из:
   - Sffs_rename


  \param filename
  \param p_fcbl

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_rename_file(const char *filename, T_stfs_fcbl *p_fcbl)
{
  T_stfs_file_descriptor  descriptor;
  T_stfs_file_descriptor *p_descriptor;
  uint32_t                sector_type;
  uint32_t                sector;
  uint32_t                res;

  sector = p_fcbl->sector;
  // Получаем указатель на первый файловый дескриптор
  p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
  if (p_descriptor == 0) return STFS_ERROR;

  // Ищем дескриптор названия заданного файла
  STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
  while (descriptor.n.file_id != EMPTY_PTTRN)
  {
    if (descriptor.n.number == 0)
    {
      // Здесь создаем новый фрагмент с именем файла и его дескриптор
      p_fcbl->version++; // Увеличиваем номер весии для нового дескриптора
      res = STfs_lw_create(filename, p_fcbl);
      if (res != STFS_OK) return res;

      // Отмечаем старый дескриптор как удаленный
      res= STfs_write_deleted_chunk_tag(&descriptor, (uint32_t)p_descriptor);
      return res;
    }
    p_descriptor--;
    STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
  }

  return STFS_ERROR;
}





/*-----------------------------------------------------------------------------------------------------


  \param res

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_check_return(T_stfs_fcbl *p_fcbl, uint32_t res)
{
  memset(p_fcbl, 0, sizeof(T_stfs_fcbl));
  STfs_put_sync_obj();
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_fcbl
  \param start_offset
  \param end_offset

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _STfs_is_area_empty(T_stfs_fcbl *p_fcbl, uint32_t start_offset, uint32_t end_offset)
{
  uint32_t  start_addr =((T_device_mem_map *)p_fcbl->dev_map)[p_fcbl->sector].start_adr + start_offset;
  uint32_t  n =(end_offset - start_offset) / sizeof(uint32_t);
  uint32_t  *ptr;

  if (start_addr % sizeof(uint32_t))
  {
    return STFS_ERROR;
  }

  ptr = (uint32_t *)start_addr;

  for (uint32_t i=0; i < n; i++)
  {
    if (*ptr != EMPTY_PTTRN) return STFS_ERROR;
    ptr++;
  }

  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Инициализация файловой системы

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_init(uint8_t drive_id, T_stfs_info *p_sffs_info)
{
  int32_t      res;

  res = STfs_flash_driver_init();
  if (res != STFS_OK)
  {
    APPLOG("STfs flash driver error %d.", res);
    return res;
  }

  // Проверяем наличие файловой системы во внутренней Data Flash микроконтроллера
  res =  STfs_check(drive_id,p_sffs_info);
  if (res != STFS_OK)
  {
    // Ошибка файловой системы SFFS либо она отсутствует в Data Flash
    APPLOG("STfs disk %d check error %d.",drive_id, res);
    // Форматируем файловую систему
    res = STfs_format(drive_id);
    if (res != STFS_OK)
    {
      APPLOG("STfs disk %d formating error %d.", drive_id, res);
    }
    else
    {
      APPLOG("STfs disk %d formating successsful.", drive_id);
    }
  }
  else
  {
    APPLOG("STfs disk %d verification completed successfully.",drive_id);
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Проверяет ошибки размещения в файловой системе

  \param drive

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_check(uint8_t drive_id, T_stfs_info *p_sffs_info)
{
  T_stfs_file_descriptor   descriptor;
  T_stfs_file_descriptor  *p_descriptor;
  T_stfs_fcbl             *p_fcbl;
  uint32_t                 sector;
  uint32_t                 chunk_start_offset    = 0;
  uint32_t                 chunk_end_offset      = 0;
  uint32_t                 prev_chunk_end_offset = 0;
  uint32_t                 data_end_offset       = 0;
  uint32_t                 sector_type;
  uint32_t                 sz;
  uint32_t                 descriptor_offset;


  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  p_sffs_info->media_size            = 0;  // Общий размер носителя для сохранения файлов и дескрипторов
  p_sffs_info->invalid_aria_size     = 0;  // Общий размер области занатой фрагментами стертых файлов
  p_sffs_info->empty_aria_size       = 0;
  p_sffs_info->valid_aria_size       = 0;  // Общий размер занятый валидными файлами
  p_sffs_info->invalid_chunks_count  = 0;  // Колическто фрагментов стертых файлов
  p_sffs_info->valid_chunks_count    = 0;  // Количестов фрагментов нестертых файлов
  p_sffs_info->file_count            = 0;  // Количество валидных файлов на носителе
  p_sffs_info->invalid_file_count    = 0;
  p_sffs_info->sectors_num           = p_fcbl->number_of_sectors;
  p_sffs_info->phiz_sector_size      =((T_device_mem_map *)p_fcbl->dev_map)[0].last_adr -((T_device_mem_map *)p_fcbl->dev_map)[0].start_adr + sizeof(T_stfs_file_descriptor);
  p_sffs_info->descriptor_size       = sizeof(T_stfs_file_descriptor);

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    p_fcbl->sector = sector;
    sz =((T_device_mem_map *)p_fcbl->dev_map)[sector].last_adr -((T_device_mem_map *)p_fcbl->dev_map)[sector].start_adr;
    p_sffs_info->media_size += sz;

    p_descriptor      = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);
    descriptor_offset = sz;
    data_end_offset   = sz;

    if ((sector_type == STFS_SECTOR_USED) || (sector_type == STFS_SECTOR_RESERVED))
    {
      Mark_Valid_Descriptor(sector, descriptor_offset);   // Отмечаем валидный дескриптор сектора
    }
    else
    {
      if (_STfs_is_area_empty(p_fcbl, 0, data_end_offset) != STFS_OK)
      {
        return _STfs_check_return(p_fcbl,STFS_DIRTY_SECTOR);
      }
      Mark_Empty_Data(sector, 0, data_end_offset);        // Отмечаем пустой сектор
      Mark_Invalid_Descriptor(sector, descriptor_offset); // Отмечаем невалидный дескриптор сектора
      continue;
    }

    if (p_descriptor == 0)
    {
      // Если нет ни одного дескриптора файла в секторе, то весь сектор отмечаем как пустой
      Mark_Empty_Data(sector, 0, data_end_offset);
      continue;
    }
    else
    {
      // Читаем первый дескриптор фрагмента файла
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));

      prev_chunk_end_offset = 0;
      // Проходим по всем занятым дескрипторам
      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        descriptor_offset = (uint32_t)p_descriptor -((T_device_mem_map *)p_fcbl->dev_map)[sector].start_adr;
        chunk_start_offset = descriptor.n.start_addr -((T_device_mem_map *)p_fcbl->dev_map)[sector].start_adr;
        chunk_end_offset   = chunk_start_offset + descriptor.n.size;
        data_end_offset    = chunk_start_offset + descriptor.n.data_size;

        if (chunk_end_offset > descriptor_offset)
        {
          APPLOG("!Error. Wrong bounds. sector=%d, file_id=%d, descr.offs=%08X chunk start offs.=%08X chunk end offs.=%08X chunk size=%d chunk data size=%d", sector, descriptor.n.file_id, descriptor_offset, chunk_start_offset, chunk_end_offset,  descriptor.n.size, descriptor.n.data_size );
          // Верхняя граница фрагмента не может быть выше нижней границы дескриптора
          return _STfs_check_return(p_fcbl,STFS_CHUNK_SIZE_ERR1);
        }
        if (chunk_end_offset % STFS_FLASH_WORD_SIZE)
        {
          APPLOG("!Error. Wrong size. sector=%d, file_id=%d, descr.offs=%08X chunk start offs.=%08X chunk end offs.=%08X chunk size=%d chunk data size=%d", sector, descriptor.n.file_id, descriptor_offset, chunk_start_offset, chunk_end_offset,  descriptor.n.size, descriptor.n.data_size );
          // Не может быть фрагментов с размером не кратным STFS_FLASH_WORD_SIZE
          return _STfs_check_return(p_fcbl,STFS_CHUNK_SIZE_ERR2);
        }
        if (descriptor.n.size == 0)
        {
          APPLOG("!Error. Wrong size 0. sector=%d, file_id=%d, descr.offs=%08X chunk start offs.=%08X chunk end offs.=%08X chunk size=%d chunk data size=%d", sector, descriptor.n.file_id, descriptor_offset, chunk_start_offset, chunk_end_offset,  descriptor.n.size, descriptor.n.data_size );
          // Не может быть фрагментов с размером 0
          return _STfs_check_return(p_fcbl,STFS_CHUNK_SIZE_ERR3);
        }
        if ((descriptor.n.size != STFS_FLASH_WORD_SIZE) && (descriptor.n.number == 0))
        {
          APPLOG("!Error. Wrong file name chunk size . sector=%d, file_id=%d, descr.offs=%08X chunk start offs.=%08X chunk end offs.=%08X chunk size=%d chunk data size=%d", sector, descriptor.n.file_id, descriptor_offset, chunk_start_offset, chunk_end_offset,  descriptor.n.size, descriptor.n.data_size );
          // Фрагмент с номером 0 должен содержать имя файла и его размер фиксирован и равен STFS_FLASH_WORD_SIZE
          return _STfs_check_return(p_fcbl,STFS_CHUNK_SIZE_ERR4);
        }
        if ((descriptor.n.size - descriptor.n.data_size) >= STFS_FLASH_WORD_SIZE)
        {
          APPLOG("!Error. Too big size. sector=%d, file_id=%d, descr.offs=%08X chunk start offs.=%08X chunk end offs.=%08X chunk size=%d chunk data size=%d", sector, descriptor.n.file_id, descriptor_offset, chunk_start_offset, chunk_end_offset,  descriptor.n.size, descriptor.n.data_size );
          // Разница в размере данных и в размере фрагмента не может превышать размера слова STFS_FLASH_WORD_SIZE
          return _STfs_check_return(p_fcbl,STFS_CHUNK_SIZE_ERR5);
        }
        if (chunk_start_offset != prev_chunk_end_offset)
        {
          APPLOG("!Error. Not aligned. sector=%d, file_id=%d, descr.offs=%08X chunk start offs.=%08X chunk end offs.=%08X chunk size=%d chunk data size=%d", sector, descriptor.n.file_id, descriptor_offset, chunk_start_offset, chunk_end_offset,  descriptor.n.size, descriptor.n.data_size );
          // Смещение старта текущего фрагмента должно точно соответствовать смещению конца предыдущего фрагмента
          return _STfs_check_return(p_fcbl,STFS_CHUNK_SIZE_ERR6);
        }

        if (descriptor.e.deleted == STSF_TAG_CHUNK_DELETED)
        {

          p_sffs_info->invalid_aria_size += descriptor.n.size + sizeof(descriptor);
          p_sffs_info->invalid_chunks_count++;
          if (descriptor.n.number == 0) p_sffs_info->invalid_file_count++;
          Mark_Invalid_Descriptor(sector, descriptor_offset);              // Отмечаем невалидный дескриптор
          Mark_Invalid_Data(sector, chunk_start_offset, chunk_end_offset); // Отмечаем невалидные данные
        }
        else if (descriptor.e.deleted == EMPTY_PTTRN)
        {
          p_sffs_info->valid_aria_size += descriptor.n.size + sizeof(descriptor);
          p_sffs_info->valid_chunks_count++;
          if (descriptor.n.number == 0) p_sffs_info->file_count++;
          Mark_Valid_Descriptor(sector, descriptor_offset);              // Отмечаем валидный дескриптор
          Mark_Valid_Data(sector, chunk_start_offset, chunk_end_offset); // Отмечаем валидные данные
        }
        else
        {
          // Неправильный тэг удаленного фрагмента
          return _STfs_check_return(p_fcbl,STFS_INCORRECT_DEL_TAG);
        }

        prev_chunk_end_offset = chunk_end_offset;
        p_descriptor--;
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }
      // Занятые дескрипторы кончились. Дальше идут пустые дескрипторы

      // Проверить что все оставшееся пространство заполнено значением EMPTY_PTTRN
      if (_STfs_is_area_empty(p_fcbl, chunk_end_offset, descriptor_offset) != STFS_OK)
      {
        return _STfs_check_return(p_fcbl,STFS_DIRTY_SECTOR);
      }

    }
    Mark_Empty_Data(sector, chunk_end_offset, descriptor_offset);     // Отмечаем пустую область данных от последнего фрагмента данных до начала последнего дескриптора
  }
  // Общий пустой размер не включает поустые дескриптры на концах цепочек дескрипторов
  p_sffs_info->empty_aria_size = p_sffs_info->media_size - p_sffs_info->invalid_aria_size - p_sffs_info->valid_aria_size - sizeof(T_stfs_file_descriptor)*p_fcbl->number_of_sectors;

  return _STfs_check_return(p_fcbl,STFS_OK);
}

/*-----------------------------------------------------------------------------------------------------
  Дефрагментирует файловую систему

  \param drive

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_defrag(uint8_t drive_id)
{
  T_stfs_fcbl             *p_fcbl;
  T_stfs_fcbl             *p_iop;
  uint32_t                i;
  uint32_t                sector;
  uint32_t                res;
  uint32_t                result = STFS_OK;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE) // Получаем параметры носителя
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  defrag_active = 0;
  // Преверянем нет ли открытых файлов
  for (i = 0, p_iop =&g_fcbls[0]; i < SFFS_NFILE; p_iop++, i++)
  {
    if (p_iop->drive_id != p_fcbl->drive_id)
    {
      continue;
    }
    if ((p_iop->status & (STFS_OPEN_TO_WRITE | STFS_OPEN_TO_READ)))
    {
      STfs_put_sync_obj();
      return STFS_NOT_CLOSED_FILE; // Ошибка поскольку обнаружен открытый файл
    }
  }

  // Стираем все грязные сектора не содержащие актуальных данных, чтобы расчистить место дефрагментатору
  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    if (STfs_lw_is_used_sector_invalid(sector, p_fcbl) == STFS_TRUE)
    {
      res = STfs_lw_erase_sector(sector, p_fcbl);
      if (res != STFS_OK)
      {
        STfs_put_sync_obj();
        return res; // Ошибка стирания сектора
      }
    }
  }


  p_fcbl->file_id       = 0; // Начинаем поиск с файла с идентификатором 0

  // Производим поиск всех актуальных файлов
  // Копирование начинается с более старых файлов.
  while (_STfs_get_next_file_on_defragmentation(p_fcbl) == STFS_TRUE)
  {
    //APPLOG("_______ File to copy id=%d ver=%d.", p_fcbl->file_id, p_fcbl->version);
    // Нашли актуальный файл, копируем его в чистые сектора
    if (_STfs_copy_file_on_defragmetation(p_fcbl) != STFS_OK)
    {
      //APPLOG("_______ Copy error");
      // Если копирование не удалось, то удаляем фрагменты нового скопированного файла в зарезервированных секторах
      res = _STfs_delete_file_on_defragmentation(p_fcbl, STFS_TRUE);
      if (res != STFS_OK)
      {
        //APPLOG("_______ New file delete Error");
        STfs_put_sync_obj();
        return res; // Не удалось удаление нового файла
      }
      else
      {
        //APPLOG("_______ New file delete Ok");
      }
      // Ошибка копирования файла. Нет места или не удалась запись
      result = STFS_FILE_COPY_ERROR;
      break;
    }
    else
    {
      //APPLOG("_______ Copy Ok");
    }
    // Если копирование удалось, то удаляем старый файл
    res = _STfs_delete_file_on_defragmentation(p_fcbl, STFS_FALSE);
    if (res != STFS_OK)
    {
      //APPLOG("_______ Old file delete Error");
      STfs_put_sync_obj();
      return res; // Не удалось удаление старого файла
    }
    else
    {
      //APPLOG("_______ Old file delete Ok");
    }
  }

  // Обновить аттрибут секторов со значения резервированный на значение занятый  и стереть старые сектора
  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    uint32_t sector_type = STfs_get_sector_type(sector, p_fcbl);

    if (sector_type == STFS_SECTOR_RESERVED)
    {
      // Зарезервированные сектора отмечаем как занятые
      if (STfs_write_used_sector_tag(sector, p_fcbl) != STFS_OK)
      {
        result = STFS_ERROR;
      }
    }
    else if (sector_type == STFS_BAD_SECTOR)
    {
      res = STfs_lw_erase_sector(sector, p_fcbl);
      if (res != STFS_OK)
      {
        STfs_put_sync_obj();
        return res; // Фатальная ошибка
      }
    }
  }
  STfs_put_sync_obj();
  return result;
}

/*-----------------------------------------------------------------------------------------------------
  Форматирует носитель файловой системы
  Обязательно вызывать при первом запуске файловой системы
  Форматирование просто стирает все сектора и сбрасывает управляющие структуры файлов

  \param drive

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_format(uint8_t drive_id)
{
  T_stfs_fcbl   *p_fcbl;
  uint32_t       i;
  uint32_t       res;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE)  // Получаем параметры носителя
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  // Сбросить все хэндреры открытых файлов
  for (i = 0; i < SFFS_NFILE; i++)
  {
    if (g_fcbls[i].drive_id != p_fcbl->drive_id) continue;
    if (g_fcbls[i].status & (STFS_OPEN_TO_READ | STFS_OPEN_TO_WRITE)) memset(&g_fcbls[i], 0, sizeof(T_stfs_fcbl));
  }
  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    res = STfs_lw_erase_sector(i, p_fcbl);
    if (res != STFS_OK)
    {
      STfs_put_sync_obj();
      return res; // Ошибка стирания сектора
    }
  }
  STfs_put_sync_obj();
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
   Вычисляет размер свободного пространства в файловой системе

  \param drive

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_free_space(uint8_t drive_id, uint32_t *free_space)
{
  T_stfs_fcbl    *p_fcbl;
  uint32_t        sector;
  uint32_t        freesp = 0;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  for (sector = 0; sector < p_fcbl->number_of_sectors; sector++)
  {
    p_fcbl->sector = sector;
    freesp += STfs_lw_obtain_free_space_limits(sector, p_fcbl);
  }
  *free_space = freesp;
  STfs_put_sync_obj();
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Закрываем файл

  \param handle

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_close(int32_t fcbl_indx)
{
  T_stfs_fcbl             *p_fcbl;
  uint32_t                 res = STFS_OK;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  p_fcbl =&g_fcbls[fcbl_indx];
  if ((p_fcbl->status & STFS_OPEN_TO_WRITE) && (p_fcbl->status & STFS_NEED_DESCRIPTOR))
  {
    res = STfs_lw_write_descriptor(p_fcbl);
  }
  memset(p_fcbl, 0, sizeof(T_stfs_fcbl));
  STfs_put_sync_obj();
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Удаляет файл

  \param filename

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_delete(uint8_t drive_id, const char *filename)
{
  T_stfs_fcbl    *p_fcbl;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  if (STfs_lw_find_file_by_name(filename, p_fcbl) == STFS_OK)
  {
    if (_STfs_delete_file_and_erase_sectors(p_fcbl, 0) == STFS_OK)
    {
      STfs_put_sync_obj();
      return STFS_OK;
    }
    else
    {
      STfs_put_sync_obj();
      return STFS_ERROR;
    }
  }
  STfs_put_sync_obj();
  return STFS_FILE_NOT_FOUND;
}

/*-----------------------------------------------------------------------------------------------------
   Открываем файл
   Возвращает индекс управлющей структуры файла если функция выполнилась успешно
   В случае неудачи возвращает код ошибки

  \param fname
  \param openmode

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_open(uint8_t drive_id, const char *file_name, int32_t openmode, int32_t *p_fcbl_indx)
{

  uint32_t        i;
  T_stfs_fcbl    *p_fcbl;
  uint32_t        version;
  uint32_t        res;

  if (p_fcbl_indx == 0) return STFS_ERRONEOUS_ARGUMENT;
  *p_fcbl_indx = -1;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl           =&g_fcbls[ix];
  *p_fcbl_indx     = ix;
  p_fcbl->drive_id = drive_id;

  // Переносим сведения о носителе в управляющую структуру
  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    p_fcbl->status = 0;
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  if (openmode & (STFS_OPEN_WRITE | STFS_OPEN_APPEND))
  {
    p_fcbl->status |= STFS_OPEN_TO_WRITE;
  }
  else
  {
    p_fcbl->status |= STFS_OPEN_TO_READ;
  }

  if (openmode & STFS_OPEN_APPEND)
  {
    p_fcbl->status |= STFS_OPEN_TO_APPEND;
  }


  if (STfs_lw_find_file_by_name(file_name, p_fcbl) == STFS_OK)
  {
    version = p_fcbl->version;
    // Файл был найден.
    // Проверить не был ли уже открыт этот файл.
    for (i = 0; i < SFFS_NFILE; i++)
    {
      if (i == ix) continue;

      if (g_fcbls[i].status & (STFS_OPEN_TO_READ | STFS_OPEN_TO_WRITE))
      {
        if (g_fcbls[i].file_id == p_fcbl->file_id)
        {
          if ((g_fcbls[i].status & STFS_OPEN_TO_WRITE) || (p_fcbl->status & STFS_OPEN_TO_WRITE))
          {
            // Ошибка, поскольку на запись файл можно открыть только один раз
            //  и нельзя открыть на чтение файл уже открытый на запись
            //  или открыть на запись файл уже открытый на чтение
            p_fcbl->status = 0;
            STfs_put_sync_obj();
            return STFS_PROHIBITED_OPERATION;
          }
        }
      }
    }

    if (p_fcbl->status & STFS_OPEN_TO_APPEND)
    {
      // !!! Требует реализации !!!
      p_fcbl->status = 0;
      STfs_put_sync_obj();
      return STFS_PROHIBITED_OPERATION;
    }

    // Если файл с таким именем уже существует и новый файл открывается на запись, то удалить уже существующий файл и создать файл заново
    if (p_fcbl->status & STFS_OPEN_TO_WRITE)
    {
      res = _STfs_delete_file_and_erase_sectors(p_fcbl, 0);
      if (res != STFS_OK)
      {
        p_fcbl->status = 0;
        STfs_put_sync_obj();
        return res;
      }
      version++;
      p_fcbl->version = version;
      if (STfs_lw_create(file_name, p_fcbl) != STFS_OK)
      {
        p_fcbl->status = 0;
        STfs_put_sync_obj();
        return STFS_FILE_CREATE_ERROR1;
      }
      STfs_put_sync_obj();
      return STFS_OK;
    }

    // Файл найден и открывается на чтение
    // Установить начало и конец фрагмента с именем файла в управляющей структуре
    p_fcbl->chunk_number = 1;
    if (STfs_lw_get_file_chunk_for_read(p_fcbl) == STFS_FALSE)
    {
      p_fcbl->status = 0;
      STfs_put_sync_obj();
      return STFS_FILE_LOCATION_ERROR;
    }
    STfs_put_sync_obj();
    return STFS_OK;
  }
  else
  {
    // Файл не был найден.

    if (p_fcbl->status & STFS_OPEN_TO_READ)
    {
      // Возврат ошибки если пытались открыть на чтение
      p_fcbl->status = 0;
      STfs_put_sync_obj();
      return STFS_FILE_NOT_FOUND;
    }
    p_fcbl->file_id = STfs_lw_get_freeID(p_fcbl);
    p_fcbl->version = 0;

    // Создание нового файла если пытались открыть на запись
    if (STfs_lw_create(file_name, p_fcbl) != STFS_OK)
    {
      p_fcbl->status = 0;
      STfs_put_sync_obj();
      return STFS_FILE_CREATE_ERROR2;
    }
    STfs_put_sync_obj();
    return STFS_OK;
  }
}

/*-----------------------------------------------------------------------------------------------------
   Чтение из файла
   Возвращаем размер прочитанных данных в переменной actual_read

  \param fcbl_indx
  \param data_buffer
  \param data_size
  \param actual_read

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_read(int32_t fcbl_indx, uint8_t *data_buffer, int32_t data_size, int32_t *actual_read)
{

  int32_t         unread_size;
  int32_t         must_be_read;
  int32_t         left_to_read;
  int32_t         sz;
  T_stfs_fcbl    *p_fcbl;


  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  left_to_read = data_size;
  must_be_read = left_to_read;
  do
  {
    p_fcbl =&g_fcbls[fcbl_indx];
    if (p_fcbl->end_addr == p_fcbl->curr_addr)
    {
      // Поиск очередного фрагмента файла
      if (STfs_lw_get_file_chunk_for_read(p_fcbl) == STFS_FALSE)
      {
        if (actual_read != 0) *actual_read = must_be_read - left_to_read;
        STfs_put_sync_obj();
        return STFS_INCOMPLETE_READING;
      }
    }
    unread_size = p_fcbl->end_addr - p_fcbl->curr_addr;       // Вычисляем размер фрагмента
    if (unread_size < left_to_read)
    {
      // Если размер фрагмента меньше требуемого буфера чтения
      sz   = unread_size;
    }
    else
    {
      sz = left_to_read;
    }
    STfs_lw_read_data(p_fcbl->curr_addr, data_buffer, sz);
    p_fcbl->curr_addr += sz;
    data_buffer       += sz;
    left_to_read      -= sz;
  }while (left_to_read != 0);

  if (actual_read != 0) *actual_read = must_be_read;
  STfs_put_sync_obj();
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
   Запись в файл

  \param fcbl_indx
  \param data_buffer
  \param data_size

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_write(int32_t fcbl_indx, const uint8_t *data_buffer, int32_t data_size)
{

  T_stfs_fcbl       *p_fcbl;
  int32_t            sz;
  uint32_t           res;

  if (data_size == 0) return STFS_OK;
  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  p_fcbl =&g_fcbls[fcbl_indx];

  do
  {
    // Свободная область должна вмещать данные и два дескриптора: один для фрагмента файла и один пустой, заканчивающий цепочку дескрипторов
    if (p_fcbl->end_addr >  (p_fcbl->curr_addr + 2 * sizeof(T_stfs_file_descriptor)))
    {
      // Находим размер области доступной для записи данных
      sz = p_fcbl->end_addr - p_fcbl->curr_addr - 2 * sizeof(T_stfs_file_descriptor);
      if (sz > data_size) sz = data_size;
      res = STfs_lw_write_data(p_fcbl, (void *)data_buffer, sz);
      if (res != STFS_OK)
      {
        STfs_put_sync_obj();
        return STFS_ERROR;
      }
#ifdef SFFS_ENABLE_TRACE_WRITES
      LOG_STFS_DATA_WRITE;
#endif
      data_size         -= sz;
      data_buffer       += sz;
      p_fcbl->curr_addr += sz;
    }

    if (data_size > 0)
    {
      // Если необходимо записать новые данные, но места в секторе нет и предыдущий блок данных уже был зафиксирован дескриптором
      // то дескриптор не записывать и сразу искать следующий сектор
      if (p_fcbl->curr_addr == p_fcbl->start_addr)
      {
        res = STfs_lw_alloc_next_sector(p_fcbl, sizeof(T_stfs_file_descriptor)+1);
        // Здесь когда в секторе не было найдено достаточно места и был выполне переход на следующий сектор
        if (res != STFS_OK)
        {
          // Здесь когда не удалось найти свободное место для записи данных
          STfs_put_sync_obj();
          return res;
        }
      }
      else
      {
        // Здесь когда нет места в текущем секторе для данных, но не все предыдущие записанные данные зафиксированы дескриптором
        res = STfs_lw_write_descriptor(p_fcbl);
        if (res == STFS_OK)
        {
          res = STfs_lw_alloc_next_sector(p_fcbl, sizeof(T_stfs_file_descriptor)+1);
          // Здесь когда в секторе не было найдено достаточно места и был выполне переход на следующий сектор
          if (res != STFS_OK)
          {
            // Здесь когда не удалось найти свободное место для записи данных
            STfs_put_sync_obj();
            return res;
          }
        }
        else
        {
          // Здесь когда не удалось записать дескриптор
          STfs_put_sync_obj();
          return STFS_ERROR;
        }
      }
    }

  } while (data_size > 0);

  p_fcbl->status |= STFS_NEED_DESCRIPTOR; // Устанавливаем флаг необходимости записи дескриптора
  STfs_put_sync_obj();
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
   Установить позицю чтения в файле от его начала на значение target_pos

  \param handle
  \param target_pos

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_setpos(int32_t fcbl_indx, uint32_t target_pos)
{
  T_stfs_file_descriptor   descriptor;
  T_stfs_file_descriptor  *p_descriptor;
  T_stfs_fcbl             *p_fcbl;
  uint32_t                 sector;
  uint32_t                 chunk_num;
  uint32_t                 i;
  uint32_t                 current_pos;
  uint32_t                 sector_type;
  uint32_t                 res;


  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  chunk_num    = 0; // Начинаем с первого фрагмента файла
  current_pos  = 0; // Начинаем с позиции 0
  p_fcbl       =&g_fcbls[fcbl_indx];
  sector       = p_fcbl->sector;

nextbl:

  // Проход по секторам по кольцу
  for (i = 0; i < p_fcbl->number_of_sectors; i++)
  {
    // Читаем дескриптор сектора
    p_descriptor = STfs_lw_get_first_fdescriptor_used(sector, p_fcbl,&sector_type);

    if (p_descriptor != 0)
    {
      STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));

      while (descriptor.n.file_id != EMPTY_PTTRN)
      {
        if ((descriptor.n.file_id == p_fcbl->file_id) &&  (descriptor.n.number == chunk_num) && (descriptor.e.deleted == EMPTY_PTTRN))
        {
          chunk_num++;
          if ((current_pos + descriptor.n.data_size) < target_pos)
          {
            // Если текущий счетчик позиции + размер фрагмента меньше заданной позиции,
            // то продолжать искать фрагменты от начала текущего сектора,
            // поскольку в результате фрагментаций и дефрагментаций порядок следования фрагментов может быть произвольным
            current_pos += descriptor.n.data_size;
            goto nextbl;
          }

          // Текущий счетчик позиции + размер фрагмента больше заданной позиции, то это тот фрагмент где надо остановиться
          p_fcbl->sector       = sector;    // Запоминаем сектор
          p_fcbl->chunk_number = chunk_num; // Запоминаем номер фрагмента
          if (p_fcbl->status & STFS_OPEN_TO_APPEND)
          {
            res = STfs_lw_alloc_next_sector(p_fcbl, sizeof(T_stfs_file_descriptor)+1);
            if (res != STFS_OK)
            {
              STfs_put_sync_obj();
              return res;
            }
          }
          else
          {
            p_fcbl->start_addr = descriptor.n.start_addr;
            p_fcbl->curr_addr  = descriptor.n.start_addr + target_pos - current_pos; // Устанавливаем текущую позицию чтения
            p_fcbl->end_addr   = descriptor.n.start_addr + descriptor.n.data_size;   // Устанавливаем позицию конца фрагмента данных
          }
          STfs_put_sync_obj();
          return STFS_OK;
        }

        // Продолжаем искать в секторе следующий фрагмент заданного файла
        p_descriptor--;
        STfs_lw_read_data((uint32_t)p_descriptor, (uint8_t *)&descriptor, sizeof(descriptor));
      }
    }

    // Организуем кольцевой обход секторов
    sector++;
    if (sector == p_fcbl->number_of_sectors) sector = 0;
  }

  STfs_put_sync_obj();
  return STFS_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
   Переименование файла

  \param oldname
  \param newname

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_rename(uint8_t drive_id, const char *oldname, const char *newname)
{
  T_stfs_fcbl    *p_fcbl;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;


  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  if (STfs_lw_find_file_by_name(newname, p_fcbl) == STFS_OK)
  {
    STfs_put_sync_obj();
    return STFS_FILE_ALREADY_EXIST; // Файл с новым именем уже существует
  }
  if (STfs_lw_find_file_by_name(oldname, p_fcbl) == STFS_OK)
  {
    if  (_STfs_rename_file(newname, p_fcbl) == 0)
    {
      STfs_put_sync_obj();
      return STFS_OK;
    }
    else
    {
      STfs_put_sync_obj();
      return STFS_ERROR;
    }
  }

  // Файл не найден
  STfs_put_sync_obj();
  return STFS_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
   Получаем размер файла

  \param handle

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t STfs_len(int32_t fcbl_indx)
{
  uint32_t sz;
  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  sz = _STfs_get_file_size(&g_fcbls[fcbl_indx], STFS_FALSE);

  STfs_put_sync_obj();
  return sz;
}

/*-----------------------------------------------------------------------------------------------------
   Поиск файла с именем по указанному строковому шаблону
   Структура file_info на входе должны быть пустая
   На выходе структура получает заполненые поля с именем файла, его размером и идентификатором

  \param pattern
  \param file_info

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_find(uint8_t drive_id, const char *pattern, T_file_info *file_info)
{
  T_stfs_fcbl    *p_fcbl;
  int32_t         len1;
  int32_t         len2;
  int32_t         len;
  uint32_t        star;  // Флаг звездочки
  uint32_t        res;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl =&g_fcbls[ix];
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  if (strcmp(pattern, "*.*") == 0)
  {
    star = STFS_TRUE;
    len1 = len2 = 0;
  }
  else
  {
    // Ищем позицию символа '*' в шаблоне  поиска
    len1 = _STfs_find_sym_pos_in_string((const char *)pattern, '*');
    len2 = strlen(pattern);
    if (len1 == -1)
    {
      star = STFS_FALSE; // Звездочка не найдена в паттерне
    }
    else
    {
      star  = STFS_TRUE;
      len2  -=(len1 + 1);
    }
  }
  memset(file_info, 0, sizeof(T_file_info));
  for (;;)
  {
    // Последовательный обход всех существующих нестертых файлов
    res = _STfs_find_next_file_by_id(file_info, p_fcbl);
    if (res != STFS_OK)
    {
      STfs_put_sync_obj();
      return STFS_FILE_NOT_FOUND;
    }
    if (star == STFS_FALSE)
    {
      // Здесь если в шаблоне нет символа '*'.
      // Совпадение должно быть посимвольным
      if (strcmp(file_info->name, pattern) == 0) goto exit_found;
    }
    else
    {

      if (len1 == 0 && len2 == 0)
      {
        // Если шаблон задан в виде '*.*' то возвращаем первый же найденный файл
        goto exit_found;
      }
      if (len1 && memcmp(file_info->name, pattern, len1) != 0)
      {
        // Если символ '*' находится в позиции большей чем 0, то игнорируем файлы с несовпадающими первыми символами расположенными до символа '*'
        continue;
      }
      if (len2 == 0)
      {
        // Если символ '*' находится в конце, то дальше проверять не надо, файл найден поскольку первые символы совпадают
        goto exit_found;
      }
      len = strlen((char *)file_info->name);
      if (len < len1 + len2)
      {
        // Игнорируем найденный файл, поскольку длина его имени меньше суммы длин первой и второй половины шаблона
        continue;
      }
      if (memcmp(&file_info->name[len - len2],&pattern[len1 + 1], len2) == 0)
      {
        // Файл найден поскольку совпали обе половины шаблона
        goto exit_found;
      }
    }
  }

exit_found:
  p_fcbl->file_id = file_info->file_id;
  p_fcbl->sector  = 0;
  file_info->size = _STfs_get_file_size(p_fcbl, STFS_FALSE);
  STfs_put_sync_obj();
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Поиск файла в системе с наименьшим fid
  Если функция возвращает SFFS_OK, то после нее надо обязательно вызывать Sffs_find_next_file, пока та не вернет  SFFS_FILE_NOT_FOUND

  \param p_fcbl_indx
  \param file_info

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_find_first_file(uint8_t drive_id, uint32_t *p_fcbl_indx, T_file_info *file_info)
{
  T_stfs_fcbl    *p_fcbl;
  uint32_t       res;


  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  int32_t ix = _STfs_find_free_fcbl();
  if (ix == STFS_EOF) // Ищем свободный управляющий блок
  {
    STfs_put_sync_obj();
    return STFS_NO_FREE_FCBL;
  }
  p_fcbl           =&g_fcbls[ix];
  *p_fcbl_indx     = ix;
  p_fcbl->drive_id = drive_id;

  if (_STfs_set_params(p_fcbl) == STFS_FALSE)
  {
    STfs_put_sync_obj();
    return STFS_BAD_DRIVE;
  }

  file_info->file_id = 0;
  p_fcbl->status     = STFS_OPEN_TO_READ; // Отметим fcbl  как занятый

  // Последовательный обход всех существующих нестертых файлов
  res = _STfs_find_next_file_by_id(file_info, p_fcbl);
  if (res != STFS_OK)
  {
    memset(p_fcbl, 0, sizeof(T_stfs_fcbl)); // Отметим fcbl  как незанятый
    STfs_put_sync_obj();
    return STFS_FILE_NOT_FOUND;
  }
  p_fcbl->file_id = file_info->file_id;
  file_info->size = _STfs_get_file_size(p_fcbl, STFS_FALSE);
  STfs_put_sync_obj();
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Поиск следующего файла по нарастанию fid
  Функция вызывается после Sffs_find_first_file

  \param fcbl_indx
  \param file_info

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_find_next_file(uint32_t fcbl_indx, T_file_info *file_info)
{
  uint32_t      res;
  T_stfs_fcbl   *p_fcbl =&g_fcbls[fcbl_indx];

  res = _STfs_find_next_file_by_id(file_info, p_fcbl);
  if (res != STFS_OK)
  {
    memset(p_fcbl, 0, sizeof(T_stfs_fcbl)); // Отметим fcbl  как незанятый
    STfs_put_sync_obj();
    return STFS_FILE_NOT_FOUND;
  }
  p_fcbl->file_id = file_info->file_id;
  file_info->size = _STfs_get_file_size(p_fcbl, STFS_FALSE);
  STfs_put_sync_obj();
  return STFS_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Запись дескриптора фрагмента файла после последней файловой записи
  Запись в файл не вызывает автоматически запись дескриптора,
  поэтому для того чтобы запись не осталась надолго в незаконченном состоянии вызывают эту функцию

  \param handle

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t STfs_flush(int32_t fcbl_indx)
{
  T_stfs_fcbl             *p_fcbl;
  uint32_t                res;

  if (STfs_get_sync_obj() != STFS_OK) return STFS_ACCESS_ERROR;

  p_fcbl =&g_fcbls[fcbl_indx];
  if ((p_fcbl->status & STFS_OPEN_TO_WRITE) == 0)
  {
    STfs_put_sync_obj();
    return STFS_ERROR;
  }

  if (p_fcbl->status & STFS_NEED_DESCRIPTOR)
  {
    res = STfs_lw_write_descriptor(p_fcbl);
    p_fcbl->status &= ~STFS_NEED_DESCRIPTOR;
    p_fcbl->start_addr += p_fcbl->chunk_size;
    p_fcbl->curr_addr   = p_fcbl->start_addr;
  }
  STfs_put_sync_obj();
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Освободить контрольные блоки захваченные задачей

  \param task_id

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t  STfs_free_task_cbls(uint32_t task_id)
{
  T_stfs_fcbl  *p_fcbl;
  uint32_t      i;

  p_fcbl =&g_fcbls[0];
  for (i = 0; i < SFFS_NFILE; i++)
  {
    if (p_fcbl->p_thread_id == task_id)
    {
      p_fcbl->status = 0;
    }
    p_fcbl++;
  }
  return STFS_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param data_size

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  STfs_estimate_file_phis_space(uint32_t data_size)
{
  return  data_size +(sizeof(T_stfs_file_descriptor) * 8)+ sizeof(uint32_t) + sizeof(T_stfs_file_descriptor)+ STFS_FLASH_WORD_SIZE + STFS_FLASH_WORD_SIZE;
}
