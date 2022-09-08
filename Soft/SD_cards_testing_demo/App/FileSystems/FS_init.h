#ifndef FS_INIT_H
  #define FS_INIT_H


#define FS_MEMORY_SZ               (512*43) // Размер в байтах буфера exFAT. В сумме с другими структурами для exFAT не должен превышать размер выделенный для AXI_SRAM_NC


typedef struct
{
    uint32_t fx_media_open_result;
    uint32_t fx_corr_buff_alloc_res;
    uint32_t fs_detected_errors;
    uint32_t fs_check_error_code;
    uint32_t creation_misc_dir_res;
    uint32_t creation_records_dir_res;
    uint32_t creation_log_dir_res;
    uint32_t deleting_win_dir_res;
    uint32_t deleting_errors_cnt;
    uint32_t fs_formated;
    uint32_t fs_format_result;

} T_file_system_init_results;


extern FX_MEDIA          fat_fs_media;

uint32_t   Init_exFAT(void);
uint32_t   Get_fs_memory_size(void);
uint32_t   Create_subdirectory(const char *dir_name);

T_file_system_init_results* Get_FS_init_res(void);

#endif // FS_INIT_H



