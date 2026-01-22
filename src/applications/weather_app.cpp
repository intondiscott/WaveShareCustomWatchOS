#include <SmartWatch_Structs.h>
#include <close-windows.h>
#define TFT_WIDTH 410
#define TFT_HEIGHT 512
static char weather_buffer[7];
extern Weather *weather_vals;

static void update_weather_data()
{
    snprintf(weather_buffer, sizeof(weather_buffer), "%3.2f", (weather_vals->temperature - 273.15) * 9 / 5 + 32);
    lv_label_set_text_fmt(SmartWatchUI_t.temperature_label, "%s°F", weather_buffer);
    snprintf(weather_buffer, sizeof(weather_buffer), "%3.2f", weather_vals->wind_speed);
    lv_label_set_text_fmt(SmartWatchUI_t.wind_speed_label, "Wind Speed: %s MPH", weather_buffer);
    lv_label_set_text_fmt(SmartWatchUI_t.humidity_label, "Hum: %d%%", weather_vals->humidity);
}

static void create_weather_page(lv_event_t *e)
{
    if (!lv_obj_is_valid(SmartWatchUI_t.weather))
    {
        SmartWatchUI_t.weather = lv_obj_create(lv_screen_active());
        lv_obj_t *weather_win = lv_obj_create(SmartWatchUI_t.weather);
        lv_obj_set_size(weather_win, TFT_WIDTH - 20, TFT_HEIGHT - 60);
        lv_obj_set_flex_flow(weather_win, LV_FLEX_FLOW_COLUMN);
        SmartWatchUI_t.wind_speed_label = lv_label_create(weather_win);
        SmartWatchUI_t.humidity_label = lv_label_create(weather_win);
        SmartWatchUI_t.temperature_label = lv_label_create(weather_win);

        lv_obj_set_style_text_font(SmartWatchUI_t.temperature_label, &lv_font_montserrat_30, LV_PART_MAIN);
        lv_obj_set_style_text_font(SmartWatchUI_t.wind_speed_label, &lv_font_montserrat_30, LV_PART_MAIN);
        lv_obj_set_style_text_font(SmartWatchUI_t.humidity_label, &lv_font_montserrat_30, LV_PART_MAIN);
        lv_obj_set_style_margin_top(SmartWatchUI_t.wind_speed_label, 40, LV_PART_MAIN);

        SmartWatchUI_t.close_btn = lv_button_create(SmartWatchUI_t.weather);
        lv_obj_set_style_bg_color(SmartWatchUI_t.close_btn, lv_color_hex(0xfc0303), LV_PART_MAIN);
        lv_obj_t *label_close = lv_label_create(SmartWatchUI_t.close_btn);
        lv_obj_set_style_text_font(label_close, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_label_set_text(label_close, LV_SYMBOL_TRASH);
        snprintf(weather_buffer, sizeof(weather_buffer), "%3.2f", (weather_vals->temperature - 273.15) * 9 / 5 + 32);
        lv_label_set_text_fmt(SmartWatchUI_t.temperature_label, "%s°F", weather_buffer);
        snprintf(weather_buffer, sizeof(weather_buffer), "%3.2f", weather_vals->wind_speed);
        lv_label_set_text_fmt(SmartWatchUI_t.wind_speed_label, "Wind Speed: %s MPH", weather_buffer);
        lv_label_set_text_fmt(SmartWatchUI_t.humidity_label, "Hum: %d%%", weather_vals->humidity);
        lv_obj_align(SmartWatchUI_t.close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_size(SmartWatchUI_t.weather, 400, 400);
        lv_obj_center(SmartWatchUI_t.weather);
        SmartWatchUI_t.datetime_timer = lv_timer_create((lv_timer_cb_t)update_weather_data, 1000, NULL);
        lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
    }
}