#include "App.h"


PCD_HandleTypeDef hpcd_USB_OTG_FS;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void MX_USB_OTG_FS_PCD_Init(void)
{
  hpcd_USB_OTG_FS.Instance                     = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints           = 9;
  hpcd_USB_OTG_FS.Init.speed                   = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable              = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface              = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable              = DISABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable        = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable              = DISABLE;
  hpcd_USB_OTG_FS.Init.battery_charging_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable     = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1       = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pcdHandle
-----------------------------------------------------------------------------------------------------*/
void HAL_PCD_MspInit(PCD_HandleTypeDef *pcdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (pcdHandle->Instance == USB_OTG_FS)
  {
    /** Enable USB Voltage detector
    */
    HAL_PWREx_EnableUSBVoltageDetector();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USB_OTG_FS GPIO Configuration
    PA12     ------> USB_OTG_FS_DP
    PA9     ------> USB_OTG_FS_VBUS
    PA11     ------> USB_OTG_FS_DM
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_12 | GPIO_PIN_11;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    GPIO_InitStruct.Pin       = GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

    HAL_NVIC_SetPriority(OTG_FS_EP1_OUT_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_EP1_OUT_IRQn);
    HAL_NVIC_SetPriority(OTG_FS_EP1_IN_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_EP1_IN_IRQn);
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pcdHandle
-----------------------------------------------------------------------------------------------------*/
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *pcdHandle)
{

  if (pcdHandle->Instance == USB_OTG_FS)
  {
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

    /**USB_OTG_FS GPIO Configuration
    PA12     ------> USB_OTG_FS_DP
    PA9     ------> USB_OTG_FS_VBUS
    PA11     ------> USB_OTG_FS_DM
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12 | GPIO_PIN_9 | GPIO_PIN_11);


    HAL_NVIC_DisableIRQ(OTG_FS_EP1_OUT_IRQn);
    HAL_NVIC_DisableIRQ(OTG_FS_EP1_IN_IRQn);
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

  }
}


