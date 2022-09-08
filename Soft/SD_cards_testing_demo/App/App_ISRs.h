#ifndef __STM32H7xx_IT_H
#define __STM32H7xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif



void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void TIM3_IRQHandler(void);
void SDMMC1_IRQHandler(void);
void OTG_FS_EP1_OUT_IRQHandler(void);
void OTG_FS_EP1_IN_IRQHandler(void);
void OTG_FS_IRQHandler(void);

void Error_Handler(void);


#ifdef __cplusplus
}
#endif

#endif

