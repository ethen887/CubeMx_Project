#ifndef __RTC_Tesk_
#define __RTC_Tesk_
#define RTC_INIT_FLAG 0x2333

#include "main.h"
#include "rtc.h"
#include "stm32f1xx.h"
#include "time.h"
HAL_StatusTypeDef KK_RTC_SetTime(struct tm *time);
struct  tm* KK_RTC_GetTime();
void KK_RTC_Init();
#endif // !__RTC_Tesk_
