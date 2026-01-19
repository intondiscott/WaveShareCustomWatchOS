#include <Wire.h>
#include <Arduino.h>
#include "pin_config.h"
#include <lvgl.h>

#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "../lv_conf.h"
#include <demos/lv_demos.h>
#include "SensorPCF85063.hpp"
#include "HWCDC.h"
#include <SmartWatch_Structs.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "credentials.c"

// #include "config.h"
// #define USE_SMARTWATCH_OS_ICONS 1
// #define USE_TDECK_OS_ICONS 0
TaskHandle_t lvglTaskHandler, sensorTaskHandler, wifiTaskHandler;
unsigned long lastTickMillis = 0;
#ifdef USE_SMARTWATCH_OS_ICONS
#include <../assets/images/smartwatch_os_images/weather.h>
#include <../assets/images/smartwatch_os_images/calculator.h>
#include <../assets/images/smartwatch_os_images/calendar.h>
#include <../assets/images/smartwatch_os_images/messages.h>
#include <../assets/images/smartwatch_os_images/telephone.h>
#include <../assets/images/smartwatch_os_images/book.h>
#include <../assets/images/smartwatch_os_images/setting.h>
#include <../assets/images/smartwatch_os_images/clock80.h>
// #include <../assets/images/smartwatch_os_images/internet.h>
// #include <../assets/images/smartwatch_os_images/no_wifi.h>
// #include <../assets/images/smartwatch_os_images/antenna.h>
// #include <../assets/images/smartwatch_os_images/full_battery.h>
// #include <../assets/images/smartwatch_os_images/bat_two_bars.h>
// #include <../assets/images/smartwatch_os_images/low_bat.h>
// #include <../assets/images/smartwatch_os_images/critical_low_bat.h>
// #include <../assets/images/smartwatch_os_images/charging.h>
#include <../assets/images/smartwatch_os_images/nature_wallpaper.h>
// #include <../assets/images/smartwatch_os_images/bluetooth.h>
#endif
#ifdef USE_TDECK_OS_ICONS
#include <../assets/images/critical_low_bat.h>
#include <../assets/images/charging.h>
#include <../assets/images/full_battery.h>
#include <../assets/images/bat_two_bars.h>
#include <../assets/images/low_bat.h>
#include <../assets/images/wallpaper.h>
#include <../assets/images/antenna.h>
#include <../assets/images/weather.h>
#include <../assets/images/book.h>
#include <../assets/images/internet.h>
#include <../assets/images/no_wifi.h>
#include <../assets/images/setting.h>
#include <../assets/images/telephone.h>
#include <../assets/images/messages.h>
#include <../assets/images/calculator.h>
#include <../assets/images/calendar.h>
#include <../assets/images/bluetooth.h>
#include <../assets/images/clock60.h>
#endif

#include <applications/contact_app.cpp>
#include <applications/messages_app.cpp>
#include <applications/phone_app.cpp>
#include <applications/weather_app.cpp>
#include <applications/calendar_app.cpp>
#include <applications/calculator_app.cpp>
#include <applications/settings_app.cpp>
#include <applications/clock_app.cpp>

HWCDC USBSerial;
#define TFT_WIDTH 410
#define TFT_HEIGHT 512

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
uint32_t screenWidth;
uint32_t screenHeight;
uint32_t bufSize;
lv_display_t *disp;
lv_color_t *disp_draw_buf;
bool isPressed = false;
// lv_obj_t *label; // Global label object
SensorPCF85063 rtc;
uint32_t lastMillis;

// #define DIRECT_RENDER_MODE
Weather *weather_vals = new Weather;
OSTime *time_vals = new OSTime;
Settings *setting_values = new Settings;

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void)
{
  FT3168->IIC_Interrupt_Flag = true;
}

uint32_t millis_cb(void)
{
  return millis();
}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  // lv_draw_sw_rgb565_swap(px_map, w * h);
  SmartWatchUI_t.gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

  /*Call it to tell LVGL you are ready*/
  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
  int32_t touchX = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t touchY = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (FT3168->IIC_Interrupt_Flag == true)
  {
    FT3168->IIC_Interrupt_Flag = false;
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    USBSerial.print("Data x ");
    USBSerial.print(touchX);

    USBSerial.print("Data y ");
    USBSerial.println(touchY);
  }
  else
  {
    data->state = LV_INDEV_STATE_REL;
  }
}

void rounder_event_cb(lv_event_t *e)
{
  lv_area_t *area = (lv_area_t *)lv_event_get_param(e);
  uint16_t x1 = area->x1;
  uint16_t x2 = area->x2;

  uint16_t y1 = area->y1;
  uint16_t y2 = area->y2;

  // round the start of coordinate down to the nearest 2M number
  area->x1 = (x1 >> 1) << 1;
  area->y1 = (y1 >> 1) << 1;
  // round the end of coordinate up to the nearest 2N+1 number
  area->x2 = ((x2 >> 1) << 1) + 1;
  area->y2 = ((y2 >> 1) << 1) + 1;
}

void screen_update()
{

  uint32_t battery_percentage = SmartWatchUI_t.power.getBatteryPercent();
  RTC_DateTime now = rtc.getDateTime();
  time_vals->hours = now.getHour();
  time_vals->minutes = now.getMinute();
  time_vals->seconds = now.getSecond();
  time_vals->day = now.getDay();
  time_vals->month = now.getMonth();
  time_vals->year = now.getYear();
  snprintf(SmartWatchUI_t.bat, sizeof(SmartWatchUI_t.bat), "%d", battery_percentage);
  if (WiFi.status() == WL_CONNECTED)
  {
    lv_label_set_text(SmartWatchUI_t.connection_status, LV_SYMBOL_WIFI);
  }
  else
  {
    lv_label_set_text(SmartWatchUI_t.connection_status, "");
  }
  if (setting_values->bluetooth_communications)
  {
    lv_label_set_text(SmartWatchUI_t.bluetooth_status, LV_SYMBOL_BLUETOOTH);
  }
  else
  {
    lv_label_set_text(SmartWatchUI_t.bluetooth_status, "");
  }
  if (setting_values->radio_communications)
  {
    lv_label_set_text(SmartWatchUI_t.lora_status, LV_SYMBOL_GPS);
  }
  else
  {
    lv_label_set_text(SmartWatchUI_t.lora_status, "");
  }

  lv_label_set_text_fmt(SmartWatchUI_t.battery_label, "%s%%", SmartWatchUI_t.bat);

  if (SmartWatchUI_t.power.isCharging())
  {
    lv_label_set_text(SmartWatchUI_t.bat_img, LV_SYMBOL_CHARGE);
  }
  else
  {

    switch (battery_percentage)
    {
    case 80 ... 100:
      lv_label_set_text(SmartWatchUI_t.bat_img, LV_SYMBOL_BATTERY_FULL);
      break;
    case 40 ... 79:
      lv_label_set_text(SmartWatchUI_t.bat_img, LV_SYMBOL_BATTERY_3);
      break;
    case 15 ... 39:
      lv_label_set_text(SmartWatchUI_t.bat_img, LV_SYMBOL_BATTERY_2);
      break;
    case 0 ... 14:
      lv_label_set_text(SmartWatchUI_t.bat_img, LV_SYMBOL_BATTERY_1);
      break;
    default:
      break;
    }
  }
  
}
void sensorsTask(void *pvParams)
{

  while (1)
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http2, http3;
      int httpCode2, httpCode3;
      float x, y, z;

      String battery = "\"pin1\":\"" + (String)SmartWatchUI_t.bat + "\",";
      String charging = "\"pin2\":\"" + (String)SmartWatchUI_t.power.getBatteryPercent() + "\",";
      String chip_temp = "\"pin3\":\"84\"";

      http2.begin("http://192.168.0.114:8080/api/v1/devices/1");
      http3.begin("http://192.168.0.223:8080/api/v1/devices/1");
      http2.addHeader("Content-type", "application/json");
      http3.addHeader("Content-type", "application/json");
      http2.PUT(
          "{" +

          battery +
          charging +
          chip_temp +
          "}");
      http3.PUT(
          "{" +

          battery +
          // charging +
          // chip_temp +
          "}");
      http2.end();
      http3.end();
    }
    else
    {

      if (setting_values->wifi_communications)
      {
        WiFi.mode(WIFI_STA);
        WiFi.begin(MY_SECRET_SSID, MY_SECRET_PASSWORD);
        Serial.print("Connecting to WiFi ..");
        while (WiFi.status() != WL_CONNECTED)
        {
          Serial.print('.');
          delay(1000);
        }
        Serial.println(WiFi.localIP());
      }
    }
    vTaskDelay(2);
  }
}

void wifiTask(void *pvParams)
{

  while (1)
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http1;

      String server_path1 = "https://api.openweathermap.org/data/2.5/weather?lat=41.3165&lon=-73.0932&appid=";

      http1.begin(server_path1 + MY_SECRET_API_KEY);

      int httpCode = http1.GET();

      if (httpCode > 0)
      {
        String payload = http1.getString();

        JsonDocument doc;
        deserializeJson(doc, payload);
        weather_vals->temperature = doc["main"]["temp"];
        weather_vals->humidity = doc["main"]["humidity"];
        weather_vals->wind_speed = doc["wind"]["speed"];

        Serial.println(payload);
      }
      else
      {
        Serial.println(httpCode);

        if (setting_values->wifi_communications)
        {
          WiFi.mode(WIFI_STA);
          WiFi.begin(MY_SECRET_SSID, MY_SECRET_PASSWORD);
          Serial.print("Connecting to WiFi ..");
          while (WiFi.status() != WL_CONNECTED)
          {
            Serial.print('.');
            delay(1000);
          }
          Serial.println(WiFi.localIP());
        }
        http1.end();
      }
    }
    vTaskDelay(10000);
  }
}

void drawUI()
{

  LV_IMAGE_DECLARE(nature_wallpaper);
  LV_IMAGE_DECLARE(book);
  LV_IMAGE_DECLARE(setting);
  LV_IMAGE_DECLARE(clock80);
  LV_IMAGE_DECLARE(telephone);
  LV_IMAGE_DECLARE(messages);
  LV_IMAGE_DECLARE(calculator);
  LV_IMAGE_DECLARE(calendar);
  LV_IMAGE_DECLARE(weather);
  static lv_style_t button_click;
  lv_style_init(&button_click);
  lv_style_set_image_recolor_opa(&button_click, LV_OPA_30);
  SmartWatchUI_t.main_screen = lv_image_create(lv_screen_active());

  SmartWatchUI_t.nav_screen = lv_obj_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.battery_label = lv_label_create(SmartWatchUI_t.nav_screen);

  SmartWatchUI_t.connection_status = lv_label_create(SmartWatchUI_t.nav_screen);
  SmartWatchUI_t.bluetooth_status = lv_label_create(SmartWatchUI_t.nav_screen);
  SmartWatchUI_t.lora_status = lv_label_create(SmartWatchUI_t.nav_screen);
  SmartWatchUI_t.bat_img = lv_label_create(SmartWatchUI_t.nav_screen);

  lv_obj_set_style_text_color(SmartWatchUI_t.connection_status, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_color(SmartWatchUI_t.bluetooth_status, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_color(SmartWatchUI_t.lora_status, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  lv_obj_set_style_text_color(SmartWatchUI_t.bat_img, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  lv_obj_set_style_text_font(SmartWatchUI_t.bat_img, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_font(SmartWatchUI_t.battery_label, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_font(SmartWatchUI_t.connection_status, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_font(SmartWatchUI_t.bluetooth_status, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_set_style_text_font(SmartWatchUI_t.lora_status, &lv_font_montserrat_28, LV_PART_MAIN);

  SmartWatchUI_t.icons[0] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[1] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[2] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[3] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[4] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[5] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[6] = lv_imagebutton_create(SmartWatchUI_t.main_screen);
  SmartWatchUI_t.icons[7] = lv_imagebutton_create(SmartWatchUI_t.main_screen);

  lv_image_set_src(SmartWatchUI_t.main_screen, &nature_wallpaper);
#ifdef USE_TDECK_OS_ICONS
  lv_image_set_scale(SmartWatchUI_t.main_screen, 500);
#endif
  lv_imagebutton_set_src(SmartWatchUI_t.icons[0], LV_IMAGEBUTTON_STATE_RELEASED, &book, &book, &book);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[1], LV_IMAGEBUTTON_STATE_RELEASED, &setting, &setting, &setting);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[2], LV_IMAGEBUTTON_STATE_RELEASED, &telephone, &telephone, &telephone);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[3], LV_IMAGEBUTTON_STATE_RELEASED, &messages, &messages, &messages);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[4], LV_IMAGEBUTTON_STATE_RELEASED, &calculator, &calculator, &calculator);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[5], LV_IMAGEBUTTON_STATE_RELEASED, &calendar, &calendar, &calendar);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[6], LV_IMAGEBUTTON_STATE_RELEASED, &weather, &weather, &weather);
  lv_imagebutton_set_src(SmartWatchUI_t.icons[7], LV_IMAGEBUTTON_STATE_RELEASED, &clock80, &clock80, &clock80);

  lv_obj_add_style(SmartWatchUI_t.icons[0], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[1], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[2], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[3], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[4], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[5], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[6], &button_click, LV_STATE_PRESSED);
  lv_obj_add_style(SmartWatchUI_t.icons[7], &button_click, LV_STATE_PRESSED);

  lv_obj_align(SmartWatchUI_t.bat_img, LV_ALIGN_RIGHT_MID, -80, 0);

  lv_obj_align(SmartWatchUI_t.connection_status, LV_ALIGN_LEFT_MID, 30, 0);
  lv_obj_align(SmartWatchUI_t.bluetooth_status, LV_ALIGN_LEFT_MID, 70, 0);
  lv_obj_align(SmartWatchUI_t.lora_status, LV_ALIGN_LEFT_MID, 100, 0);

  lv_obj_set_size(SmartWatchUI_t.main_screen, TFT_WIDTH - 10,
                  TFT_HEIGHT - 20);

  lv_obj_center(SmartWatchUI_t.main_screen);

  lv_obj_set_style_margin_top(SmartWatchUI_t.main_screen, 30, LV_PART_MAIN);
  lv_obj_set_flex_flow(SmartWatchUI_t.main_screen, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_size(SmartWatchUI_t.nav_screen, TFT_WIDTH - 50, 30);
  lv_obj_align(SmartWatchUI_t.battery_label, LV_ALIGN_RIGHT_MID, 10, 0);

  lv_obj_set_style_pad_all(SmartWatchUI_t.nav_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(SmartWatchUI_t.main_screen, 0, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.nav_screen, 10, LV_PART_MAIN);
  lv_obj_set_style_bg_color(SmartWatchUI_t.main_screen, lv_color_hex(0x98a3a2), LV_PART_MAIN);

  static lv_style_t transparent_style;
  lv_style_init(&transparent_style);
  lv_style_set_bg_opa(&transparent_style, LV_OPA_0);
  lv_style_set_border_opa(&transparent_style, LV_OPA_0);
  lv_obj_add_style(SmartWatchUI_t.nav_screen, &transparent_style, LV_PART_MAIN);

  lv_obj_align(SmartWatchUI_t.battery_label, LV_ALIGN_RIGHT_MID, -10, 0);
  lv_obj_set_style_text_color(SmartWatchUI_t.battery_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  // lv_label_set_text(SmartWatchUI_t.battery_label, LV_SYMBOL_BATTERY_FULL);

  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[0], 10, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[1], 10, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[2], 10, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[3], 10, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[4], 10, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[5], 10, LV_PART_MAIN);
  lv_obj_set_style_margin_all(SmartWatchUI_t.icons[6], 10, LV_PART_MAIN);

#ifdef USE_SMARTWATCH_OS_ICONS
  lv_obj_set_size(SmartWatchUI_t.icons[0], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[1], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[2], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[3], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[4], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[5], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[6], 80, 80);
  lv_obj_set_size(SmartWatchUI_t.icons[7], 80, 80);
#endif

#ifdef USE_TDECK_OS_ICONS
  lv_obj_set_size(SmartWatchUI_t.icons[0], 60, 60);
  lv_obj_set_size(SmartWatchUI_t.icons[1], 60, 60);
  lv_obj_set_size(SmartWatchUI_t.icons[2], 60, 60);
  lv_obj_set_size(SmartWatchUI_t.icons[3], 60, 60);
  lv_obj_set_size(SmartWatchUI_t.icons[4], 60, 60);
  lv_obj_set_size(SmartWatchUI_t.icons[5], 60, 60);
  lv_obj_set_size(SmartWatchUI_t.icons[6], 60, 60);
#endif

  lv_obj_add_event_cb(SmartWatchUI_t.icons[0], create_contact_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[1], create_setting_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[2], create_phone_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[3], create_messages_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[4], create_calculator_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[5], create_calendar_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[6], create_weather_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
  lv_obj_add_event_cb(SmartWatchUI_t.icons[7], create_clock_page, LV_EVENT_CLICKED, SmartWatchUI_t.main_screen);
}

void setupLVGL(void *pvParameters)
{
  screenWidth = SmartWatchUI_t.gfx->width();
  screenHeight = SmartWatchUI_t.gfx->height();
  lv_init();

  // lv_display_t *display = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  // bufSize = TFT_WIDTH * 50 * BYTE_PER_PIXEL;
  // /*Declare a buffer for 1/10 screen size*/
  // // static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(bufSize,MALLOC_CAP_8BIT);
  // static lv_color_t buf1[300 * 400 / 10];

  // lv_display_set_buffers(display, buf1, NULL, bufSize, LV_DISP_RENDER_MODE_PARTIAL); /*Initialize the display buffer.*/
  // lv_display_set_flush_cb(display, my_disp_flush);

  // LV_IMAGE_DECLARE(mouse_pointer);
  // lv_obj_t *mouse_cursor = lv_img_create(lv_screen_active());

  // lv_image_set_src(mouse_cursor, &mouse_pointer);
  // lv_indev_set_cursor(indev, mouse_cursor);

  bufSize = screenWidth * 50;

  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize * 2, MALLOC_CAP_8BIT);

  disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);

  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_indev_t *indev = lv_indev_create();           /*Create an input device*/
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touch pad is a pointer-like device*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
  lv_display_add_event_cb(disp, rounder_event_cb, LV_EVENT_INVALIDATE_AREA, NULL);

  drawUI();
  while (1)
  {
    screen_update();
    // USBSerial.println("Button: "+String(analogRead(10)));
    u_int8_t tickPeriod = millis() - lastTickMillis;
    lv_tick_inc(tickPeriod);
    lastTickMillis = millis();
    lv_timer_handler();
    vTaskDelay(1);
  }
}

void setup()
{

  USBSerial.begin(115200);

  delay(1000);
  // Init Display
  if (!SmartWatchUI_t.gfx->begin())
  {
    USBSerial.println("gfx->begin() failed!");
  }
  pinMode(10, INPUT);
  SmartWatchUI_t.gfx->fillScreen(RGB565_BLACK);
  SmartWatchUI_t.gfx->setCursor(100, 100);
  SmartWatchUI_t.gfx->setTextSize(3);
  SmartWatchUI_t.gfx->setTextColor(0xCCCC, 0xFFFF);
  SmartWatchUI_t.gfx->println("Setting up stuff...");
  Wire.begin(IIC_SDA, IIC_SCL);

  while (FT3168->begin() == false)
  {
    USBSerial.println("FT3168 initialization fail");
    delay(2000);
  }
  USBSerial.println("FT3168 initialization successfully");
  SmartWatchUI_t.power.setDLDO1Voltage(2200);
  FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                 FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);
  if (!rtc.begin(Wire, IIC_SDA, IIC_SCL))
  {
    USBSerial.println("Failed to find PCF8563 - check your wiring!");
    while (1)
    {
      delay(1000);
    }
  }

  uint16_t year = 2026;
  uint8_t month = 1;
  uint8_t day = 18;
  uint8_t hour = 19;
  uint8_t minute = 13;
  uint8_t second = 0;

  // rtc.setDateTime(year, month, day, hour, minute, second);
  xTaskCreatePinnedToCore(setupLVGL, "setupLVGL", 1024 * 10, NULL, 3, &lvglTaskHandler, 0);
  xTaskCreatePinnedToCore(wifiTask, "wifiTask", 1024 * 6, NULL, 2, &wifiTaskHandler, 1);
  xTaskCreatePinnedToCore(sensorsTask, "sensorsTask", 1024 * 6, NULL, 1, &sensorTaskHandler, 1);
}

void loop()
{
}