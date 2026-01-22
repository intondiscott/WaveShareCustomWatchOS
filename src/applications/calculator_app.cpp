#include <SmartWatch_Structs.h>
#include <close-windows.h>

#define TFT_WIDTH 410
#define TFT_HEIGHT 512

static float num1, num2, result = 0.0f;
static char operator_char;



static void update_calculator_label(lv_event_t *e)
{
    lv_obj_t *label = (lv_obj_t *)lv_event_get_target(e);
    std::string input = std::to_string(result).c_str();
    lv_label_set_text(label, "input.c_str()");
}

static void textarea_event_handler(lv_event_t *e)
{
    lv_obj_t *ta = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    LV_UNUSED(ta);
    LV_UNUSED(label);

    LV_LOG_USER("Enter was pressed. The current text is: %s", lv_textarea_get_text(ta));
    const char *input = lv_textarea_get_text(ta);

    result += operator_char == '+' ? num1 + (float)atof(input) : operator_char == '-' ? num1 - (float)atof(input)
                                                             : operator_char == '*'   ? num1 * (float)atof(input)
                                                             : operator_char == '/'   ? num1 / (float)atof(input)
                                                                                      : 0;

    std::string result_str = "Result: " + std::to_string(result);
    lv_label_set_text(label, result_str.c_str());
    printf("Calculator input: %s\n", input);
    lv_textarea_set_text(ta, ""); // Clear the textarea after getting input
}

static void btnm_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
    const char *txt = lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj));

    if (lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
        lv_textarea_delete_char(ta);
    else if (txt == "=")
        lv_obj_send_event(ta, LV_EVENT_READY, NULL);
    else if (txt == "AC")
    {
        lv_textarea_set_text(ta, "");
        result = 0;
        num1 = 0;
        operator_char = '\0';
    }

    else if (txt == "+" || txt == "-" || txt == "*" || txt == "/")
    {
        operator_char = txt[0];
        num1 = atof(lv_textarea_get_text(ta));

        lv_textarea_set_text(ta, "");
    }

    else
    {
        lv_textarea_add_text(ta, txt);
    }
}

static void create_calculator_page(lv_event_t *e)
{

    if (!lv_obj_is_valid(SmartWatchUI_t.calculator))
    {
        SmartWatchUI_t.calculator = lv_obj_create(lv_screen_active());
        lv_obj_set_style_bg_color(SmartWatchUI_t.calculator, lv_color_hex(0x43687a), LV_PART_MAIN);
        lv_obj_t *ta = lv_textarea_create(SmartWatchUI_t.calculator);
        lv_obj_t *label = lv_label_create(SmartWatchUI_t.calculator);
        lv_group_t *keyboard_input_group = lv_group_create();
        lv_indev_t *indev = lv_indev_create();
        lv_indev_set_group(indev, keyboard_input_group);

        

        lv_group_add_obj(keyboard_input_group, ta);
        lv_textarea_set_one_line(ta, true);
        lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 50);
        lv_obj_set_size(ta, 350, 60);
        lv_obj_set_style_margin_bottom(ta, 5, LV_PART_MAIN);
        lv_obj_set_style_text_font(ta, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, (ta, label));
        lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_34, LV_PART_MAIN);

        lv_label_set_text(label, "Result: 0");

        static const char *btnm_map[] =
            {
                "AC", "+/-", "%", "/", "\n",
                "7", "8", "9", "*", "\n",
                "4", "5", "6", "-", "\n",
                "1", "2", "3", "+", "\n",
                LV_SYMBOL_BACKSPACE, "0", ".", "=", ""};

        lv_obj_t *btnm = lv_buttonmatrix_create(SmartWatchUI_t.calculator);
        lv_obj_set_size(btnm, 350, 300);
        lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 10);
        lv_obj_set_style_text_font(btnm, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_obj_set_style_bg_color(btnm, lv_color_hex(0x00), LV_PART_MAIN);
        lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);

        lv_obj_remove_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
        lv_buttonmatrix_set_map(btnm, btnm_map);

        SmartWatchUI_t.close_btn = lv_button_create(SmartWatchUI_t.calculator);
        lv_obj_set_style_bg_color(SmartWatchUI_t.close_btn, lv_color_hex(0xfc0303), LV_PART_MAIN);
        lv_obj_t *label_close = lv_label_create(SmartWatchUI_t.close_btn);
        lv_obj_set_style_text_font(label_close, &lv_font_montserrat_28, LV_PART_MAIN);
        lv_label_set_text(label_close, LV_SYMBOL_TRASH);

        lv_obj_align(SmartWatchUI_t.close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_size(SmartWatchUI_t.calculator, TFT_WIDTH - 20, TFT_HEIGHT - 60);

        lv_obj_center(SmartWatchUI_t.calculator);
        lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(SmartWatchUI_t.close_btn, close_window_cb, LV_EVENT_CLICKED, NULL);
    }
}