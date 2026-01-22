#include <SmartWatch_Structs.h>
#include <close-windows.h>
#define TFT_WIDTH 410
#define TFT_HEIGHT 512

extern OSTime *time_vals;

static void create_calendar_page(lv_event_t *e)
{
    if (!lv_obj_is_valid(SmartWatchUI_t.calendar))
    {
        SmartWatchUI_t.calendar = lv_obj_create(lv_screen_active());
        lv_obj_t *calendar = lv_calendar_create(SmartWatchUI_t.calendar);
        SmartWatchUI_t.close_btn = lv_button_create(SmartWatchUI_t.calendar);
        lv_obj_set_style_bg_color(SmartWatchUI_t.close_btn, lv_color_hex(0xfc0303), LV_PART_MAIN);
        lv_obj_t *label_close = lv_label_create(SmartWatchUI_t.close_btn);
        lv_obj_set_style_text_font(label_close, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_label_set_text(label_close, LV_SYMBOL_TRASH);
        lv_calendar_set_today_date(calendar, time_vals->year, time_vals->month, time_vals->day);

        lv_calendar_set_showed_date(calendar, time_vals->year, time_vals->month);
        lv_obj_align(SmartWatchUI_t.close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
        lv_obj_set_size(SmartWatchUI_t.calendar, TFT_WIDTH - 20, TFT_HEIGHT - 60);
        lv_obj_set_size(calendar, 220, TFT_HEIGHT - 100);
        lv_obj_center(SmartWatchUI_t.calendar);
        lv_obj_align(calendar, LV_ALIGN_CENTER, -20, 0);
        /*Highlight a few days*/
        static lv_calendar_date_t highlighted_days[3]; /*Only its pointer will be saved so should be static*/
        highlighted_days[0].year = time_vals->year;
        highlighted_days[0].month = time_vals->month;
        highlighted_days[0].day = time_vals->day; // Today

        

        lv_calendar_set_highlighted_dates(calendar, highlighted_days, 1);
        lv_calendar_header_dropdown_create(calendar);
        lv_calendar_header_arrow_create(calendar);

               lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
    }
}