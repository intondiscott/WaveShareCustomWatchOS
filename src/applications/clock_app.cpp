#include <SmartWatch_Structs.h>
#include <close-windows.h>

#define TFT_WIDTH 410
#define TFT_HEIGHT 512
extern OSTime *time_vals;

static void update_clock_label(lv_timer_t *timer)
{
    lv_obj_t *label = (lv_obj_t *)lv_timer_get_user_data(timer);
    lv_label_set_text_fmt(label,
                          "%02d:%02d:%02d%02s\n%02d/%02d/%04d",
                          time_vals->military_time ? time_vals->hours : time_vals->hours - 12,
                          time_vals->minutes,
                          time_vals->seconds,
                          time_vals->military_time ? "" : time_vals->hours > 12 ? "PM"
                                                                                : "AM",
                          time_vals->month,
                          time_vals->day,
                          time_vals->year);
}

static void military_switch_event_cb(lv_event_t *e)
{
    lv_obj_t *military_switch = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *military_label = (lv_obj_t *)lv_event_get_user_data(e);

    time_vals->military_time = lv_obj_has_state(military_switch, LV_STATE_CHECKED);
    lv_label_set_text(military_label, time_vals->military_time ? "24hr" : "12hr");
}

static void create_clock_page(lv_event_t *e)
{
    time_vals->military_time = false;

    if (!lv_obj_is_valid(SmartWatchUI_t.datetime_label))
    {

        SmartWatchUI_t.datetime_label = lv_obj_create(lv_screen_active());
        lv_obj_t *label = lv_label_create(SmartWatchUI_t.datetime_label);

        lv_obj_t *military_switch = lv_switch_create(SmartWatchUI_t.datetime_label);
        lv_obj_set_size(military_switch, 90, 50);
        lv_obj_t *military_label = lv_label_create(SmartWatchUI_t.datetime_label);
        SmartWatchUI_t.close_btn = lv_button_create(SmartWatchUI_t.datetime_label);
        lv_obj_t *label_close = lv_label_create(SmartWatchUI_t.close_btn);
        lv_obj_set_style_bg_color(SmartWatchUI_t.close_btn, lv_color_hex(0xfc0303), LV_PART_MAIN);
        lv_obj_set_size(SmartWatchUI_t.close_btn, TFT_WIDTH, 80);

        lv_obj_set_style_text_font(military_label, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_label_set_text(military_label, time_vals->military_time ? "24hr" : "12hr");
        lv_obj_add_event_cb(
            military_switch,
            military_switch_event_cb,
            LV_EVENT_VALUE_CHANGED,
            (military_switch, military_label));
        lv_obj_align(SmartWatchUI_t.close_btn, LV_ALIGN_BOTTOM_MID, 0, 20);
        lv_obj_align(military_label, LV_ALIGN_BOTTOM_MID, 0, -120);
        lv_obj_align(military_switch, LV_ALIGN_BOTTOM_MID, 0, -70);
        lv_obj_set_style_text_font(label_close, &lv_font_montserrat_40, LV_PART_MAIN);
        lv_obj_center(label_close);

        lv_label_set_text(label_close, "CLOSE");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_48, LV_PART_MAIN);
        lv_obj_center(label);
        lv_label_set_text_fmt(label,
                              "%02d:%02d:%02d%02s\n%02d/%02d/%04d",
                              time_vals->military_time ? time_vals->hours : time_vals->hours - 12,
                              time_vals->minutes,
                              time_vals->seconds,
                              time_vals->military_time ? "" : time_vals->hours > 12 ? "PM"
                                                                                    : "AM",
                              time_vals->month,
                              time_vals->day,
                              time_vals->year);
        lv_obj_set_style_radius(SmartWatchUI_t.datetime_label, 40, LV_PART_MAIN);
        lv_obj_set_style_pad_all(SmartWatchUI_t.datetime_label, 20, LV_PART_MAIN);
        lv_obj_set_size(SmartWatchUI_t.datetime_label, TFT_WIDTH - 0, TFT_HEIGHT - 0);
        lv_obj_center(SmartWatchUI_t.datetime_label);
        SmartWatchUI_t.datetime_timer = lv_timer_create(update_clock_label, 1000, label);
        lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
    }
}