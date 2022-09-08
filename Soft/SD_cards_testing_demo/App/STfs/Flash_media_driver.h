#ifndef SFFS_FLASH_DRIVER_H
  #define SFFS_FLASH_DRIVER_H

  #ifdef __cplusplus
extern "C"
{
  #endif

#define  EMPTY_PTTRN            (0xFFFFFFFF)

#ifdef ENABLE_SECTORS_ERASING_LOG
#define STFS_ERASELOG(...)  APPLOG(...)
#else
#define STFS_ERASELOG(...)
#endif


  typedef struct
  {
      unsigned int start_adr;
      unsigned int end_adr;
  }
  T_flash_sect_map;

  typedef struct
  {
      unsigned int                 flash_id;
      unsigned int                 flash_sect_size;
      unsigned int                 flash_sect_num;
      const T_flash_sect_map      *sect_map;
  }
  T_DataFlash_configuration;

  typedef struct
  {
      uint32_t stfs_last_flash_progr_err;
      uint32_t addr;
      uint32_t stfs_last_sec_erasing_err;
      uint32_t sector;
      uint64_t min_sec_erasing_time;
      uint64_t max_sec_erasing_time;
  }
  T_DataFlash_driver_stat;

  T_DataFlash_driver_stat  *FlashDriver_get_stat(void);
  uint8_t*   STfs_get_sector(uint32_t sector_num);
  uint32_t   STfs_get_sector_size(uint32_t sector_num);
  uint32_t   STfs_get_sec_num(void);
  uint32_t   STfs_get_sector_erase_num(uint32_t sector);

  uint32_t   STfs_get_sync_obj(void);
  uint32_t   STfs_put_sync_obj(void);


  uint32_t   STfs_flash_driver_init(void);
  int32_t    FlashDriver_get_flash_mem_params(void **pdev_map, uint32_t *number_of_sectors);
  int32_t    FlashDriver_erase_sector(uint32_t sector);
  int32_t    FlashDriver_program_aligned_pages(uint32_t  addr, uint32_t bufsz, uint8_t *buf);
  int32_t    FlashDriver_program_pages(uint32_t  addr, uint32_t sz, uint8_t *buf, uint8_t *prgwrd_buf, uint8_t *prog_word_cnt);
  int32_t    FlashDriver_read_data(uint32_t addr, uint32_t sz, uint8_t *buf);

#ifdef   STFS_SIMULATOR_FLASH_DRIVER


#define EMPTY_DESCRIPTOR_COLOR     1
#define VALID_DESCRIPTOR_COLOR     2
#define INVALID_DESCRIPTOR_COLOR   3
#define EMPTY_DATA_CHUNK_COLOR     4
#define VALID_DATA_CHUNK_COLOR     5
#define INVALID_DATA_CHUNK_COLOR   6

#define    tx_thread_identify()   (0)


  typedef struct
  {
      uint8_t color;
  }
  T_sim_state;

  uint8_t* STfs_get_sector(uint32_t sector_num);
  uint32_t STfs_get_sector_size(uint32_t sector_num);

  void     Mark_Empty_Descriptor(uint32_t sector, uint32_t offset);
  void     Mark_Valid_Descriptor(uint32_t sector, uint32_t offset);
  void     Mark_Invalid_Descriptor(uint32_t sector, uint32_t offset);
  void     Mark_Invalid_Data(uint32_t sector, uint32_t start_offset, uint32_t end_offset);
  void     Mark_Valid_Data(uint32_t sector, uint32_t start_offset, uint32_t end_offset);
  void     Mark_Empty_Data(uint32_t sector, uint32_t start_offset, uint32_t end_offset);
  uint32_t Get_STfs_mark(uint32_t sector, uint32_t position);
  uint32_t STfs_get_sec_num(void);
  uint32_t STfs_get_sector_erase_num(uint32_t sector);

#else


#define  Mark_Empty_Descriptor(...)
#define  Mark_Valid_Descriptor(...)
#define  Mark_Invalid_Descriptor(...)
#define  Mark_Invalid_Data(...)
#define  Mark_Valid_Data(...)
#define  Mark_Empty_Data(...)


#endif

#ifdef __cplusplus
}
#endif

#endif



