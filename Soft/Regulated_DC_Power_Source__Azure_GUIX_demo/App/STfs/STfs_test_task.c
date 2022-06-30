#include   "App.h"
#include   "STfs_int.h"
#include   "STfs_api.h"
#include   "Flash_media_driver.h"
#include   "STfs_tests.h"

TX_THREAD                     stfs_test_thread;

uint8_t                       *thread_stfs_task_stack;

static void                   Thread_STfs_test(ULONG initial_input);

#define                       SPEED_TEST
#define                       TEST_FILE_SIZE 10000

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_STfs_test_create(uint32_t arg)
{
  UINT res;

  // Если указатель стека не обнулен значит задача уже создана
  // Вторую копию создавать нельзя
  if (thread_stfs_task_stack != NULL) return RES_ERROR;

  thread_stfs_task_stack = App_malloc(THREAD_STFS_TEST_STACK_SIZE);
  if (thread_stfs_task_stack == NULL)
  {
    return RES_ERROR;
  }

  res = tx_thread_create(&stfs_test_thread, "STFS_test", Thread_STfs_test,
       arg,
       (void *)thread_stfs_task_stack, // stack_start
       THREAD_STFS_TEST_STACK_SIZE,    // stack_size
       THREAD_STFS_TEST_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
       THREAD_STFS_TEST_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
       TX_NO_TIME_SLICE,
       TX_AUTO_START);

  if (res != TX_SUCCESS)
  {
    App_free(thread_stfs_task_stack);
    thread_stfs_task_stack = NULL;
    return RES_ERROR;
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void  Thread_STfs_test_delete(void)
{
  if (thread_stfs_task_stack != NULL)
  {
    tx_thread_terminate(&stfs_test_thread);
    STfs_free_task_cbls((uint32_t)&stfs_test_thread);
    tx_thread_delete(&stfs_test_thread);
    App_free(thread_stfs_task_stack);
    thread_stfs_task_stack = NULL;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param sector
-----------------------------------------------------------------------------------------------------*/
void Dump_flash_sector(uint32_t sector)
{
  char str1[32];
  char str[256];

  STfs_flash_driver_init();

  APPLOG("Dump of Sector %d",sector);

  // Выводим в memo формы дамп содержимого сектора
  uint8_t  *sec_ptr  = STfs_get_sector(sector);
  uint32_t sec_size  = STfs_get_sector_size(sector);

  str[0] = 0;
  uint32_t offs = 0;
  uint8_t *ptr  = sec_ptr;
  do
  {
    sprintf(str1, "%08X %08X   ", (uint32_t)ptr, offs);
    strcat(str, str1);
    for (uint32_t i=0; i < 32; i++)
    {
      uint8_t b =*ptr;
      sprintf(str1, "%02X ",b);
      strcat(str, str1);
      ptr++;
      offs++;
      if (offs >= sec_size) break;
    }
    APPLOG(str);
    Wait_ms(10);
    str[0] = 0;

  }while (offs < sec_size);
  APPLOG("...................");
}

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_STfs_test(ULONG initial_input)
{
  T_stfs_info               stfs_info;
  T_sys_timestump           st;
  T_DataFlash_driver_stat  *fstat;
  T_stfs_test_cbl           test_cbl;
  uint8_t                   enable_fragmentation;
  float                     max_speed;
  float                     min_speed;

  APPLOG("STfs test task start.");

  if (initial_input & BIT(31))
  {
    // Выводим дамп сектора
    Dump_flash_sector(initial_input & 0xFF);
    goto exit;
  }
  else
  {
    if (STfs_init(INT_FLASH_BANK2,&stfs_info) != STFS_OK)
    {
      APPLOG("STfs init error.");
      goto exit;
    }

    memset(&test_cbl, 0, sizeof(test_cbl));

#ifdef SPEED_TEST
    test_cbl.min_file_size  = TEST_FILE_SIZE;
    test_cbl.max_file_size  = TEST_FILE_SIZE;
    test_cbl.min_file_chunk = TEST_FILE_SIZE;
    test_cbl.max_file_chunk = TEST_FILE_SIZE;
    test_cbl.max_num_chunks = 1;
#else
     test_cbl.min_file_size  = 512   ;
     test_cbl.max_file_size  = 48000 ;
     test_cbl.min_file_chunk = 8     ;
     test_cbl.max_file_chunk = 512   ;
     test_cbl.max_num_chunks = 150   ;
#endif

    STfs_init_test(&test_cbl);
    enable_fragmentation    = 0;

    srand(1234);

    for (uint32_t i=0; i < 200; i++)
    {
      uint8_t enable_deleting = 0;

      if (i % 10) enable_deleting = 1; // Каждый 10-й файл не стираем
      Get_hw_timestump(&st);           // Рандомизируем
      if (STfs_test_write_read_delete(INT_FLASH_BANK2, i, st.cycles, enable_deleting, enable_fragmentation) != STFS_OK)
      {
        APPLOG("STfs test task error.");
        goto exit;
      }
    }
  }

exit:
  fstat = FlashDriver_get_stat();
  APPLOG("FlashDriver last erasing error=%d in secotr %d, last programming error=%d at addr %08X", fstat->stfs_last_sec_erasing_err, fstat->sector,  fstat->stfs_last_flash_progr_err, fstat->addr);

  uint32_t secs_num = STfs_get_sec_num();
  for (uint32_t i=0;i<secs_num;i++)
  {
    APPLOG("Sector %d number of erasures = %d", i, STfs_get_sector_erase_num(i));
  }
  APPLOG("Minimum sector erase time = %d us",(uint32_t)fstat->min_sec_erasing_time);
  APPLOG("Maximum sector erase time = %d us",(uint32_t)fstat->max_sec_erasing_time);

  APPLOG("Open  time.  Min = %08d us, Max = %08d us",(uint32_t)test_cbl.min_open_time, (uint32_t)test_cbl.max_open_time);
  APPLOG("Write time.  Min = %08d us, Max = %08d us",(uint32_t)test_cbl.min_write_time, (uint32_t)test_cbl.max_write_time);
  APPLOG("Find  time.  Min = %08d us, Max = %08d us",(uint32_t)test_cbl.min_find_time, (uint32_t)test_cbl.max_find_time);
  APPLOG("Read  time.  Min = %08d us, Max = %08d us",(uint32_t)test_cbl.min_read_time, (uint32_t)test_cbl.max_read_time);
  APPLOG("Check time.  Min = %08d us, Max = %08d us",(uint32_t)test_cbl.min_check_time, (uint32_t)test_cbl.max_check_time);

#ifdef SPEED_TEST
  max_speed =  (float)TEST_FILE_SIZE*1000.0f/(float)test_cbl.min_write_time;
  min_speed =  (float)TEST_FILE_SIZE*1000.0f/(float)test_cbl.max_write_time;
  APPLOG("Write speed.  Min = %08.0f KB/s, Max = %08.0f KB/s",min_speed,max_speed);

  max_speed =  (float)TEST_FILE_SIZE*1000.0f/(float)test_cbl.min_read_time;
  min_speed =  (float)TEST_FILE_SIZE*1000.0f/(float)test_cbl.max_read_time;
  APPLOG("Read  speed.  Min = %08.0f KB/s, Max = %08.0f KB/s",min_speed,max_speed);
#endif

  APPLOG("STfs test task end.");
  Wait_ms(100);
  Send_cmd_delete_STfs_test();
  return;

}

