#include <lvgl.h>
#include <Arduino.h>
#include <close-windows.h>
#include <SmartWatch_Structs.h>
//extern SmartWatchUI_t SmartWatchUI_t;
#define TFT_WIDTH 410
#define TFT_HEIGHT 512
static void create_contact_page(lv_event_t *e)
{
    if (!lv_obj_is_valid(SmartWatchUI_t.contact))
    {
        SmartWatchUI_t.contact = lv_obj_create(lv_screen_active());

        lv_obj_t *label = lv_label_create(SmartWatchUI_t.contact);
        SmartWatchUI_t.close_btn = lv_button_create(SmartWatchUI_t.contact);
        lv_obj_set_style_bg_color(SmartWatchUI_t.close_btn, lv_color_hex(0xfc0303), LV_PART_MAIN);
        lv_obj_t *label_close = lv_label_create(SmartWatchUI_t.close_btn);
        lv_obj_set_style_text_font(label_close, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_label_set_text(label_close, LV_SYMBOL_TRASH);
        lv_label_set_text(label, "Contact Page");
        lv_obj_align(SmartWatchUI_t.close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_size(SmartWatchUI_t.contact, TFT_WIDTH-20, TFT_HEIGHT-60);
        lv_obj_center(SmartWatchUI_t.contact);
        
        lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
    }
}