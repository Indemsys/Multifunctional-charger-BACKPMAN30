#ifndef APP_JSON_SERIALIZER_H
  #define APP_JSON_SERIALIZER_H


  #define MAIN_PARAMETERS_KEY                  "Parameters"           //
  #define DATETIME_SETTINGS_KEY                "DateTime"             //
  #define DEVICE_HEADER_KEY                    "Device"               //
  #define PARAMETERS_TREE_KEY                  "Parameters_tree"      //


uint32_t    Serialze_settings_schema_to_JSON_file(char *filename, size_t flags);
uint32_t    Serialze_settings_to_JSON_file(char *filename, size_t flags);
uint32_t    Serialze_settings_to_mem(char **mem, uint32_t *str_size, uint32_t  flags);
uint32_t    Serialze_device_state_to_mem(char **mem, uint32_t *str_size);
uint32_t    Serialze_Ack_message(char **mem, uint32_t *str_size, uint32_t ack_code, const char *ack_str);

uint32_t    Write_json_str_to_file(FX_FILE *p_file, size_t flags, char *str);
uint32_t    Serialize_device_description_to_file(FX_FILE *p_file, size_t flags, char *obj_name);

#endif // APP_PARAMS_SERIALIZER_H



