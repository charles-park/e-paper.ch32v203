/*---------------------------------------------------------------------------*/
/**
 * @file flash.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief E-Paper User data control
 * @version 0.1
 * @date 2024-05-23
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"
#include "flash.h"

#include <string.h>

#if 0
EPD_FB_SIZE = (EPD_W_SIZE * EPD_H_SIZE / 8);



union {
} cfg;

flash_write (page, data);
flash_read  (start_page, data, size);
#endif
/*--------------------- ------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#if 0
#define FLASH_CFG_ADDR      0x0800C000  /* upper 48K */
#define FLASH_PAGE_SIZE     0x100       /* 256 bytes */
#define FLASH_END_ADDR      0x08010000  /* 64K size */

#define CFG_SIGNATURE       0xA5A55A5A
#define USER_SCREEN_COUNT   4

#define EPD_FB_SIZE         (296 * 16)  //(EPD_FB_W * (EPD_FB_H / 8))

struct sitems {
    unsigned long flag;
    unsigned char data [EPD_FB_SIZE];  // fb_size
    unsigned char xor, add;
};

union uitems {
    struct sitems items;
    unsigned char bytes[sizeof(struct sitems)];
};

union uitems CFGItems;
#endif

/*---------------------------------------------------------------------------*/
#define FLASH_CFG_ADDR      0x0800C000  /* upper 48K */
#define FLASH_PAGE_SIZE     0x100       /* 256 bytes */
#define USER_SCREEN_COUNT   4

#define EPD_FB_SIZE         (296 * 16)  //(EPD_FB_W * (EPD_FB_H / 8))

/*---------------------------------------------------------------------------*/
int cal_page_count (int item_bytes)
{
    int page_count;

    // cal use flash page count
    page_count  = (item_bytes / FLASH_PAGE_SIZE);
    page_count += (item_bytes % FLASH_PAGE_SIZE) ? 1:0;

    return page_count;
}

/*---------------------------------------------------------------------------*/
uint32_t flash_page_addr (uint32_t page)
{
    return (FLASH_CFG_ADDR + page * FLASH_PAGE_SIZE);
}

/*---------------------------------------------------------------------------*/
void flash_page_read (uint32_t page_addr, uint32_t *r_buf)
{
    for (int32_t i = 0; i < FLASH_PAGE_SIZE / sizeof(uint32_t); i++) {
        r_buf [i] = (*(uint32_t *)(page_addr + (i * 4)));
    }
}

/*---------------------------------------------------------------------------*/
void flash_page_write (uint32_t page_addr, uint32_t *w_buf)
{
    FLASH_Unlock_Fast ();
    FLASH_ErasePage_Fast   (page_addr);
    FLASH_ProgramPage_Fast (page_addr, w_buf);
    FLASH_Lock_Fast ();
}

/*---------------------------------------------------------------------------*/
void save_user_screen (unsigned char *pfb, char screen)
{
    int page_cnt = cal_page_count (EPD_FB_SIZE), start_page, cnt, size;
    uint32_t buf [(FLASH_PAGE_SIZE / 4)];

    // cal page offset
    if (screen < USER_SCREEN_COUNT) start_page = page_cnt * screen;
    else                            start_page = page_cnt;

    for (size = EPD_FB_SIZE, cnt = 0; cnt < page_cnt; cnt++) {
        memset (buf, 0, sizeof(buf));

        if (size < FLASH_PAGE_SIZE) {
            memcpy (buf, &pfb[cnt * FLASH_PAGE_SIZE], size);
            size = 0;
        } else {
            memcpy (buf, &pfb[cnt * FLASH_PAGE_SIZE], FLASH_PAGE_SIZE);
            size -= FLASH_PAGE_SIZE;
        }
        flash_page_write (flash_page_addr(start_page + cnt), buf);
        if (size == 0)  break;
    }
}

/*---------------------------------------------------------------------------*/
void load_user_screen (unsigned char *pfb, char screen)
{
    int page_cnt = cal_page_count (EPD_FB_SIZE), start_page, cnt, size;
    uint32_t buf [(FLASH_PAGE_SIZE / 4)];

    // cal page offset
    if (screen < USER_SCREEN_COUNT) start_page = page_cnt * screen;
    else                            start_page = page_cnt;

    for (size = EPD_FB_SIZE, cnt = 0; cnt < page_cnt; cnt++) {
        memset (buf, 0, sizeof(buf));
        flash_page_read (flash_page_addr(start_page + cnt), buf);

        if (size < FLASH_PAGE_SIZE) {
            memcpy (&pfb[cnt * FLASH_PAGE_SIZE], buf, size);
            size = 0;
        } else {
            memcpy (&pfb[cnt * FLASH_PAGE_SIZE], buf, FLASH_PAGE_SIZE);
            size -= FLASH_PAGE_SIZE;
        }
        if (size == 0)  break;
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
