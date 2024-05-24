//-----------------------------------------------------------------------------
/**
 * @file lib_fb.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief Framebuffer control library
 * @version 0.1
 * @date 2022-05-10 (2024-05-14 mono e-paper fb)
 *
 * @copyright Copyright (c) 2022
 *
 */
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_fb.h"
//-----------------------------------------------------------------------------
// Fonts (Font Selected from Makefile)
//-----------------------------------------------------------------------------
// Hangul font size = 12kbytes
#if defined(_FONT_HANGUL_)
    #include "fonts/FontHangul.h"
#endif
#if defined(_FONT_HANSOFT_)
    #include "fonts/FontHansoft.h"
#endif
#if defined(_FONT_HANBOOT_)
    #include "fonts/FontHanboot.h"
#endif
#if defined(_FONT_HANGODIC_)
    #include "fonts/FontHangodic.h"
#endif
#if defined(_FONT_HANPIL_)
    #include "fonts/FontHanpil.h"
#endif

#include "fonts/FontAscii_8x16.h"   // 4Kbytes

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// hangul image base 16x16
//-----------------------------------------------------------------------------
static unsigned char HANFontImage[32] = {0,};

const char D_ML[22] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 																	};
const char D_FM[40] = { 1, 3, 0, 2, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 0, 2, 1, 3, 1, 3, 1, 3 			};
const char D_MF[44] = { 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 1, 6, 3, 7, 3, 7, 3, 7, 1, 6, 2, 6, 4, 7, 4, 7, 4, 7, 2, 6, 1, 6, 3, 7, 0, 5 };

static unsigned char *HANFONT1 = NULL;
static unsigned char *HANFONT2 = NULL;
static unsigned char *HANFONT3 = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void make_image  (unsigned char is_first,
                        unsigned char *dest,
                        unsigned char *src)
{
    int i;
    if (is_first)   for (i = 0; i < 32; i++)    dest[i]  = src[i];
    else            for (i = 0; i < 32; i++)    dest[i] |= src[i];
}

//-----------------------------------------------------------------------------
static unsigned char *get_hangul_image( unsigned char HAN1,
                                        unsigned char HAN2,
                                        unsigned char HAN3)
{
    unsigned char f, m, l;
    unsigned char f1, f2, f3;
    unsigned char first_flag = 1;
    unsigned short utf16 = 0;

    /*------------------------------
    UTF-8 을 UTF-16으로 변환한다.

    UTF-8 1110xxxx 10xxxxxx 10xxxxxx
    ------------------------------*/
    utf16 = ((unsigned short)HAN1 & 0x000f) << 12 |
            ((unsigned short)HAN2 & 0x003f) << 6  |
            ((unsigned short)HAN3 & 0x003f);
    utf16 -= 0xAC00;

    /* 초성 / 중성 / 종성 분리 */
    l = (utf16 % 28);
    utf16 /= 28;
    m = (utf16 % 21) +1;
    f = (utf16 / 21) +1;

    /* 초성 / 중성 / 종성 형태에 따른 이미지 선택 */
    f3 = D_ML[m];
    f2 = D_FM[(f * 2) + (l != 0)];
    f1 = D_MF[(m * 2) + (l != 0)];

    memset(HANFontImage, 0, sizeof(HANFontImage));
    if (f)  {   make_image(         1, HANFontImage, HANFONT1 + (f1*16 + f1 *4 + f) * 32);    first_flag = 0; }
    if (m)  {   make_image(first_flag, HANFontImage, HANFONT2 + (        f2*22 + m) * 32);    first_flag = 0; }
    if (l)  {   make_image(first_flag, HANFontImage, HANFONT3 + (f3*32 - f3 *4 + l) * 32);    first_flag = 0; }

    return HANFontImage;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void put_pixel_mono (fb_info_t *fb, int x, int y, int color)
{
    unsigned int data_offset;
    unsigned char bit_mask;

    bit_mask = 0x80 >> (y % 8);

    data_offset = (y / 8) + ((fb->w - (x)) % fb->w) * (fb->h / 8);

    if (data_offset < fb->size) {
        if (fb->reversed) {
            if (color)  fb->data[data_offset]  &= ~bit_mask;
            else        fb->data[data_offset]  |=  bit_mask;
        } else {
            if (color)  fb->data[data_offset]  |=  bit_mask;
            else        fb->data[data_offset]  &= ~bit_mask;
        }
    }
//    else {
//        fprintf(stdout, "Out of range.(width = %d, x = %d, height = %d, y = %d)\n",
//            fb->w, x, fb->h, y);
//    }
}

//-----------------------------------------------------------------------------
void put_pixel (fb_info_t *fb, int x, int y, int color)
{
    unsigned int cal_x, cal_y;

    switch (fb->rotate) {
        case eROTATE_0: default :
            cal_x = x;
            cal_y = y;
            break;
        case 90:
        case eROTATE_90:    // 90 degree
            cal_x = fb->w -y -1;
            cal_y = x;
            break;
        case 180:
        case eROTATE_180:   // 180 degree
            cal_x = fb->w -x -1;
            cal_y = fb->h -y -1;
            break;
        case 270:
        case eROTATE_270:   // 270 degree
            cal_x = y;
            cal_y = fb->h -x -1;
            break;
    }
    put_pixel_mono  (fb, cal_x, cal_y, color);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void draw_hangul_bitmap (fb_info_t *fb,
                    int x, int y, unsigned char *p_img,
                    int f_color, int b_color, int scale)
{
    int pos, i, j, mask, x_off, y_off, scale_y, scale_x;

    for (i = 0, y_off = 0, pos = 0; i < 16; i++) {
        for (scale_y = 0; scale_y < scale; scale_y++) {
            if (scale_y)
                pos -= 2;
            for (x_off = 0, j = 0; j < 2; j++) {
                for (mask = 0x80; mask > 0; mask >>= 1) {
                    for (scale_x = 0; scale_x < scale; scale_x++) {
                        int c;
                        c = (p_img[pos] & mask) ? f_color : b_color;

                        put_pixel(fb, x + x_off, y + y_off, c);
                        x_off++;
                    }
                }
                pos++;
            }
            y_off++;
        }
    }
}

//-----------------------------------------------------------------------------
static void draw_ascii_bitmap (fb_info_t *fb,
                    int x, int y, unsigned char *p_img,
                    int f_color, int b_color, int scale)
{
    int pos, mask, x_off, y_off, scale_y, scale_x;

    for (pos = 0, y_off = 0; pos < 16; pos++) {
        for (scale_y = 0; scale_y < scale; scale_y++) {
            for (x_off = 0, mask = 0x80; mask > 0; mask >>= 1) {
                for (scale_x = 0; scale_x < scale; scale_x++) {
                    int c;
                    c = (p_img[pos] & mask) ? f_color : b_color;

                    put_pixel(fb, x + x_off, y + y_off, c);
                    x_off++;
                }
            }
            y_off++;
        }
    }
}

//-----------------------------------------------------------------------------
static void _draw_text (fb_info_t *fb, int x, int y, char *p_str,
                        int f_color, int b_color, int scale)
{
    unsigned char *p_img;
    unsigned char c1, c2, c3;

    while(*p_str) {
        c1 = *(unsigned char *)p_str++;

        //---------- 한글 ---------
        /* 모든 문자는 기본적으로 UTF-8형태로 저장되며 한글은 3바이트를 가진다. */
        /* 한글은 3바이트를 일어 UTF8 to UTF16으로 변환후 초/중/종성을 분리하여 조합형으로 표시한다. */
        if (c1 >= 0x80){
            c2 = *(unsigned char *)p_str++;
            c3 = *(unsigned char *)p_str++;

            p_img = get_hangul_image(c1, c2, c3);
            draw_hangul_bitmap(fb, x, y, p_img, f_color, b_color, scale);
            x = x + FONT_HANGUL_WIDTH * scale;
        }
        //---------- ASCII ---------
        else {
            p_img = (unsigned char *)FONT_ASCII[c1];
            draw_ascii_bitmap(fb, x, y, p_img, f_color, b_color, scale);
            x = x + FONT_ASCII_WIDTH * scale;
        }
    }
}

//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int my_strlen(char *str)
{
   int cnt = 0, err = 512;

   /* utf-8 에서 한글표현은 3바이트 */
   while ((*str != 0x00) && err--) {
      if (*str & 0x80) {
         str += 3;   cnt += 2;
      } else {
         str += 1;   cnt++;
      }
   }
   return err ? cnt : 0;
}

//------------------------------------------------------------------------------
void draw_text (fb_info_t *fb, int x, int y,
                int f_color, int b_color, int scale, char *fmt, ...)
{
    char buf[256];
    va_list va;
    int cal_x, cal_y, cal_strlen;

    memset(buf, 0x00, sizeof(buf));

    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);

    switch (x) {
        case ALIGN_X_LEFT:
            cal_x = 0;
            break;
        case ALIGN_X_RIGHT:     case ALIGN_X_CENTER:
            cal_strlen = my_strlen (buf);
            cal_x = (fb->w - cal_strlen * FONT_ASCII_WIDTH * scale);
            if (x == ALIGN_X_CENTER)    cal_x /= 2;
            if (cal_x < 0)              cal_x  = 0;
            break;
        default :
            cal_x = x;
            break;
    }
    switch (y) {
        case ALIGN_Y_TOP:
            cal_y = 0;
            break;
        case ALIGN_Y_BOTTOM:    case ALIGN_Y_CENTER:
            // font height = 16 pixel
            cal_y = (fb->h - 16 * scale);
            if (y == ALIGN_Y_CENTER)    cal_y = (fb->h / 2) - 8;
            if (cal_y < 0)              cal_y = 0;
            break;
        default :
            cal_y = y;
            break;
    }

    _draw_text(fb, cal_x, cal_y, buf, f_color, b_color, scale);
}

//-----------------------------------------------------------------------------
void draw_line (fb_info_t *fb, int x, int y, int w, int color)
{
    int dx;

    for (dx = 0; dx < w; dx++)
        put_pixel(fb, x + dx, y, color);
}

//-----------------------------------------------------------------------------
void draw_rect (fb_info_t *fb, int x, int y, int w, int h, int lw, int color)
{
	int dy, i;

	for (dy = 0; dy < h; dy++) {
        if (dy < lw || (dy > (h - lw -1)))
            draw_line (fb, x, y + dy, w, color);
        else {
            for (i = 0; i < lw; i++) {
                put_pixel (fb, x + 0    +i, y + dy, color);
                put_pixel (fb, x + w -1 -i, y + dy, color);
            }
        }
	}
}

//-----------------------------------------------------------------------------
void draw_fill_rect (fb_info_t *fb, int x, int y, int w, int h, int color)
{
	int dy;

	for (dy = 0; dy < h; dy++)
        draw_line(fb, x, y + dy, w, color);
}

//-----------------------------------------------------------------------------
void set_font(enum eFONTS_HANGUL s_font)
{
    switch(s_font)
    {
        default :
#if defined(__FONT_HANBOOT_H__)
        case    eFONT_HANBOOT:
            HANFONT1 = (unsigned char *)FONT_HANBOOT1;
            HANFONT2 = (unsigned char *)FONT_HANBOOT2;
            HANFONT3 = (unsigned char *)FONT_HANBOOT3;
        break;
#endif
#if defined(__FONT_HANGODIC_H__)
        case    eFONT_HANGODIC:
            HANFONT1 = (unsigned char *)FONT_HANGODIC1;
            HANFONT2 = (unsigned char *)FONT_HANGODIC2;
            HANFONT3 = (unsigned char *)FONT_HANGODIC3;
        break;
#endif
#if defined(__FONT_HANPIL_H__)
        case    eFONT_HANPIL:
            HANFONT1 = (unsigned char *)FONT_HANPIL1;
            HANFONT2 = (unsigned char *)FONT_HANPIL2;
            HANFONT3 = (unsigned char *)FONT_HANPIL3;
        break;
#endif
#if defined(__FONT_HANSOFT_H__)
        case    eFONT_HANSOFT:
            HANFONT1 = (unsigned char *)FONT_HANSOFT1;
            HANFONT2 = (unsigned char *)FONT_HANSOFT2;
            HANFONT3 = (unsigned char *)FONT_HANSOFT3;
        break;
#endif
#if defined(__FONT_HANGUL_H__)
        case    eFONT_HANGUL:
            HANFONT1 = (unsigned char *)FONT_HANGUL1;
            HANFONT2 = (unsigned char *)FONT_HANGUL2;
            HANFONT3 = (unsigned char *)FONT_HANGUL3;
        break;
#endif
    }
}

//-----------------------------------------------------------------------------
void fb_set_rotate (fb_info_t *fb, int rotate)
{
    fb->rotate = rotate;
}

//-----------------------------------------------------------------------------
void fb_set_reversed (fb_info_t *fb, unsigned char status)
{
    fb->reversed = status;
}

//-----------------------------------------------------------------------------
void fb_set_fill (fb_info_t *fb, unsigned char fill)
{
    memset(fb->data, fill, fb->size);
}

//-----------------------------------------------------------------------------
void fb_clear (fb_info_t *fb)
{
    fb_set_fill (fb, fb->reversed ? 0xff : 0x00);
}

//-----------------------------------------------------------------------------
fb_info_t *fb_init (int fb_w, int fb_h, int fb_bpp)
{
    fb_info_t *fb = (fb_info_t *)malloc(sizeof(fb_info_t));

    if (fb == NULL) {
        fprintf(stdout, "ERROR: framebuffer malloc error!\n");
        return NULL;
    }
    memset(fb, 0, sizeof(fb_info_t));

    fb->w       = (fb_w % 8) ? (fb_w + 1) : fb_w;
    fb->h       = (fb_h % 8) ? (fb_h + 1) : fb_h;
    fb->bpp     = fb_bpp;
    fb->size    = (fb_w * fb_h * fb_bpp) / 8;

    fb->stride  = (fb_w * fb_bpp) / 8;
    if ((fb->data = (unsigned char *)malloc(fb->size)) == NULL)
        goto out;

    set_font (eFONT_HANGODIC);
    return  fb;
out:
    return  NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
