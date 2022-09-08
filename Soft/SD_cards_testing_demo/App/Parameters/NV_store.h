#ifndef BACKPMAN10_NV_STORE_H
  #define BACKPMAN10_NV_STORE_H




extern uint8_t    nv_store_ok;



void           Return_def_params(void);
int32_t        Restore_NV_parameters(void);
int32_t        Restore_from_STfs(void);
int32_t        Save_Params_to_STfs(void);

#endif // NV_STORE_H



