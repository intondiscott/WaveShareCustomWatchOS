#include <SmartWatch_Structs.h>
#include <close-windows.h>

#define TFT_WIDTH 410
#define TFT_HEIGHT 512
extern OSTime *time_vals;

static void update_clock_label(lv_timer_t *timer)
{
    lv_obj_t *label = (lv_obj_t *)lv_timer_get_user_data(timer);
    lv_label_set_text_fmt(label, "%02d:%02d:%02d\n%02d/%02d/%04d", time_vals->hours, time_vals->minutes, time_vals->seconds, time_vals->month, time_vals->day, time_vals->year);
}

static void create_clock_page(lv_event_t *e)
{
    
    if (!lv_obj_is_valid(SmartWatchUI_t.datetime_label))
    {
        SmartWatchUI_t.datetime_label = lv_obj_create(lv_screen_active());
        lv_obj_t *label = lv_label_create(SmartWatchUI_t.datetime_label);
        SmartWatchUI_t.close_btn = lv_button_create(SmartWatchUI_t.datetime_label);
        lv_obj_set_style_bg_color(SmartWatchUI_t.close_btn, lv_color_hex(0xfc0303), LV_PART_MAIN);
        lv_obj_t *label_close = lv_label_create(SmartWatchUI_t.close_btn);
        lv_obj_set_style_text_font(label_close, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_label_set_text(label_close, LV_SYMBOL_TRASH);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_PART_MAIN);
        lv_obj_center(label);
        lv_label_set_text_fmt(label, "%02d:%02d:%02d\n%02d/%02d/%04d", time_vals->hours, time_vals->minutes, time_vals->seconds, time_vals->month, time_vals->day, time_vals->year);
        lv_obj_align(SmartWatchUI_t.close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_size(SmartWatchUI_t.datetime_label, TFT_WIDTH - 20, TFT_HEIGHT - 60);
        lv_obj_center(SmartWatchUI_t.datetime_label);
       SmartWatchUI_t.datetime_timer = lv_timer_create(update_clock_label, 500, label);
        lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
    }
    
    
}