#include QMK_KEYBOARD_H
#include "quantum.h"

enum custom_keycodes
{
    KC_MY_BTN1 = KEYBALL_SAFE_RANGE, // Remap上では 0x5DAF
    KC_MY_BTN2,                      // Remap上では 0x5DB0
    KC_MY_BTN3                       // Remap上では 0x5DB1
};

enum click_state
{
    NONE = 0,
    WAITING,   // マウスレイヤーが有効になるのを待つ。 Wait for mouse layer to activate.
    CLICKABLE, // マウスレイヤー有効になりクリック入力が取れる。 Mouse layer is enabled to take click input.
    CLICKING,  // クリック中。 Clicking.
};

enum click_state state; // 現在のクリック入力受付の状態 Current click input reception status
uint16_t click_timer;   // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system.

uint16_t to_reset_time = 800; // この秒数(千分の一秒)、CLICKABLE状態ならクリックレイヤーが無効になる。 For this number of seconds (milliseconds), the click layer is disabled if in CLICKABLE state.

const int16_t to_clickable_movement = 0; // クリックレイヤーが有効になるしきい値
const uint16_t click_layer = 4;          // マウス 入力が可能になった際に有効になるレイヤー。Layers enabled when mouse input is enabled

int16_t mouse_record_threshold = 30; // ポインターの動きを一時的に記録するフレーム数。 Number of frames in which the pointer movement is temporarily recorded.
int16_t mouse_move_count_ratio = 5;  // ポインターの動きを再生する際の移動フレームの係数。 The coefficient of the moving frame when replaying the pointer movement.

int16_t mouse_movement;

// クリック用のレイヤーを有効にする。 Enable layers for clicks
void enable_click_layer(void)
{
    layer_on(click_layer);
    click_timer = timer_read();
    state = CLICKABLE;
}

// クリック用のレイヤーを無効にする。 Disable layers for clicks.
void disable_click_layer(void)
{
    state = NONE;
    layer_off(click_layer);
}

// 自前の絶対数を返す関数。 Functions that return absolute numbers.
int16_t my_abs(int16_t num)
{
    if (num < 0)
    {
        num = -num;
    }

    return num;
}

// 自前の符号を返す関数。 Function to return the sign.
int16_t mmouse_move_y_sign(int16_t num)
{
    if (num < 0)
    {
        return -1;
    }

    return 1;
}

// 現在クリックが可能な状態か。 Is it currently clickable?
bool is_clickable_mode(void)
{
    return state == CLICKABLE || state == CLICKING;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{

    switch (keycode)
    {
        case KC_MY_BTN1:
        case KC_MY_BTN2:
        case KC_MY_BTN3:
        {
            report_mouse_t currentReport = pointing_device_get_report();

            // どこのビットを対象にするか。 Which bits are to be targeted?
            uint8_t btn = 1 << (keycode - KC_MY_BTN1);

            if (record->event.pressed)
            {
                // ビットORは演算子の左辺と右辺の同じ位置にあるビットを比較して、両方のビットのどちらかが「1」の場合に「1」にします。
                // Bit OR compares bits in the same position on the left and right sides of the operator and sets them to "1" if either of both bits is "1".
                currentReport.buttons |= btn;
                state = CLICKING;
            }
            else
            {
                // ビットANDは演算子の左辺と右辺の同じ位置にあるビットを比較して、両方のビットが共に「1」の場合だけ「1」にします。
                // Bit AND compares the bits in the same position on the left and right sides of the operator and sets them to "1" only if both bits are "1" together.
                currentReport.buttons &= ~btn;
                enable_click_layer();
            }

            pointing_device_set_report(currentReport);
            pointing_device_send();
            return false;
        }

        default:
            if (record->event.pressed)
            {
                disable_click_layer();
            }
    }

    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report)
{
    int16_t current_x = mouse_report.x;
    int16_t current_y = mouse_report.y;

    if (current_x != 0 || current_y != 0)
    {

        switch (state)
        {
            case CLICKABLE:
                click_timer = timer_read();
                break;

            case CLICKING:
                break;

            case WAITING:
                mouse_movement += my_abs(current_x) + my_abs(current_y);

                if (mouse_movement >= to_clickable_movement)
                {
                    mouse_movement = 0;
                    enable_click_layer();
                }
                break;

            default:
                click_timer = timer_read();
                state = WAITING;
                mouse_movement = 0;
        }
    }
    else
    {
        switch (state)
        {
            case CLICKING:
                break;

            case CLICKABLE:
                if (timer_elapsed(click_timer) > to_reset_time)
                {
                    disable_click_layer();
                }
                break;

            case WAITING:
                if (timer_elapsed(click_timer) > 50)
                {
                    mouse_movement = 0;
                    state = NONE;
                }
                break;

            default:
                mouse_movement = 0;
                state = NONE;
        }
    }

    mouse_report.x = current_x;
    mouse_report.y = current_y;

    return mouse_report;
}

////////////////////////////////////
///
/// 自動マウスレイヤーの実装 ここまで
///
////////////////////////////////////

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
// keymap for default
[0] = LAYOUT_universal(
//    .-----------------------------------------------------------------------------.                    .-----------------------------------------------------------------------------.
        KC_TAB     , KC_Q       , KC_W       , KC_E       , KC_R       , KC_T       ,                      KC_Y       , KC_U       , KC_I       , KC_O       , KC_P       , KC_NUHS    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        KC_LCTL    , KC_A       , KC_S       , KC_D       , KC_F       , KC_G       ,                      KC_H       , KC_J       , KC_K       , KC_L       , KC_SCLN    , KC_QUOT    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        KC_LSFT    , KC_Z       , KC_X       , KC_C       , KC_V       , KC_B       ,                      KC_N       , KC_M       , KC_COMM    , KC_DOT     , KC_SLSH    , KC_INT1    ,
//    |------------+------------+------------+------------+------------+------------|=======|    |=======|------------+------------+------------+------------+------------+------------|
                    KC_LALT , KC_LGUI , LCTL_T(KC_LNG2) , LT(1,KC_SPC)    , LT(3,KC_LNG1)   ,      KC_BSPC         , LT(2,KC_ENT)    , _______         , _______ , KC_ESC
//                |---------+---------+-----------------+-----------------+-----------------|    |-----------------+-----------------+-----------------+---------+---------|

),

[1] = LAYOUT_universal(
//    .-----------------------------------------------------------------------------.                    .-----------------------------------------------------------------------------.
        _______    , _______    , KC_7       , KC_8       , KC_9       , _______    ,                      KC_GRV     , S(KC_9)    , S(KC_0)    , KC_LBRC    , KC_RBRC    , _______    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        _______    , _______    , KC_4       , KC_5       , KC_6       , _______    ,                      KC_MINS    , S(KC_5)    , S(KC_6)    , S(KC_7)    , S(KC_8)    , _______    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        _______    , _______    , KC_1       , KC_2       , KC_3       , _______    ,                      KC_EQL     , S(KC_1)    , S(KC_2)    , S(KC_3)    , S(KC_4)    , _______    ,
//    |------------+------------+------------+------------+------------+------------|=======|    |=======|------------+------------+------------+------------+------------+------------|
                    KC_0    , KC_DOT  , _______         , _______         , _______         ,      KC_DEL          , _______         , _______         , _______ , _______
//                |---------+---------+-----------------+-----------------+-----------------|    |-----------------+-----------------+-----------------+---------+---------|
),

[2] = LAYOUT_universal(
//    .-----------------------------------------------------------------------------.                    .-----------------------------------------------------------------------------.
        _______    ,  _______   , KC_F7      , KC_F8      , KC_F9      , _______    ,                      _______    , _______    , _______    , _______    , _______    , _______    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        _______    ,  _______   , KC_F4      , KC_F5      , KC_F6      , _______    ,                      KC_PGUP    , KC_BTN1    , KC_UP      , KC_BTN2    , KC_BTN5    , _______    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        _______    ,  _______   , KC_F1      , KC_F2      , KC_F3      , _______    ,                      KC_PGDN    , KC_LEFT    , KC_DOWN    , KC_RGHT    , KC_BTN4    , _______    ,
//    |------------+------------+------------+------------+------------+------------|=======|    |=======|------------+------------+------------+------------+------------+------------|
                    _______ , _______ , KC_F10          , KC_F11          , KC_F12          ,      _______         , _______         , _______         , _______ , _______
//                |---------+---------+-----------------+-----------------+-----------------|    |-----------------+-----------------+-----------------+---------+---------|
),

[3] = LAYOUT_universal(
//    .-----------------------------------------------------------------------------.                    .-----------------------------------------------------------------------------.
        KC_SLEP    , KC_VOLU    , _______    , _______    , _______    , _______    ,                      RGB_M_P    , RGB_M_B    , RGB_M_R    , RGB_M_SW   , RGB_M_SN   , RGB_M_K    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        KC_WAKE    , KC_VOLD    , RGB_SAI    , RGB_VAI    , _______    , SCRL_DVI   ,                      RGB_M_X    , RGB_M_G    , RGB_M_T    , RGB_M_TW   , _______    , _______    ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        KC_PWR     , KC_MUTE    , RGB_SAD    , RGB_VAD    , _______    , SCRL_DVD   ,                      CPI_D1K    , CPI_D100   , CPI_I100   , CPI_I1K    , _______    , KBC_SAVE   ,
//    |------------+------------+------------+------------+------------+------------|=======|    |=======|------------+------------+------------+------------+------------+------------|
                    KC_MCTL , KC_LPAD , _______         , _______         , _______         ,      _______         , _______         , _______         , _______ , QK_BOOT
//                |---------+---------+-----------------+-----------------+-----------------|    |-----------------+-----------------+-----------------+---------+---------|
),

[4] = LAYOUT_universal(
//    .-----------------------------------------------------------------------------.                    .-----------------------------------------------------------------------------.
        _______    ,  KC_F1     , KC_F2      , KC_F3      , KC_F4      , KC_F5      ,                      KC_F6      , KC_F7      , KC_F8      , KC_F9      , KC_F10     , KC_F11     ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        _______    ,  _______   , _______    , KC_UP      , KC_ENT     , KC_DEL     ,                      KC_PGUP    , KC_MY_BTN1 , KC_UP      , KC_MY_BTN2 , KC_BTN3    , KC_F12     ,
//    |------------+------------+------------+------------+------------+------------|                    |------------+------------+------------+------------+------------+------------|
        _______    ,  _______   , KC_LEFT    , KC_DOWN    , KC_RGHT    , KC_BSPC    ,                      KC_PGDN    , KC_LEFT    , KC_DOWN    , KC_RGHT    , _______    , _______    ,
//    |------------+------------+------------+------------+------------+------------|=======|    |=======|------------+------------+------------+------------+------------+------------|
                    _______ , _______ , _______         , _______         , _______         ,      _______         , _______         , _______         , _______ , _______
//                |---------+---------+-----------------+-----------------+-----------------|    |-----------------+-----------------+-----------------+---------+---------|
)
};
// clang-format on

layer_state_t layer_state_set_user(layer_state_t state)
{
    // レイヤーが1または3の場合、スクロールモードが有効になる
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    // keyball_set_scroll_mode(get_highest_layer(state) == 1 || get_highest_layer(state) == 3);

    // レイヤーとLEDを連動させる
    uint8_t layer = biton32(state);
    switch (layer)
    {
        case 0:
            rgblight_sethsv(HSV_BLUE);
            break;
        case 1:
            rgblight_sethsv(HSV_GREEN);
            break;
        case 2:
            rgblight_sethsv(HSV_RED);
            break;
        case 3:
            rgblight_sethsv(HSV_YELLOW);
            break;
        case 4:
            rgblight_sethsv(HSV_ORANGE);
            break;
        default:
            rgblight_sethsv(HSV_OFF);
    }

    return state;
}

#ifdef OLED_ENABLE

#include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void)
{
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
