#include <lvgl.h>
#include <SmartWatch_Structs.h>
#pragma once
static void close_window_cb(lv_event_t *e)
{

    // lv_obj_clean(lv_obj_get_parent((lv_obj_t *)lv_event_get_target(e)));
    lv_obj_t *parent = lv_obj_get_parent((lv_obj_t *)lv_event_get_target(e));

    if (SmartWatchUI_t.datetime_timer != nullptr)
    {
        Serial.println("Timer is valid, deleting...");
        lv_timer_delete(SmartWatchUI_t.datetime_timer);
        SmartWatchUI_t.datetime_timer = nullptr;
    }

        lv_obj_del(parent);
}