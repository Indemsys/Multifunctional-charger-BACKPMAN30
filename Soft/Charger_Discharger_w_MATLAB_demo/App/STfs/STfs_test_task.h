#ifndef SFFS_TEST_TASK_H
  #define SFFS_TEST_TASK_H


uint32_t Thread_STfs_test_create(uint32_t arg);
void     Thread_STfs_test_delete(void);
void     Dump_flash_sector(uint32_t sector);

#endif // SFFS_TEST_TASK_H



