/*---------------------------------------------------------------------------*/
/**
 * @file display.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief E-Paper Display control
 * @version 0.1
 * @date 2024-05-23
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"

#include "display.h"
#include "flash.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
const char *Week_EngStr[] = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT",
};

/*---------------------------------------------------------------------------*/
const char *Week_HanStr[] = {
    "일요일", "월요일", "화요일", "수요일", "목요일", "금요일", "토요일",
};

/*---------------------------------------------------------------------------*/
const char *Month_HanStr   = "월";
const char *Num10_HanStr   = "십";
const char *Num100_HanStr  = "백";
const char *Num1000_HanStr = "천";

/*---------------------------------------------------------------------------*/
const char *Num_HanStr[] = {
    "영", "일", "이", "삼", "사", "오", "육", "칠", "팔", "구",
};

/*---------------------------------------------------------------------------*/
const char *AmPm_HanStr[] = {
    "오전", "오후"
};

const char *NumHour_HanStr[] = {
    "영시", "한시", "두시", "세시", "네시", "다섯시", "여섯시", "일곱시", "여덟시", "아홉시", "열시", "열한시", "열두시",
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int int_to_hanstr (char *han_str, int num)
{
    int num_10, num_100, num_1000, pos = 0;

    num_1000 = num ? (num / 1000) : 0;    num = num ? (num % 1000) : 0;
    num_100  = num ? (num / 100)  : 0;    num = num ? (num % 100)  : 0;
    num_10   = num ? (num / 10)   : 0;    num = num ? (num % 10)   : 0;

    if (num_1000) {
        if (num_1000 > 1)
            pos += sprintf (&han_str [pos], "%s%s", Num_HanStr[num_1000], Num1000_HanStr);
        else
            pos += sprintf (&han_str [pos], "%s", Num1000_HanStr);
    }

    if (num_100) {
        if (num_100 > 1)
            pos += sprintf (&han_str [pos], "%s%s", Num_HanStr[num_100], Num100_HanStr);
        else
            pos += sprintf (&han_str [pos], "%s", Num100_HanStr);
    }

    if (num_10) {
        if (num_10 > 1)
            pos += sprintf (&han_str [pos], "%s%s", Num_HanStr[num_10], Num10_HanStr);
        else
            pos += sprintf (&han_str [pos], "%s", Num10_HanStr);
    }

    if (num)
        pos += sprintf (&han_str [pos], "%s", Num_HanStr[num]);

    return pos;
}

/*---------------------------------------------------------------------------*/
void display_han_calendar (fb_info_t *pfb)
{
    char f[20], m[20], l[20];

    USBSerial_print ("%s EPD partial init status = %d\r\n", __func__, epd_init_partial ());

    draw_text (pfb, ALIGN_X_CENTER,  ALIGN_Y_CENTER, 0, 1, 1, " * MODE : Hangul Calendar * ");

    epd_update_partial(pfb->data, pfb->size);
    fb_clear  (pfb);

    draw_text (pfb, ALIGN_X_CENTER,  0, 1, 0, 3, "달력 표시중");

    memset (f, 0, sizeof(f));   int_to_hanstr (f, calendar.w_year);
    memset (m, 0, sizeof(m));   int_to_hanstr (m, calendar.w_month);
    memset (l, 0, sizeof(l));   int_to_hanstr (l, calendar.w_date);
    draw_text (pfb, ALIGN_X_CENTER,  ALIGN_Y_CENTER, 1, 0, 1, "%s년 %s월 %s일", f, m, l);

    memset (m, 0, sizeof(m));   int_to_hanstr (m, calendar.min);
    if (calendar.min != 0)
        draw_text (pfb, ALIGN_X_CENTER,  96, 1, 0, 1, "%s %s %s분 [%s]",
            AmPm_HanStr[calendar.hour / 12],
            (calendar.hour == 12) ? NumHour_HanStr[12] : NumHour_HanStr[calendar.hour % 12],
            m, Week_HanStr[calendar.week]);
    else
        draw_text (pfb, ALIGN_X_CENTER,  96, 1, 0, 1, "%s %s %s",
            AmPm_HanStr[calendar.hour / 12],
            (calendar.hour == 12) ? NumHour_HanStr[12] : NumHour_HanStr[calendar.hour % 12],
            m, Week_HanStr[calendar.week]);

    epd_update_partial(pfb->data, pfb->size);
    epd_deep_sleep (1);
}

/*---------------------------------------------------------------------------*/
void display_eng_calendar (fb_info_t *pfb)
{
    USBSerial_print ("%s EPD partial init status = %d\r\n", __func__, epd_init_partial ());

    draw_text (pfb, ALIGN_X_CENTER,  ALIGN_Y_CENTER, 0, 1, 1, " * MODE : English Calendar * ");

    epd_update_partial(pfb->data, pfb->size);
    fb_clear  (pfb);

    draw_text (pfb, ALIGN_X_CENTER,  0, 1, 0, 3, "Calendar");

    draw_text (pfb, ALIGN_X_CENTER,  ALIGN_Y_CENTER, 1, 0, 1, "%d Year / %d Month / %d Day",
        calendar.w_year, calendar.w_month, calendar.w_date);

    draw_text (pfb, ALIGN_X_CENTER,  96, 1, 0, 1, "%d hour %d minute [%s]",
        calendar.hour, calendar.min, Week_EngStr[calendar.week]);

    epd_update_partial(pfb->data, pfb->size);
    epd_deep_sleep (1);
}

/*---------------------------------------------------------------------------*/
#if defined (EPD_COLOR_MODEL)
void display_clear_screen (fb_info_t *pfb, fb_info_t *pfb_red)
{
    epd_hw_reset ();

    USBSerial_print ("%s EPD init status = %d\r\n", __func__, epd_init ());

    fb_clear  (pfb);    fb_clear  (pfb_red);

    epd_color_update(   pfb->data, pfb->size,
                        pfb_red->data, pfb_red->size);
    delay (1000);
    epd_deep_sleep (1);
}

void display_user_screen (fb_info_t *pfb, fb_info_t *pfb_red, char mode)
{
    USBSerial_print ("%s EPD init status = %d\r\n", __func__, epd_init ());

    // red fb
    draw_text (pfb_red, ALIGN_X_CENTER,   8, 1, 0, 3, "잠시 주차중");

    // black fb
    draw_text (pfb, ALIGN_X_CENTER,  64, 1, 0, 2, "010 1234 5678");
    draw_text (pfb, ALIGN_X_CENTER,  96, 1, 0, 1, "공일공  일이삼사  오육칠팔");

    epd_color_update(epd_fb->data, epd_fb->size,
                     epd_fb_red->data, epd_fb_red->size);
    epd_deep_sleep (1);
}
#else
void display_clear_screen (fb_info_t *pfb)
{
    epd_hw_reset ();

    USBSerial_print ("%s EPD init status = %d\r\n", __func__, epd_init ());

    fb_clear  (pfb);

    epd_mono_update (pfb->data, pfb->size);

    delay (1000);
    epd_deep_sleep (1);
}

/*---------------------------------------------------------------------------*/
void display_user_screen (fb_info_t *pfb, char mode)
{
    USBSerial_print ("%s EPD partial init status = %d\r\n", __func__, epd_init_partial ());

    draw_text (pfb, ALIGN_X_CENTER,  ALIGN_Y_CENTER, 0, 1, 1, " * MODE : User Screen %d * ", mode);

    epd_update_partial(pfb->data, pfb->size);
    // load user screen data from flash
    load_user_screen (pfb->data, mode);
#if 0
    draw_text (pfb, ALIGN_X_CENTER,   8, 1, 0, 3, "잠시 주차중");
    draw_text (pfb, ALIGN_X_CENTER,  64, 1, 0, 2, "010 1234 5678");
    draw_text (pfb, ALIGN_X_CENTER,  96, 1, 0, 1, "공일공  일이삼사  오육칠팔");
#endif
    epd_update_partial(pfb->data, pfb->size);
    epd_deep_sleep (1);
}
#endif

/*---------------------------------------------------------------------------*/
void display_edit_screen (fb_info_t *pfb)
{
    epd_update_partial(pfb->data, pfb->size);
}

/*---------------------------------------------------------------------------*/
void display_update (fb_info_t *pfb, char mode, char force)
{
    static int Min = 0;

    if ((Min != calendar.min) || force) {

        Min = calendar.min;
        // Debug message
        USBSerial_print("Display Mode = %d, %d-%d-%d  %d  %d:%d:%d\r\n",
                mode,
                calendar.w_year, calendar.w_month, calendar.w_date,
                calendar.week, calendar.hour, calendar.min, calendar.sec);

        switch (mode) {
            case eMODE_USER_SCREEN1:    case eMODE_USER_SCREEN2:
            case eMODE_USER_SCREEN3:    case eMODE_USER_SCREEN4:
                display_user_screen  (pfb, mode);
                break;
            case eMODE_EDIT_SCREEN:
                display_edit_screen  (pfb);
                break;
            default :
            case eMODE_HAN_CALENDAR:    display_han_calendar (pfb);  break;
            case eMODE_ENG_CALENDAR:    display_eng_calendar (pfb);  break;
        }
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
