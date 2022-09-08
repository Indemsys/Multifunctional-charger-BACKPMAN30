#ifndef BACKPMAN10_PRAMS_DYN_TABLES_H
  #define BACKPMAN10_PRAMS_DYN_TABLES_H

#define  TYPE_MISC_TABLE           0

#define  MISC_LIST_KEY             "MISC_list"       //

#define  MISC_TABLE_REC_NUM        5


#define FTP_PATH_LEN               128
#define FTP_LOGIN_LEN              16
#define FTP_PASS_LEN               16

#define SHCHED_MODE_EXACT_DATE     0
#define SHCHED_MODE_BY_WEEKDAY     1

typedef struct
{
    enum enm_parmnlev  level;    // Уровень в меню параметров
    uint8_t            table_id; // Идентификатор таблицы
    const char        *name;
    uint32_t           records_num;
} T_params_table_info;

typedef struct
{
  const char          *field_name;
  enum vartypes       fld_type;
  uint32_t            fld_sz;
  size_t              fld_offset;

} T_table_fields_props;


// Демонстрационная структуры полей таблицы
typedef struct
{
    uint8_t       field1;
    char          text1[32];
} T_misc_table;




extern  T_misc_table   tbl_client_ap_list[];


uint32_t                    Get_params_tables_count(void);
T_params_table_info*        Get_params_table_record_ptr(uint32_t indx);

uint32_t                    Get_table_records_count(uint32_t table_id);
char const*                 Get_table_JSON_key(uint32_t table_id);
T_table_fields_props const* Get_table_fields_props(uint32_t table_id, uint32_t *filds_cnt);
void*                       Get_table_records_ptr(uint32_t table_id, uint32_t indx);

void                        Restore_default_dyn_tables(void);


#endif // S7V10_PRAMS_DYN_TABLES_H



