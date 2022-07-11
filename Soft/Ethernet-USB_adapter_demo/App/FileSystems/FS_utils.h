#ifndef APP_FILE_UTILS_H
  #define APP_FILE_UTILS_H

uint32_t   Open_FileX_media(void);
uint64_t   Get_media_total_sectors(void);
uint32_t   Get_media_bytes_per_sector(void);
uint64_t   Get_media_total_size(void);
uint64_t   Get_media_available_size(void);

int32_t    Read_line_from_file(FX_FILE  *fp, char *buf, uint32_t buf_len);
uint32_t   Scanf_from_file(FX_FILE  *fp, int32_t *scan_res, char *tmp_buf, uint32_t tmp_buf_sz,  const char  *fmt_ptr, ...);
uint32_t   Recreate_file_for_write(FX_FILE  *f, CHAR *filename);

uint32_t   FS_format(void);
uint32_t   Delete_all_files_in_current_dir(void);
uint32_t   Check_file_extension(char *file_name, const char * const *ext_list);
void       Get_file_extension(char *file_name, char *ext, uint32_t ext_buf_sz);
uint32_t   Get_records_files_count(const char *dir_name, const char * const *valid_extensions, uint8_t filter_only_new);
uint32_t   Create_files_list_from_dir(const char *dir_name, const char *list_file_name, const char * const *valid_extensions, uint8_t filter_only_new, uint32_t *files_cnt);
uint32_t   Get_files_info_in_directory(char *path, char *name_prefix, uint32_t *files_count, uint64_t *files_size, uint32_t *file_max_num);
uint32_t   Delete_files_with_min_number(char *path, char *name_prefix, uint32_t *file_size, uint32_t *file_num);
#endif // APP_FILE_UTILS_H



