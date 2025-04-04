/*---------------------------------------------------------------------------*/
/**
 * @file e-paper.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief 2.9" E-paper module project (CH32V203 RISC-V)
 * @version 0.1
 * @date 2024-05-08
 *
 * @copyright Copyright (c) 2022
**/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#include "Arduino.h"

#include "e-paper.h"
#include "display.h"
#include "lib_fb.h"
#include "lib_epd.h"
#include "protocol.h"

/*---------------------------------------------------------------------------*/
//#define EPD_COLOR_MODEL

fb_info_t *epd_fb;
#if defined (EPD_COLOR_MODEL)
    fb_info_t *epd_fb_red;
#endif

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
void blink_status_led (void)
{
    digitalWrite (PORT_ALIVE_LED, !digitalRead(PORT_ALIVE_LED));
}

/*---------------------------------------------------------------------------*/
void watchdog_setup (uint16_t reload)
{
    // 40Khz(LSI Clk) / Prescaler / Reloadcounter (Decrement)
    // 3.2s IWDG reset T = 4000 / (40000 / 32 = 1250)
    IWDG_WriteAccessCmd (IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler (IWDG_Prescaler_32);
    IWDG_SetReload (reload);
    IWDG_ReloadCounter ();
    IWDG_Enable ();
}

/*---------------------------------------------------------------------------*/
void port_init (void)
{
    // Board Alive led init
    pinMode (PORT_ALIVE_LED, OUTPUT);   digitalWrite (PORT_ALIVE_LED, LOW);
    pinMode (PORT_ADC_KEY, INPUT);
}

/*---------------------------------------------------------------------------*/
void setup() {

    port_init ();   delay (2000);

    USBSerial_print ("\r\n ***BOOT1***\r\n");

    /* epd fb init (w, h, bpp) */
    epd_fb = fb_init (296, 128, 1);

    if (epd_fb == NULL)
        USBSerial_print ("epd fb = NULL\r\n");
    else
        USBSerial_print ("epd fb = OK\r\n");

    fb_set_reversed (epd_fb, 1);
    fb_set_rotate   (epd_fb, eROTATE_0);
    fb_clear        (epd_fb);

#if defined (EPD_COLOR_MODEL)
    epd_fb_red = fb_init (296, 128, 1);

    if (epd_fb_red == NULL)
        USBSerial_print ("epd fb red = NULL\r\n");
    else
        USBSerial_print ("epd fb red = OK\r\n");

    fb_reversed (epd_fb_red, 1);
    fb_rotate   (epd_fb_red, eROTATE_0);
    fb_clear    (epd_fb_red);
#endif

    /* GPIO, SPI1 port init */
    epd_port_init ();
    display_clear_screen (epd_fb);

    RTC_Init();

    USBSerial_print ("*** Serial input ready ***\r\n");
    //watchdog_setup (WDT_RELOAD_3_2_S)
    USBSerial_print ("%s EPD partial init status = %d\r\n", __func__, epd_init_partial ());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* EPD Update typ 3 sec */
#define PERIOD_MAIN_LOOP    1000
uint32_t MillisCheck = 0;
char DisplayMode = eMODE_HAN_CALENDAR;

/*---------------------------------------------------------------------------*/
void loop() {
    /* system watchdog */
    //WDT_CLR();

    /* serial data read & write check(ttyACM) */
    if (protocol_data_check (epd_fb)) {
        if (DisplayMode != eMODE_EDIT_SCREEN) {
            // exit sleep mode
            epd_init_partial ();
            DisplayMode = eMODE_EDIT_SCREEN;
        }
        display_update (epd_fb, DisplayMode, 1);
    }
    /* lt8619c check loop (1 sec) */
    if (MillisCheck + PERIOD_MAIN_LOOP < millis()) {
        blink_status_led ();

        if (digitalRead (PORT_ADC_KEY))
            DisplayMode = (DisplayMode + 1) % eMODE_END;

        if (DisplayMode != eMODE_EDIT_SCREEN)
            display_update (epd_fb, DisplayMode, digitalRead (PORT_ADC_KEY));

        MillisCheck = millis();
    }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
