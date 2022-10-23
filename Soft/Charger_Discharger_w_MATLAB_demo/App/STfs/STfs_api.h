#ifndef __SFFS_API
  #define __SFFS_API

  #ifdef __cplusplus
extern "C"
{
  #endif


  // Режимы открытия файла
#define STFS_OPEN_READ   0  // Открыть на чтение
#define STFS_OPEN_WRITE  4  // Открыть на запись
#define STFS_OPEN_APPEND 8  // Открыт в режиме append

#define STFS_OK                    0
#define STFS_ERROR                 1
#define STFS_NO_FREE_FCBL          2
#define STFS_BAD_DRIVE             3
#define STFS_NOT_CLOSED_FILE       4
#define STFS_FILE_COPY_ERROR       5
#define STFS_FILE_NOT_FOUND        6
#define STFS_PROHIBITED_OPERATION  7
#define STFS_ERRONEOUS_ARGUMENT    8
#define STFS_FILE_CREATE_ERROR1    9
#define STFS_FILE_CREATE_ERROR2    10
#define STFS_FILE_LOCATION_ERROR   11
#define STFS_INCOMPLETE_READING    12
#define STFS_SECTOR_ALLOC_ERROR    13
#define STFS_ACCESS_ERROR          14
#define STFS_FILE_ALREADY_EXIST    15
#define STFS_FATAL_ERROR           16
#define STFS_BAD_FILE_NAME         17
#define STFS_CHUNK_DELETE_ERROR    18


#define STFS_CHUNK_SIZE_ERR1       101
#define STFS_CHUNK_SIZE_ERR2       102
#define STFS_CHUNK_SIZE_ERR3       103
#define STFS_CHUNK_SIZE_ERR4       104
#define STFS_CHUNK_SIZE_ERR5       105
#define STFS_CHUNK_SIZE_ERR6       106
#define STFS_INCORRECT_DEL_TAG     107
#define STFS_DIRTY_SECTOR          108

#define STFS_SECTOR_ERASE_ERROR1   201
#define STFS_SECTOR_ERASE_ERROR2   202
#define STFS_SECTOR_ERASE_ERROR3   203

#define STFS_FLASH_PROGR_ERROR1    301
#define STFS_FLASH_PROGR_ERROR2    301
#define STFS_FLASH_PROGR_ERROR3    301



  // Структура для поиска файла
  typedef struct
  {
      char       name[STFS_FLASH_WORD_SIZE+1];       // Имя файла
      uint32_t   size;                          // Размер файла в байтах
      uint32_t   file_id;                       // Числовой идентификатор файла
      uint32_t   file_name_sector;
      uint32_t   ver;
  } T_file_info;

  // Структура статистики файловой системы
  typedef struct
  {
      uint32_t media_size;           // Полный размер пространства на носителе
      uint32_t invalid_aria_size;    // Размер пространства занятого стертыми файлами
      uint32_t empty_aria_size;      // Размер пространства пригодный для записи
      uint32_t valid_aria_size;      // Размер пространства занятого актуальными файлами
      uint32_t invalid_chunks_count; // Количество фрагментов стертых файлов
      uint32_t valid_chunks_count;   // Количество фрагментов файлов
      uint32_t file_count;           // Количество актуальных файлов
      uint32_t invalid_file_count;   //
      uint32_t sectors_num;          // Количестов стираемых секторов на носителе
      uint32_t phiz_sector_size;     // Физический размер сектора
      uint32_t descriptor_size;      // Размер дескриптора

  } T_stfs_info;


  extern int32_t           STfs_init(uint8_t drive_id, T_stfs_info *p_sffs_info);
  extern int32_t           STfs_find(uint8_t drive_id, const char *pattern, T_file_info *info);
  extern int32_t           STfs_find_first_file(uint8_t drive_id, uint32_t *p_fcbl_indx, T_file_info *file_info);
  extern int32_t           STfs_find_next_file(uint32_t fcbl_indx, T_file_info *file_info);


  extern int32_t           STfs_open(uint8_t drive_id, const char *fname, int32_t openmode, int32_t *p_fcbl_indx);
  extern int32_t           STfs_rename(uint8_t drive_id, const char *oldname, const char *newname);
  extern int32_t           STfs_read(int32_t fcbl_indx, uint8_t *data_buffer, int32_t data_size, int32_t *actual_read);
  extern int32_t           STfs_write(int32_t fcbl_indx, const uint8_t *data_buffer, int32_t data_size);
  extern int32_t           STfs_setpos(int32_t fcbl_indx, uint32_t target_pos);
  extern uint32_t          STfs_len(int32_t fcbl_indx);
  extern int32_t           STfs_close(int32_t fcbl_indx);
  extern int32_t           STfs_delete(uint8_t drive_id, const char *filename);
  extern int32_t           STfs_flush(int32_t fcbl_indx);

  extern uint32_t          STfs_free_space(uint8_t drive_id, uint32_t *free_space);
  extern int32_t           STfs_format(uint8_t drive_id);
  extern int32_t           STfs_check(uint8_t drive_id, T_stfs_info *p_sffs_info);
  extern int32_t           STfs_defrag(uint8_t drive_id);

  extern int32_t           STfs_free_task_cbls(uint32_t task_id);
  extern uint32_t          STfs_estimate_file_phis_space(uint32_t data_size);

#ifdef __cplusplus
}
#endif

#endif
