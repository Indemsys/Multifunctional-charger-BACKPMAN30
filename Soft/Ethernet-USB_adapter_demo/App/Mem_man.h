#ifndef BACKPMAN10_APP_MEM_MAN_H
  #define BACKPMAN10_APP_MEM_MAN_H



extern TX_BYTE_POOL    g_app_pool;


uint32_t       App_create_pool_memry(VOID *first_unused_memory);
void*          App_malloc_pending(ULONG size, ULONG wait_option);
void*          App_malloc(size_t size);
void*          App_calloc(size_t num, size_t size);
void           App_free(VOID *block_ptr);
void           App_get_mem_statistic(uint32_t *size, uint32_t *avail_bytes, uint32_t *fragments);
void           Mem_man_log_statistic(void);

#endif // APP_MEM_MAN_H



