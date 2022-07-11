#ifndef BACKPMAN3_RTC_H
  #define BACKPMAN3_RTC_H

typedef struct tm              rtc_time_t;

void   RTC_Init(void);
void   RTC_get_system_DateTime(rtc_time_t *rtc_time);


#endif // BACKPMAN3_RTC_H



