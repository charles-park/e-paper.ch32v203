/*---------------------------------------------------------------------------*/
/**
 * @file lib_epd.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief 2.9" E-paper module project (CH32V203 RISC-V)
 * @version 0.1
 * @date 2024-05-08
 *
 * @Gitub: https://github.com/WeActStudio/WeActStudio.EpaperModule
 * @product: https://ko.aliexpress.com/item/1005004644515880.html?spm=a2g0o.detail.pcDetailBottomMoreOtherSeller.2.9d92h53Rh53Ruc&gps-id=pcDetailBottomMoreOtherSeller&scm=1007.40000.326746.0&scm_id=1007.40000.326746.0&scm-url=1007.40000.326746.0&pvid=78c0b815-53ae-4dd6-a3d7-8469fcd48f03&_t=gps-id:pcDetailBottomMoreOtherSeller,scm-url:1007.40000.326746.0,pvid:78c0b815-53ae-4dd6-a3d7-8469fcd48f03,tpp_buckets:668%232846%238115%23841&pdp_npi=4%40dis%21KRW%2111103%2111103%21%21%2157.51%2157.51%21%40214112dd17161811532316553ebafc%2112000031468032209%21rec%21KR%211614259518%21&utparam-url=scene%3ApcDetailBottomMoreOtherSeller%7Cquery_from%3A
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"

#include "lib_epd.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
const uint8_t REG_DIRVER_OUTPUT_DATA[] = {
    0x27, 0x01, 0x01
};
const uint8_t REG_DATA_ENTRYMODE_DATA[] = {
    0x01
};

const uint8_t REG_TEMPERATURE_SENSOR_DATA[] = {
    0x80
};

const uint8_t REG_DISPLAY_UPDATE1_DATA[] = {
    0x00, 0x80
};

/* EPD display partial init */
const uint8_t REG_LUT_REGISTER_DATA[] = {
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
};

const uint8_t REG_RAMX_ADDRESS_DATA[] = {
    /* (15 + 1) * 8 = 128 */
    0x00, 0x0F
};

const uint8_t REG_RAMY_ADDRESS_DATA[] = {
    /* (295 + 1) = 296 */
    0x27, 0x01, 0x00, 0x00
//     0x00, 0x00, 0x27, 0x01,
};

const uint8_t REG_BOADER_WAVEFORM_DATA[] = {
    0x05
};

/*---------------------------------------------------------------------------*/
volatile uint8_t DeepSleepEPD = 1;  /* Power on default value */

/*---------------------------------------------------------------------------*/
uint8_t epd_busy (void)
{
    uint32_t timeout = 40000;   /* 4 sec wait */

    while (digitalRead (PORT_EPD_BUSY) && timeout--)    delay(1);

    if (!timeout)
        USBSerial_print ("error E-Paper busy.\r\n");

    return timeout ? 0 : 1;
}

/*---------------------------------------------------------------------------*/
void epd_hw_reset (void)
{
    digitalWrite (PORT_EPD_RESET, LOW);     delay (100);
    digitalWrite (PORT_EPD_RESET, HIGH);    delay (100);
    DeepSleepEPD = 0;
}

/*---------------------------------------------------------------------------*/
void epd_spi_write (const uint8_t d)
{
    while (SPI_I2S_GetFlagStatus (PORT_EPD_SPI, SPI_I2S_FLAG_TXE) == RESET) ;
    SPI_I2S_SendData (PORT_EPD_SPI, d);
    while (SPI_I2S_GetFlagStatus (PORT_EPD_SPI, SPI_I2S_FLAG_BSY) != RESET) ;
}

/*---------------------------------------------------------------------------*/
void epd_write_data (const uint8_t *d, uint16_t cnt, uint8_t d_reversed)
{
    digitalWrite (PORT_EPD_DC, HIGH);   digitalWrite (PORT_EPD_CS, LOW);

    for (uint16_t i = 0; i < cnt; i++)  epd_spi_write (d_reversed ? (~d[i]) : (d[i]));

    digitalWrite (PORT_EPD_CS, HIGH);   digitalWrite (PORT_EPD_DC, HIGH);
}

/*---------------------------------------------------------------------------*/
void epd_write_cmd (const uint8_t cmd, const uint8_t *d, uint16_t cnt)
{
    // register setup
    digitalWrite (PORT_EPD_DC, LOW);    digitalWrite (PORT_EPD_CS, LOW);

    epd_spi_write (cmd);

    digitalWrite (PORT_EPD_CS, HIGH);   digitalWrite (PORT_EPD_DC, HIGH);

    if (cnt)    epd_write_data (d, cnt, 0);
}

/*---------------------------------------------------------------------------*/
void epd_set_pos (const uint16_t x, const uint16_t y)
{
    uint8_t d[2];
    d[0]= x;
    epd_write_cmd (REG_SET_RAMX, d, 1);
    d[0] = y & 0xFF;    d[1] = (y >> 8) & 0x01;
    epd_write_cmd (REG_SET_RAMY, d, 2);
}

/*---------------------------------------------------------------------------*/
uint8_t epd_power (uint8_t status)
{
    /* Activate Display update sequency */
    uint8_t d = status ? 0xF8 : 0x83;

    /* Display update control */
    epd_write_cmd  (REG_DISPLAY_UPDATE2, &d, 1);
    epd_write_cmd  (REG_MASTER_ACTIVATION, NULL, 0);
    return epd_busy ();
}

/*---------------------------------------------------------------------------*/
void epd_deep_sleep (uint8_t mode)
{
    epd_power (0);  /* power off */
    epd_write_cmd (REG_DEEP_SLEEP, &mode, 1);
    DeepSleepEPD = 1;
}

/*---------------------------------------------------------------------------*/
void epd_update (void)
{
    /* Activate Display update sequency */
    uint8_t d = 0xF7;

    /* Display update control */
    epd_write_cmd  (REG_DISPLAY_UPDATE2, &d, 1);
    epd_write_cmd  (REG_MASTER_ACTIVATION, NULL, 0);
    epd_busy ();
}

/*---------------------------------------------------------------------------*/
void epd_update_partial (uint8_t *fb_data, uint16_t fb_size)
{
    /* Activate Display update sequency */
    uint8_t d = 0xCC;

    epd_set_pos (0, 0); epd_write_cmd  (0x24, fb_data, fb_size);

    /* Display update control */
    epd_write_cmd  (REG_DISPLAY_UPDATE2, &d, 1);
    epd_write_cmd  (REG_MASTER_ACTIVATION, NULL, 0);
    epd_busy ();

    epd_set_pos (0, 0); epd_write_cmd  (0x26, fb_data, fb_size);
}

/*---------------------------------------------------------------------------*/
uint8_t epd_init_partial (void)
{
    // busy is return 1
    if (epd_init())     return 1;

    epd_write_cmd (REG_LUT_REGISTER,
        REG_LUT_REGISTER_DATA, sizeof(REG_LUT_REGISTER_DATA));

    return 0;
}

/*---------------------------------------------------------------------------*/
void epd_color_update  (uint8_t *fb_data, uint16_t fb_size,
                        uint8_t *fb_color_data, uint16_t fb_color_size)
{
    epd_set_pos (0, 0);
    epd_write_cmd  (0x24, NULL, 0);
    epd_write_data (fb_data, fb_size, 0); // data reversed
    epd_set_pos (0, 0);
    epd_write_cmd  (0x26, NULL, 0);
    epd_write_data (fb_color_data, fb_color_size, 1); // data reversed
    epd_update ();
}

/*---------------------------------------------------------------------------*/
void epd_mono_update  (uint8_t *fb_data, uint16_t fb_size)
{
    epd_set_pos (0, 0); epd_write_cmd  (0x26, fb_data, fb_size);
    epd_set_pos (0, 0); epd_write_cmd  (0x24, fb_data, fb_size);
    epd_update ();
}

/*---------------------------------------------------------------------------*/
uint8_t epd_init (void)
{
    if (DeepSleepEPD)   epd_hw_reset ();

    // software reset (0x12). delay 10ms
    epd_write_cmd (REG_SOFTWARE_RESET, NULL, 0);    delay (10);

    if (epd_busy()) return 1;

    // Driver output control (0x01)
    epd_write_cmd (REG_DRIVER_OUTPUT,
        REG_DIRVER_OUTPUT_DATA, sizeof(REG_DIRVER_OUTPUT_DATA));

    // Data entrymode (0x11)
    epd_write_cmd  (REG_DATA_ENTRYMODE,
        REG_DATA_ENTRYMODE_DATA, sizeof(REG_DATA_ENTRYMODE_DATA));

    // set RAM-X address start/end position
    epd_write_cmd  (REG_RAMX_ADDRESS,
        REG_RAMX_ADDRESS_DATA, sizeof(REG_RAMX_ADDRESS_DATA));

    // set RAM-Y address start/end position
    epd_write_cmd  (REG_RAMY_ADDRESS,
        REG_RAMY_ADDRESS_DATA, sizeof(REG_RAMY_ADDRESS_DATA));

    // Border wave form
    epd_write_cmd  (REG_BOADER_WAVEFORM,
        REG_BOADER_WAVEFORM_DATA, sizeof(REG_BOADER_WAVEFORM_DATA));

    // Display update control 1
    epd_write_cmd  (REG_DISPLAY_UPDATE1,
        REG_DISPLAY_UPDATE1_DATA, sizeof(REG_DISPLAY_UPDATE1_DATA));

    // Read built-in temperature sensor
    epd_write_cmd  (REG_TEMPERATURE_SENSOR,
        REG_TEMPERATURE_SENSOR_DATA, sizeof(REG_TEMPERATURE_SENSOR_DATA));

    epd_set_pos (0, 0);

    // epd power on/off (return busy status)
    return epd_power (1);
}

/*---------------------------------------------------------------------------*/
void epd_port_init (void)
{
    SPI_InitTypeDef spiConfig = {0};

    // E-Paper module connection (Use SPI1)
    pinMode (PORT_EPD_BUSY, INPUT_PULLUP);
    pinMode (PORT_EPD_RESET, OUTPUT);   digitalWrite (PORT_EPD_RESET, LOW);
    pinMode (PORT_EPD_CS, OUTPUT);      digitalWrite (PORT_EPD_CS, LOW);
    pinMode (PORT_EPD_DC, OUTPUT);      digitalWrite (PORT_EPD_CS, HIGH);

    pinMode (PORT_EPD_SCLK, FUNC_PP);   pinMode (PORT_EPD_SDA, FUNC_PP);

    /* SPI CLOCK = SYSTEM Clock / divide (SPI_BaudRatePrescaler) */
    /* 96000000 Hz / 16 = 6000000 Hz*/
    /* EPD Write Max clock = 20Mhz */
    spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    spiConfig.SPI_DataSize = SPI_DataSize_8b;
    spiConfig.SPI_Direction = SPI_Direction_1Line_Tx;
    spiConfig.SPI_FirstBit = SPI_FirstBit_MSB;
    spiConfig.SPI_Mode = SPI_Mode_Master;
    spiConfig.SPI_CPOL = SPI_CPOL_High;
    spiConfig.SPI_CPHA = SPI_CPHA_2Edge;
    spiConfig.SPI_NSS = SPI_NSS_Soft;

    /* configure spi1 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    SPI_Init(PORT_EPD_SPI, &spiConfig);
    SPI_Cmd(PORT_EPD_SPI, ENABLE);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
