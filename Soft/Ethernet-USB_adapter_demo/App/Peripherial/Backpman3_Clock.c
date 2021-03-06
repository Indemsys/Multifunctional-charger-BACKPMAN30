// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-02-09
// 11:46:22 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  // Supply configuration update enable
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  // Configure the main internal regulator output voltage
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  // Configure LSE Drive Capability
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  // Macro to configure the PLL clock source
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  // Initializes the RCC Oscillators according to the specified parameters  in the RCC_OscInitTypeDef structure.
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState      = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState      = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM      = 1;
  RCC_OscInitStruct.PLL.PLLN      = 60;
  RCC_OscInitStruct.PLL.PLLP      = 2;
  RCC_OscInitStruct.PLL.PLLQ      = 8; // 120 MHz
  RCC_OscInitStruct.PLL.PLLR      = 2;
  RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN  = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/*-----------------------------------------------------------------------------------------------------
   Peripherals Common Clock Configuration

-----------------------------------------------------------------------------------------------------*/
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  // Задаем для какой периферии будет инициализироваться тактирование
  // Для остальной периферии инициализация производиться не будет
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB | RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_SPI2 | RCC_PERIPHCLK_SPI1 | RCC_PERIPHCLK_FDCAN | RCC_PERIPHCLK_SPI45;

  // Блок PLL1 запрограммирован на 480 Мгц и здесь его настройки уже не трогаем


  // Программирование блока PLL2
  // Используем этот PLL для тактирования SPI, FDCAN, ADC
  PeriphClkInitStruct.PLL2.PLL2M           = 1;   // Делитель частоты PLL source, в данном случае частоты кварца 16 МГц
  PeriphClkInitStruct.PLL2.PLL2N           = 15;  // Умножитель после делителя (240 МГц)
  PeriphClkInitStruct.PLL2.PLL2P           = 4;   // Делитель после умножителя для формирования частоты PLL2P (60 МГц)
  PeriphClkInitStruct.PLL2.PLL2Q           = 4;   // Делитель после умножителя для формирования частоты PLL2Q (60 Мгц)
  PeriphClkInitStruct.PLL2.PLL2R           = 2;   // Делитель после умножителя для формирования частоты PLL2R (120 Мгц)
  PeriphClkInitStruct.PLL2.PLL2RGE         = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL      = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN       = 0;

  // Программирование блока PLL3
  // Используем этот PLL для тактирования USB
  PeriphClkInitStruct.PLL3.PLL3M           = 1;   // Делитель частоты PLL source, в данном случае частоты кварца 16 МГц
  PeriphClkInitStruct.PLL3.PLL3N           = 12;  // Умножитель после делителя (192 МГц)
  PeriphClkInitStruct.PLL3.PLL3P           = 2;   // Делитель после умножителя для формирования частоты PLL3P (96 МГц)
  PeriphClkInitStruct.PLL3.PLL3Q           = 4;   // Делитель после умножителя для формирования частоты PLL3Q (48 МГц)
  PeriphClkInitStruct.PLL3.PLL3R           = 2;   // Делитель после умножителя для формирования частоты PLL3R (96 МГц)
  PeriphClkInitStruct.PLL3.PLL3RGE         = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL      = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN       = 0;


  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;  //  Тактирование SPI1,2,3 от PLL2P  60 MHz (Fclk max = 200 Mhz)
  PeriphClkInitStruct.Spi45ClockSelection  = RCC_SPI45CLKSOURCE_PCLK1;  //  Тактирование SPI4,5   от PCLK  120 MHz (Fclk max = 200 Mhz)
  PeriphClkInitStruct.FdcanClockSelection  = RCC_FDCANCLKSOURCE_PLL2;   //  Тактирование FDCAN    от PLL2Q  60 MHz (Fclk max = 125 MHz)
  PeriphClkInitStruct.UsbClockSelection    = RCC_USBCLKSOURCE_PLL3;     //  Тактирование USB      от PLL3Q  48 MHz
  PeriphClkInitStruct.AdcClockSelection    = RCC_ADCCLKSOURCE_PLL2;     //  Тактирование ADC      от PLL2P  60 MHz (Fclk max = 80 MHz)
  PeriphClkInitStruct.RTCClockSelection    = RCC_RTCCLKSOURCE_HSE_DIV16;// Переводим тактирование часов на генератор HSE (16 МГц), поскольку внешний часовой кварцевый генератор перестал работать.


  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_RCC_CRC_CLK_ENABLE(); // Тактирование CRC нужно для работы библиотеки TouchGFX
}

