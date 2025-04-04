/*---------------------------------------------------------------------------*/
/**
 * @file protocol.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief E-Paper Serial protocol control
 * @version 0.1
 * @date 2024-02-16
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "display.h"
#include "protocol.h"
#include "rtc.h"
#include "flash.h"

#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define PROTOCOL_SIZE       (2)
#define PROTOCOL_DATA_SIZE  (1024)
unsigned char Protocol[PROTOCOL_SIZE], ProtocolData[PROTOCOL_DATA_SIZE];
unsigned char PDataStart = 0;
unsigned int PDataCount = 0;

/*---------------------------------------------------------------------------*/
#define LINE_TEXT_SIZE  37
#define DEFAULT_FONT_W  8
#define DEFAULT_FONT_H  16

/*---------------------------------------------------------------------------*/
void parse_page_msg (fb_info_t *pfb, char *ptr)
{
    char line [LINE_TEXT_SIZE +2];
    int i;

    fb_clear  (pfb);
    for (i = 0; i < 8; i++) {
        memset (line, 0, sizeof(line));
        memcpy (&line[0], &ptr [2 + i * LINE_TEXT_SIZE], LINE_TEXT_SIZE);
        draw_text (pfb, 0, i * DEFAULT_FONT_H, 1, 0, 1, line);
    }
    USBSerial_flush ();
}

/*---------------------------------------------------------------------------*/
void parse_text_msg (fb_info_t *pfb, char *ptr)
{
    int x = 0, y = 0, f = 0, b = 0, s = 0;

    if ((ptr = strtok (NULL, ",")) != NULL) x = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) y = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) f = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) b = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) s = atoi(ptr);

    // msg
    if ((ptr = strtok (NULL, ",")) != NULL)
        draw_text (pfb, x, y, f, b, s, ptr);
}

/*---------------------------------------------------------------------------*/
void parse_time_msg (char *ptr)
{
    int y = 0, m = 0, d = 0, h = 0, b = 0, s = 0;

    if ((ptr = strtok (NULL, ",")) != NULL) y = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) m = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) d = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) h = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) b = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) {
        s = atoi(ptr);  RTC_Set(y, m, d, h, b, s);
    }
}

/*---------------------------------------------------------------------------*/
void parse_bits_msg (fb_info_t *pfb, char *ptr)
{
    int x = 0, y = 0;

    if ((ptr = strtok (NULL, ",")) != NULL) x = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) y = atoi(ptr);

    while ((ptr = strtok (NULL, ",")) != NULL) {
        if ((*ptr == 'E') && (*(ptr +1) == '#')) break;

        if (x < (pfb->w -1))    {
            put_pixel (pfb, x, y, (atoi(ptr) == 0) ? 0 : 1);   x ++;
        }
    }
}

/*---------------------------------------------------------------------------*/
void parse_bytes_msg (fb_info_t *pfb, char *ptr)
{
    int x = 0, y = 0;
    unsigned char b_data = 0;

    if ((ptr = strtok (NULL, ",")) != NULL) x = atoi(ptr);
    if ((ptr = strtok (NULL, ",")) != NULL) y = atoi(ptr);

    while ((ptr = strtok (NULL, ",")) != NULL) {
        if ((*ptr == 'E') && (*(ptr +1) == '#')) break;

        b_data = strtoul (ptr, NULL, 16);
        if (x < (pfb->w -1))    {
            if ((y +0) < pfb->h)    put_pixel (pfb, x, y +0, (b_data & 0x01) ? 1 : 0);
            if ((y +1) < pfb->h)    put_pixel (pfb, x, y +1, (b_data & 0x02) ? 1 : 0);
            if ((y +2) < pfb->h)    put_pixel (pfb, x, y +2, (b_data & 0x04) ? 1 : 0);
            if ((y +3) < pfb->h)    put_pixel (pfb, x, y +3, (b_data & 0x08) ? 1 : 0);
            if ((y +4) < pfb->h)    put_pixel (pfb, x, y +4, (b_data & 0x10) ? 1 : 0);
            if ((y +5) < pfb->h)    put_pixel (pfb, x, y +5, (b_data & 0x20) ? 1 : 0);
            if ((y +6) < pfb->h)    put_pixel (pfb, x, y +6, (b_data & 0x40) ? 1 : 0);
            if ((y +7) < pfb->h)    put_pixel (pfb, x, y +7, (b_data & 0x80) ? 1 : 0);
            x ++;
        }
    }
}

/*---------------------------------------------------------------------------*/
void parse_clear_msg (fb_info_t *pfb, char *ptr)
{
    epd_hw_reset ();

    USBSerial_print ("%s EPD init status = %d\r\n", __func__, epd_init ());

    fb_clear  (pfb);

    epd_mono_update (pfb->data, pfb->size);

    delay (1000);
    epd_deep_sleep (1);
}

/*---------------------------------------------------------------------------*/
void parse_save_msg (fb_info_t *pfb, char *ptr)
{
    if ((ptr = strtok (NULL, ",")) != NULL) {
        char screen = atoi(ptr);
        save_user_screen (pfb->data, screen);
    }
}

/*---------------------------------------------------------------------------*/
void parse_load_msg (fb_info_t *pfb, char *ptr)
{
    if ((ptr = strtok (NULL, ",")) != NULL) {
        char screen = atoi(ptr);
        load_user_screen (pfb->data, screen);
    }
}

/*---------------------------------------------------------------------------*/
char protocol_data_parse (fb_info_t *pfb, char *p)
{
    char *ptr = strtok (p, ",");

    if (ptr != NULL) {
        switch (*ptr) {
            case 'p':   parse_page_msg  (pfb, ptr); break;
            case 'm':   parse_text_msg  (pfb, ptr); break;
            case 't':   parse_time_msg  (ptr);      break;
            case 'b':   parse_bits_msg  (pfb, ptr); break;
            case 'B':   parse_bytes_msg (pfb, ptr); break;

            case 's':   parse_save_msg  (pfb, ptr); break;
            case 'l':   parse_load_msg  (pfb, ptr); break;

            case 'c':   parse_clear_msg (pfb, ptr);// break;
            default:
                return 0;
        }
        // parse_set_mode
        // parse_clear_screen
        // parse_save_screen
        // parse_text_message
        // parse_set_time
        // parse_bits_msg
        // parse_bytes_msg
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
char protocol_data_check (fb_info_t *pfb)
{
    if (USBSerial_available()) {
        Protocol[0] = Protocol [1];
        Protocol[1] = USBSerial_read();

        if (Protocol [1] == '\r')   USBSerial_print ("\r\n");
        else                        USBSerial_print ("%c", Protocol[1]);

        if        ((Protocol [0] == '@') && (Protocol [1] == 'S')) { // Protocol Start check
            PDataStart = 1; PDataCount = 0;
        } else if ((Protocol [0] == 'E') && (Protocol [1] == '#')) { // Protocol End check
            char ret = 0;
            USBSerial_print ("\r\n");
            if (PDataStart)
                ret = protocol_data_parse (pfb, ProtocolData);

            PDataCount = 0; PDataStart = 0;

            return ret;
        } else {
            if (PDataStart) {
                ProtocolData [PDataCount] = Protocol[1];
                if (PDataCount < PROTOCOL_DATA_SIZE -1) PDataCount++;
                else                                    PDataStart = 0;
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
