#ifndef __SFFS_INT
  #define __SFFS_INT

  #ifdef __cplusplus
extern "C"
{
  #endif

#define SFFS_NFILE        5   // Максимальное количество открытых файлов в файловой системе


#ifndef NULL
#ifdef __cplusplus              // EC++
#define NULL   0
#else
#define NULL   ((void *) 0)
#endif
#endif

  //#define SFFS_ENABLE_TRACE_WRITES
#define LOG_STFS_DESCRIPTOR_WRITE  APPLOG("~~~~~ Write descriptor in sector %d at %04X. file_id=%d, chunk=%d, %s %d", p_fcbl->sector, p_fcbl->end_addr, descriptor.n.file_id, descriptor.n.number, __FUNCTION__, __LINE__ );
#define LOG_STFS_DATA_WRITE        APPLOG("----- Write data in sector %d from  %04X to %04X, %s %d", p_fcbl->sector,  p_fcbl->curr_addr, p_fcbl->curr_addr +size, __FUNCTION__, __LINE__);

#define STFS_EOF                   -1
#define STFS_TRUE                   1
#define STFS_FALSE                  0

#define STFS_SECTOR_EMPTY           0
#define STFS_SECTOR_RESERVED        1
#define STFS_SECTOR_USED            2
#define STFS_BAD_SECTOR             3


#define INT_FLASH_BANK2             1                  // Идентификатор  носителя файловой системы
#define STFS_FLASH_WORD_SIZE        32                 // Размер слова записываемого во Flash
#define STFS_TMPBUFLEN              (8*STFS_FLASH_WORD_SIZE)

#define STSF_TAG_RESERVED_SECTOR    0x14789632
#define STSF_TAG_USED_SECTOR        0x12369874

#define STSF_TAG_CHUNK_DELETED      0x25896541


  // Тип описателя карты секторов Flash памяти. Сектор - это стираемая единица Flash памяти
  typedef struct
  {
      uint32_t start_adr;
      uint32_t last_adr;

  } T_device_mem_map;


  // Управляющая структура файла
  typedef struct
  {
      uint32_t   file_id;               // Идентификационный номер файла
      uint16_t   status;                // Статус файла
      uint16_t   drive_id;              // Тип носителя
      uint32_t   number_of_sectors;     // Количество секторов
      void       *dev_map;              // Указатель на карту секторов носителя
      uint32_t   sector;                // Номер текущего сектора текущего фрагмента файла
      uint32_t   chunk_number;          // Номер текущего фрагмента файла. Номер фрагмента равен 0 у фрагмента с названием и у первого фрагмента с данными
      int32_t    end_addr;              // Верхний адрес записываемого или читаемого фрагмента
      int32_t    curr_addr;             // Текущий алрес чтения или записи
      int32_t    start_addr;            // Начальный адрес читаемого или записываемого фрагмента
      uint32_t   version;               // Номер версии дескриптора. Используется для безопасного переименования файла. По номеру версии определяется какая весия актуальна.
      uint32_t   chunk_size;            // Размер фрагмента данных. В общем случае он не равен количеству данных во фрагменте.
      uint8_t    prog_word_buf[STFS_FLASH_WORD_SIZE]; // Буфер хранения слова программирования Flash
      uint8_t    prog_word_cnt;                       // Счетчик заполненности слова программирования Flash
      uint32_t   p_thread_id;
  } T_stfs_fcbl;

  // Определения флагов в статусе
#define STFS_OPEN_TO_READ         0x0001
#define STFS_OPEN_TO_WRITE        0x0002
#define STFS_OPEN_TO_APPEND       0x0004
#define STFS_NEED_DESCRIPTOR      0x0008


  // Каждый сектор может быть в одном из следующих состояний:
  // CLEAR     чистый             - в секторе не записывалось еще ни одино 32 байтное слово        . Идентифицируется по чистому дескриптору сектора
  // RESERVED  зарезервированный  - в секторе производится запись дефрагментированных файлов       . Идентифицируется по штампу "зарезервированный" и отсутствию штампа "используемый"
  // USED      используемый       - в секторе уже произведена запись актуальных файлов             . Идентифицируется по штампу "используемый"
  // FULL      заполненный (не используется) - в секторе записаны файлы и больше ничего в него записать нельзя. Идентифицируется путем сканирования свободного места



  // Дескриптор блока данных
  // Дескрипторы записываются один за другим от самого верхнего адреса сектора вниз по убыванию адресов.
  // Каждая запись фрагмента файла сопровождается записью дескриптора.
  // Самый верхний дескриптор - служебный и содержит флаги состояния всего сектора
  // Фрагменты файлов записываются от младших адресов сектора вверх.  Первый фрагмент файла содержит запись с именем файла
  // Не рекомедуется записывать в файлы маленькие блоки, поскольку любая запись ведет к созданию дополнительной записи размещения !!!


  // Размер структуры дескриптора сектора - 64 байта
  // Дескриптор состоит из двух 32 байтных частей.
  // Вторая часть дописывается когда в дескрипторе ставится штамп используемого сектора после того как он был в состоянии зарезервирован

  typedef struct
  {
      uint32_t   sector_reserved;
      uint32_t   reserv2[7];
  }
  T_stfs_reserved_sector_descriptor;

  typedef struct
  {
      uint32_t   sector_used;
      uint32_t   reserv2[7];
  }
  T_stfs_used_sector_descriptor;

  typedef  struct
  {
      T_stfs_used_sector_descriptor     u;
      T_stfs_reserved_sector_descriptor r;
  } T_stfs_sector_descriptor;

  // Размер структуры дескриптора фрагмента файла - 64 байта
  // Дескриптор состоит из двух 32 байтных частей.
  // Вторая часть дописывается когда в дескрипторе ставится тэг(штамп) стертого файла
  typedef struct
  {
      uint32_t   file_id;       // Идентификационный номер файла. Если здесь находится EMPTY_PTTRN, то это чистый блок без данных о фрагменте
      int32_t    start_addr;    // Абсолютный адрес начала фрагмента
      int32_t    size;          // Размер фрагмента. Запись ведется 32 байтными блоками поэтому этот размер должен быть кратным 32
      uint32_t   number;        // Порядковый номер фрагмента, начинается с 0. В фрагменте с номером 0 хранится имя файла
      int32_t    data_size;     // Количество данных реально записанных во фрагменте
      uint32_t   version;       // Номер версии файла. Используется для безопасного переименования файла. По номеру версии определяется какая версия актуальна.
      uint32_t   reserv1[2];

  } T_stfs_file_normal_descriptor;

  typedef struct
  {
      uint32_t   deleted;      // Тэг удаленного файла
      uint32_t   reserv2[7];
  } T_stfs_file_erase_descriptor;


  typedef  struct
  {
      T_stfs_file_erase_descriptor  e;
      T_stfs_file_normal_descriptor n;
  } T_stfs_file_descriptor;


  T_stfs_file_descriptor*  STfs_lw_get_first_fdescriptor_used(uint32_t sector, T_stfs_fcbl *p_fcbl, uint32_t *p_sector_type);
  T_stfs_file_descriptor*  STfs_lw_get_first_fdescriptor_resv(uint32_t sector, T_stfs_fcbl *p_fcbl, uint32_t *p_sector_type);
  uint32_t                 STfs_get_sector_type(uint32_t sector, T_stfs_fcbl *p_fcbl);

  uint32_t                 STfs_lw_write_file_name_data(uint32_t adr, void *buf, uint32_t cnt);
  uint32_t                 STfs_lw_write_data(T_stfs_fcbl *p_fcbl, void *buf, uint32_t cnt);
  uint32_t                 STfs_lw_write_data_rest(T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_write_used_sector_tag(uint32_t sector, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_write_reserved_sector_tag(uint32_t sector, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_write_deleted_chunk_tag(T_stfs_file_descriptor *p_src_descriptor, uint32_t dest_addr);

  uint32_t                 STfs_lw_read_data(uint32_t adr, void *buf, uint32_t sz);
  int32_t                  STfs_lw_get_flash_mem_params(void **pdev_map, uint32_t *number_of_sectors);
  uint32_t                 STfs_lw_write_descriptor(T_stfs_fcbl *p_fcbl);


  uint16_t                 STfs_lw_get_freeID(T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_obtain_free_space_limits(uint32_t sector, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_is_sector_used_for_write(uint32_t sector, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_get_file_chunk_for_read(T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_find_file_by_name(const char *name, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_alloc_next_sector(T_stfs_fcbl *p_fcbl, uint32_t needed_space);
  uint32_t                 STfs_lw_is_used_sector_invalid(uint32_t sector, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_is_resv_sector_invalid(uint32_t sector, T_stfs_fcbl *p_fcbl);
  uint32_t                 STfs_lw_erase_sector(uint32_t sector, T_stfs_fcbl *p_fcbl);
  int32_t                  STfs_lw_create(const char *fname, T_stfs_fcbl *p_fcbl);

#ifdef __cplusplus
}
#endif


#endif
