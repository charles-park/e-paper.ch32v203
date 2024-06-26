//-----------------------------------------------------------------------------
/**
 * @file lib_fb.h
 * @author charles-park (charles-park@hardkernel.com)
 * @brief framebuffer library header file.
 * @version 0.1
 * @date 2022-05-10
 *
 * @copyright Copyright (c) 2022
 *
 */
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef __LIB_FB_H__
#define __LIB_FB_H__

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------------
// Color table & convert macro
//-----------------------------------------------------------------------------
#include "color_table.h"

//-----------------------------------------------------------------------------
// Frame buffer struct
//-----------------------------------------------------------------------------
typedef union fb_color__u {
    struct {
        unsigned int b:8;    // lsb
        unsigned int g:8;
        unsigned int r:8;
        unsigned int a:8;
    } bits;
    unsigned int uint;
}   fb_color_u;

//-----------------------------------------------------------------------------
// Frame buffer struct
//-----------------------------------------------------------------------------
typedef struct fb_info__t {
    int             w, h, size, bpp, stride, rotate;
    unsigned char   *data, reversed;
}   fb_info_t;

//-----------------------------------------------------------------------------
#define FONT_HANGUL_WIDTH   16
#define FONT_ASCII_WIDTH    8
#define FONT_HEIGHT         16

enum eFONTS_HANGUL {
    eFONT_HANGUL = 0,
    eFONT_HANBOOT,
    eFONT_HANGODIC,
    eFONT_HANPIL,
    eFONT_HANSOFT,
    eFONT_DEFAULT,
    eFONT_END
};

//-----------------------------------------------------------------------------
#define ALIGN_X_LEFT    (-1)
#define ALIGN_X_CENTER  (-2)
#define ALIGN_X_RIGHT   (-3)

#define ALIGN_Y_TOP     (-1)
#define ALIGN_Y_CENTER  (-2)
#define ALIGN_Y_BOTTOM  (-3)

//-----------------------------------------------------------------------------
enum eFB_ROTATE {
    eROTATE_0 = 0,
    eROTATE_90,
    eROTATE_180,
    eROTATE_270,
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern void         put_pixel       (fb_info_t *fb, int x, int y, int color);
extern void         draw_text       (fb_info_t *fb, int x, int y,
                                        int f_color, int b_color, int scale, char *fmt, ...);
extern void         draw_line       (fb_info_t *fb, int x, int y, int w, int color);
extern void         draw_rect       (fb_info_t *fb, int x, int y, int w, int h, int lw, int color);
extern void         draw_fill_rect  (fb_info_t *fb, int x, int y, int w, int h, int color);
extern void         set_font        (enum eFONTS_HANGUL s_font);
extern void         fb_set_rotate   (fb_info_t *fb, int rotate);
extern void         fb_set_reversed (fb_info_t *fb, unsigned char status);
extern void         fb_set_fill     (fb_info_t *fb, unsigned char fill);
extern void         fb_clear        (fb_info_t *fb);
extern fb_info_t    *fb_init        (int fb_w, int fb_h, int fb_bpp);

//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------------------------
#endif  // #define __LIB_FB_H__
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
