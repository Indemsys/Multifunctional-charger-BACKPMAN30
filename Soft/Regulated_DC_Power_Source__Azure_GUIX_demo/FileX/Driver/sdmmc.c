#include "App.h"

extern SD_HandleTypeDef   hsd_sdmmc[];

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t MX_SDMMC1_SD_Init(void)
{
  SD_HandleTypeDef *p_hsd1;

  p_hsd1 = &hsd_sdmmc[SD_INSTANCE];

  p_hsd1->Instance                 = SDMMC1;
  p_hsd1->Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  p_hsd1->Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  p_hsd1->Init.BusWide             = SDMMC_BUS_WIDE_4B;
  p_hsd1->Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  p_hsd1->Init.ClockDiv            = 1; // div 2 -> 60 MHz
  if (HAL_SD_Init(p_hsd1) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MX_SDMMC1_SD_Deinit(void)
{
  SD_HandleTypeDef *p_hsd1;
  p_hsd1 = &hsd_sdmmc[SD_INSTANCE];
  if (HAL_SD_DeInit(p_hsd1) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MX_SDMMC1_SD_GetStatus(void)
{
  SD_HandleTypeDef *p_hsd1;
  p_hsd1 = &hsd_sdmmc[SD_INSTANCE];
  if(HAL_SD_GetCardState(p_hsd1) != HAL_SD_CARD_TRANSFER)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param hsd
  \param p_hsd1
  \param BlockAdd
  \param NumberOfBlocks

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MX_SDMMC1_SD_ReadBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SD_HandleTypeDef *p_hsd1;
  p_hsd1 = &hsd_sdmmc[SD_INSTANCE];

  if(HAL_SD_ReadBlocks_DMA(p_hsd1, pData, BlockAdd, NumberOfBlocks) != HAL_OK)
  {
    return HAL_ERROR;
  }
  return HAL_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param pData
  \param BlockAdd
  \param NumberOfBlocks

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
HAL_StatusTypeDef MX_SDMMC1_SD_WriteBlocks_DMA(uint8_t *pData, uint32_t BlockAdd, uint32_t NumberOfBlocks)
{
  SD_HandleTypeDef *p_hsd1;
  p_hsd1 = &hsd_sdmmc[SD_INSTANCE];
  return HAL_SD_WriteBlocks_DMA(p_hsd1, pData, BlockAdd, NumberOfBlocks);
}

/*-----------------------------------------------------------------------------------------------------


  \param sdHandle
-----------------------------------------------------------------------------------------------------*/
void HAL_SD_MspInit(SD_HandleTypeDef *sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if (sdHandle->Instance == SDMMC1)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_SDMMC1_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**SDMMC1 GPIO Configuration
    PD2     ------> SDMMC1_CMD
    PC11     ------> SDMMC1_D3
    PC10     ------> SDMMC1_D2
    PC12     ------> SDMMC1_CK
    PC9     ------> SDMMC1_D1
    PC8     ------> SDMMC1_D0
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_2;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
    HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);

    GPIO_InitStruct.Pin       = GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_9 | GPIO_PIN_8;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
    HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);

    HAL_NVIC_SetPriority(SDMMC1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sdHandle
-----------------------------------------------------------------------------------------------------*/
void HAL_SD_MspDeInit(SD_HandleTypeDef *sdHandle)
{

  if (sdHandle->Instance == SDMMC1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC1_CLK_DISABLE();

    /**SDMMC1 GPIO Configuration
    PD2     ------> SDMMC1_CMD
    PC11     ------> SDMMC1_D3
    PC10     ------> SDMMC1_D2
    PC12     ------> SDMMC1_CK
    PC9     ------> SDMMC1_D1
    PC8     ------> SDMMC1_D0
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_9 | GPIO_PIN_8);

    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
  }
}

