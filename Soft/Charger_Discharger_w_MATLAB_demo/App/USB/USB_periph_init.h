#ifndef __USB_PERIPH_INIT_H__
  #define __USB_PERIPH_INIT_H__

  #ifdef __cplusplus
extern "C"
{
  #endif

  extern PCD_HandleTypeDef          hpcd_USB_OTG_FS;
  extern HCD_HandleTypeDef          hhcd_USB_OTG_FS;

  void   MX_USB_OTG_FS_PCD_Init(void);
  void   MX_USB_OTG_FS_HCD_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_OTG_H__ */

