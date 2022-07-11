// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.12
// 18:13:44
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



uint8_t         *g_app_pool_memory;
uint32_t         g_app_pool_size;

TX_BYTE_POOL    g_app_pool;


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t App_create_pool_memry(VOID *first_unused_memory)
{
  g_app_pool_memory = (uint8_t *)first_unused_memory;
  g_app_pool_size   = AXI_SRAM_END - (uint32_t)g_app_pool_memory + 1;

  return tx_byte_pool_create(&g_app_pool,"app_pool_mem", g_app_pool_memory, g_app_pool_size);
}

/*-----------------------------------------------------------------------------------------------------


  \param memory_size
  \param wait_option

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* App_malloc_pending(ULONG size, ULONG wait_option)
{
  void *ptr;
  if (tx_byte_allocate(&g_app_pool, &ptr, size,wait_option) != TX_SUCCESS) return NULL;
  memset(ptr, 0, size);
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------
  Выделение блока памяти
  Блоки выделяются с выравниванием равным ALIGN_TYPE. Макрос ALIGN_TYPE определен в tx_api.h и равен ULONG

  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* App_malloc(size_t size)
{
  return App_malloc_pending(size, TX_NO_WAIT);
}


/*-----------------------------------------------------------------------------------------------------

  \param block_ptr  - Pointer to the previously allocated memory block.

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void App_free(VOID *block_ptr)
{
  tx_byte_release(block_ptr);
}

/*-----------------------------------------------------------------------------------------------------
    Функция используется в WICED

  \param num
  \param size

  \return void*
-----------------------------------------------------------------------------------------------------*/
void* App_calloc(size_t num, size_t size)
{
  void    *ptr;
  uint32_t res;
  res =  tx_byte_allocate(&g_app_pool, &ptr, num*size, TX_NO_WAIT);
  if (res != TX_SUCCESS) return NULL;
  return ptr;
}

/*-----------------------------------------------------------------------------------------------------


  \param avail_bytes
  \param fragments
-----------------------------------------------------------------------------------------------------*/
void App_get_mem_statistic(uint32_t *size, uint32_t *avail_bytes, uint32_t *fragments)
{
  CHAR           *pool_name;
  TX_THREAD      *first_suspended;
  ULONG           suspended_count;
  TX_BYTE_POOL   *next_pool;
  *size = g_app_pool_size;
  tx_byte_pool_info_get(&g_app_pool,&pool_name,(ULONG *)avail_bytes,(ULONG *)fragments,&first_suspended,&suspended_count,&next_pool);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Mem_man_log_statistic(void)
{
  uint32_t size;
  uint32_t avail_bytes;
  uint32_t fragments;
  App_get_mem_statistic(&size, &avail_bytes, &fragments);
  LOGs(__FUNCTION__, __LINE__, SEVERITY_DEFAULT,"Memory pool: size=%d, available bytes=%d, fragments=%d", size, avail_bytes, fragments);
}

