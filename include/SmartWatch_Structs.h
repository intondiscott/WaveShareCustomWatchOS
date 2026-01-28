#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include <lvgl.h>
#include <XPowersLib.h>
#pragma once

struct
{
    XPowersPMU power;
    Arduino_DataBus *bus = new Arduino_ESP32QSPI(
        LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
        LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

    Arduino_GFX *gfx = new Arduino_CO5300(bus, LCD_RESET /* RST */,
                                          0 /* rotation */, LCD_WIDTH, LCD_HEIGHT,
                                          22 /* col_offset1 */,
                                          0 /* row_offset1 */,
                                          0 /* col_offset2 */,
                                          0 /* row_offset2 */);

    lv_obj_t
        *main_screen,
        *nav_screen,
        *battery_label,
        *datetime_label,
        *bat_bar,
        *bat_img,
        *wallpaper,
        *root_page,
        *low_bat_img,
        *button_text,
        *charging_img,
        *messages,
        *calculator,
        *calendar,
        *weather,
        *close_btn,
        *phone,
        *setting,
        *icons[20],
        *connection_status,
        *bluetooth_status,
        *lora_status,
        *weather_conditions,
        *temperature_label,
        *wind_speed_label,
        *humidity_label,
        *contact;
    char bat[6];

    lv_timer_t *datetime_timer;
} static SmartWatchUI_t;

struct Weather
{
    float temperature = 255.372; // kelvin temp
    int humidity = 0;
    float wind_speed = 0.0;
    char icon[5];
};

struct Settings
{
    uint8_t brightness = 100;
    bool radio_communications = false;
    bool wifi_communications = false;
    bool bluetooth_communications = false;
};

struct OSTime
{
    uint8_t hours = 12;
    uint8_t minutes = 59;
    uint8_t seconds = 9;
    uint8_t day = 18;
    uint8_t month = 1;
    uint16_t year = 2026;
    bool military_time = true;
};
