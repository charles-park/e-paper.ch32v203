/*---------------------------------------------------------------------------*/
/**
 * @file lib_epd.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief 2.9" E-paper module project (CH32V203 RISC-V)
 * @version 0.1
 * @date 2024-05-08
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifndef __LIB_EPD_H__
#define __LIB_EPD_H__

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// CH32V203(RISC-V) PORT Config
/*---------------------------------------------------------------------------*/
#define PORT_EPD_SPI        SPI1
#define PORT_EPD_BUSY       PA2
#define PORT_EPD_RESET      PA3
/* SPI NSS */
#define PORT_EPD_CS         PA4
/* SPI MCLK */
#define PORT_EPD_SCLK       PA5
/* 1 = DATA, 1 = COMMAND */
#define PORT_EPD_DC         PA6
/* SPI MOSI */
#define PORT_EPD_SDA        PA7

/*---------------------------------------------------------------------------*/
// E-Paper register (2.9")
/*---------------------------------------------------------------------------*/
enum EPD_REG {
    REG_DRIVER_OUTPUT = 0x01,
    REG_DEEP_SLEEP = 0x10,
    REG_DATA_ENTRYMODE = 0x11,
    REG_SOFTWARE_RESET = 0x12,
    REG_TEMPERATURE_SENSOR = 0x18,
    REG_MASTER_ACTIVATION = 0x20,
    REG_DISPLAY_UPDATE1 = 0x21,
    REG_DISPLAY_UPDATE2 = 0x22,
    REG_LUT_REGISTER = 0x32,
    REG_SET_RAMX = 0x4E,
    REG_SET_RAMY = 0x4F,
    REG_RAMX_ADDRESS = 0x44,
    REG_RAMY_ADDRESS = 0x45,
    REG_BOADER_WAVEFORM = 0x3C,
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
extern uint8_t epd_busy            (void);
extern void    epd_hw_reset        (void);
extern void    epd_spi_write       (const uint8_t d);
extern void    epd_write_data      (const uint8_t *d, uint16_t cnt, uint8_t d_reversed);
extern void    epd_write_cmd       (const uint8_t cmd, const uint8_t *d, uint16_t cnt);
extern void    epd_set_pos         (const uint16_t x, const uint16_t y);
extern uint8_t epd_power           (uint8_t status);
extern void    epd_deep_sleep      (uint8_t mode);
extern void    epd_update          (void);
extern void    epd_update_partial  (uint8_t *fb_data, uint16_t fb_size);
extern uint8_t epd_init_partial    (void);
extern void    epd_color_update    (uint8_t *fb_data, uint16_t fb_size,
                                    uint8_t *fb_color_data, uint16_t fb_color_size);
extern void    epd_mono_update     (uint8_t *fb_data, uint16_t fb_size);
extern uint8_t epd_init            (void);
extern void    epd_port_init       (void);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
#endif  // #ifndef __LIB_EPD_H__
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
