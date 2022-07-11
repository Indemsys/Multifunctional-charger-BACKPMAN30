/**
  ******************************************************************************
  * @file    sdmmc.h
  * @brief   This file contains all the function prototypes for
  *          the sdmmc.c file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __SDMMC_H__
#define __SDMMC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "App.h"



uint32_t          MX_SDMMC1_SD_Init(void);
uint32_t          MX_SDMMC1_SD_Deinit(void);
uint32_t          MX_SDMMC1_SD_GetStatus(void);
uint32_t          MX_SDMMC1_SD_ReadBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);
HAL_StatusTypeDef MX_SDMMC1_SD_WriteBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks);

#ifdef __cplusplus
}
#endif

#endif /* __SDMMC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
