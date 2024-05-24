/*---------------------------------------------------------------------------*/
/**
 * @file e-paper.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief CH32V203 RISC-V rtc control
 * @version 0.1
 * @date 2024-05-20
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#include "ch32v20x.h"

typedef struct
{
    __IO uint8_t hour;
    __IO uint8_t min;
    __IO uint8_t sec;

    __IO uint16_t w_year;
    __IO uint8_t  w_month;
    __IO uint8_t  w_date;
    __IO uint8_t  week;
} _calendar_obj;

extern _calendar_obj calendar;

uint8_t RTC_Init(void);
uint8_t Is_Leap_Year(uint16_t year);
uint8_t RTC_Alarm_Set(uint16_t syear, uint8_t smon, uint8_t sday, uint8_t hour, uint8_t min, uint8_t sec);
uint8_t RTC_Get(void);
uint8_t RTC_Get_Week(uint16_t year, uint8_t month, uint8_t day);
uint8_t RTC_Set(uint16_t syear, uint8_t smon, uint8_t sday, uint8_t hour, uint8_t min, uint8_t sec);

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/

#endif
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
