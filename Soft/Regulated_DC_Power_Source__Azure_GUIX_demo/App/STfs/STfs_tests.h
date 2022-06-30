#ifndef SFFS_TESTS_H
  #define SFFS_TESTS_H


  #ifdef __cplusplus
extern "C"
{
  #endif

  typedef struct
  {
      uint32_t min_file_size;
      uint32_t max_file_size;
      uint32_t min_file_chunk;
      uint32_t max_file_chunk;
      uint32_t max_num_chunks;

      uint64_t min_open_time;
      uint64_t max_open_time;

      uint64_t min_write_time;
      uint64_t max_write_time;

      uint64_t min_find_time;
      uint64_t max_find_time;

      uint64_t min_read_time;
      uint64_t max_read_time;

      uint64_t min_check_time;
      uint64_t max_check_time;
  } T_stfs_test_cbl;


  void    STfs_init_test(T_stfs_test_cbl *test_cbl);
  int32_t STfs_test_write_read_delete(uint8_t drive_id, uint32_t iter_number, uint32_t number, uint8_t en_deleting, uint8_t fragm_flag);
  void    STfs_test_check(uint8_t disk_id);

#ifdef __cplusplus
}
#endif

#endif // SFFS_TESTS_H



