// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-03-02
// 11:01:11
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"

RTC_HandleTypeDef hrtc;
/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void RTC_Init(void)
{


  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /** Initialize RTC Only
  */
  hrtc.Instance            = RTC;
  hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv   = 100; // Делители изменены чтобы из 1 МГц  получить 1 Гц
  hrtc.Init.SynchPrediv    = 10000;
  hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutRemap    = RTC_OUTPUT_REMAP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    return;
  }

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours          = 0x0;
  sTime.Minutes        = 0x0;
  sTime.Seconds        = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc,&sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    return;
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month   = RTC_MONTH_JANUARY;
  sDate.Date    = 0x1;
  sDate.Year    = 0x0;

  if (HAL_RTC_SetDate(&hrtc,&sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    return;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param rtc_time_t
-----------------------------------------------------------------------------------------------------*/
void   RTC_get_system_DateTime(rtc_time_t *rtc_time)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  HAL_RTC_GetTime(&hrtc,&sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc,&sDate, RTC_FORMAT_BIN);

  rtc_time->tm_year  = sDate.Year;
  rtc_time->tm_mon   = sDate.Month;
  rtc_time->tm_mday  = sDate.Date;
  rtc_time->tm_wday  = sDate.WeekDay;
  rtc_time->tm_hour  = sTime.Hours;
  rtc_time->tm_min   = sTime.Minutes;
  rtc_time->tm_sec   = sTime.Seconds;
}

