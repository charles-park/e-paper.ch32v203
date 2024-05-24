/*---------------------------------------------------------------------------*/
/**
 * @file display.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief E-Paper Display control
 * @version 0.1
 * @date 2024-05-23
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>

#include "lib_fb.h"
#include "lib_epd.h"
#include "rtc.h"

/*---------------------------------------------------------------------------*/
enum eDisplayMode {
    eMODE_USER_SCREEN1 = 0,
    eMODE_USER_SCREEN2,
    eMODE_USER_SCREEN3,
    eMODE_USER_SCREEN4,
    eMODE_HAN_CALENDAR,
    eMODE_ENG_CALENDAR,
    eMODE_EDIT_SCREEN,
    eMODE_END
};

/*---------------------------------------------------------------------------*/
extern int      int_to_hanstr           (char *han_str, int num);
extern void     display_han_calendar    (fb_info_t *pfb);
extern void     display_eng_calendar    (fb_info_t *pfb);
extern void     display_clear_screen    (fb_info_t *pfb);
extern void     display_user_screen     (fb_info_t *pfb, char mode);
extern void     display_update          (fb_info_t *pfb, char mode, char force);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#endif  // #define __DISPLAY_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/


