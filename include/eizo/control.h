#pragma once

#include <stddef.h>

#include "handle.h"

enum eizo_color_temperature : uint16_t {
    EIZO_COLOR_TEMPERATURE_OFF    = 0,
    EIZO_COLOR_TEMPERATURE_4000K  = 1,
    EIZO_COLOR_TEMPERATURE_4500K  = 2,
    EIZO_COLOR_TEMPERATURE_5000K  = 3,
    EIZO_COLOR_TEMPERATURE_5500K  = 4,
    EIZO_COLOR_TEMPERATURE_6000K  = 5,
    EIZO_COLOR_TEMPERATURE_6500K  = 6,
    EIZO_COLOR_TEMPERATURE_7000K  = 7,
    EIZO_COLOR_TEMPERATURE_7500K  = 8,
    EIZO_COLOR_TEMPERATURE_8000K  = 9,
    EIZO_COLOR_TEMPERATURE_8500K  = 10,
    EIZO_COLOR_TEMPERATURE_9000K  = 11,
    EIZO_COLOR_TEMPERATURE_9300K  = 12,
    EIZO_COLOR_TEMPERATURE_9500K  = 13,
    EIZO_COLOR_TEMPERATURE_10000K = 14,
    EIZO_COLOR_TEMPERATURE_10500K = 15,
    EIZO_COLOR_TEMPERATURE_11000K = 16,
    EIZO_COLOR_TEMPERATURE_11500K = 17,
    EIZO_COLOR_TEMPERATURE_12000K = 18,
    EIZO_COLOR_TEMPERATURE_12500K = 19,
    EIZO_COLOR_TEMPERATURE_13000K = 20,
    EIZO_COLOR_TEMPERATURE_13500K = 21,
    EIZO_COLOR_TEMPERATURE_14000K = 22,
    EIZO_COLOR_TEMPERATURE_14500K = 23,
    EIZO_COLOR_TEMPERATURE_15000K = 24,
};

enum eizo_gamma : uint16_t {
    EIZO_GAMMA_1_4        = 0x00,
    EIZO_GAMMA_1_6        = 0x01,
    EIZO_GAMMA_1_8        = 0x02,
    EIZO_GAMMA_2_0        = 0x03,
    EIZO_GAMMA_2_2        = 0x04,
    EIZO_GAMMA_2_4        = 0x05,
    EIZO_GAMMA_2_6        = 0x06,
    EIZO_GAMMA_2_8        = 0x07,
    EIZO_GAMMA_3_0        = 0x08,
    EIZO_GAMMA_FPS_HIGH   = 0xf4,
    EIZO_GAMMA_FPS_MEDIUM = 0xf5,
    EIZO_GAMMA_FPS_LOW    = 0xf6,
    EIZO_GAMMA_FPS_RTS    = 0xf7,
    EIZO_GAMMA_FPS_POWER  = 0xfc,
};

enum eizo_input_signal_mode : uint16_t {
    EIZO_INPUT_SIGNAL_MODE_ANALOGUE = 0,
    EIZO_INPUT_SIGNAL_MODE_DIGITAL = 1,
    EIZO_INPUT_SIGNAL_MODE_UNKNOWN = 2,
};

enum eizo_auto_input : bool {
    EIZO_AUTO_INPUT_DISABLED = false,
    EIZO_AUTO_INPUT_ENABLED = true,
};

enum eizo_input_port : uint16_t {
    EIZO_INPUT_PORT_UNDEFINED  = 0x0000,
    EIZO_INPUT_PORT_DSUB1      = 0x0100,
    EIZO_INPUT_PORT_DSUB2      = 0x0101,
    EIZO_INPUT_PORT_DVI1       = 0x0200,
    EIZO_INPUT_PORT_DVI2       = 0x0201,
    EIZO_INPUT_PORT_DP1        = 0x0300,
    EIZO_INPUT_PORT_DP2        = 0x0301,
    EIZO_INPUT_PORT_HDMI1      = 0x0400,
    EIZO_INPUT_PORT_HDMI2      = 0x0401,
    EIZO_INPUT_PORT_SDI1       = 0x0500,
    EIZO_INPUT_PORT_SDI2       = 0x0501,
    EZIO_INPUT_PORT_SDIAB      = 0x0502,
    EZIO_INPUT_PORT_SDIBA      = 0x0503,
    EIZO_INPUT_PORT_D1         = 0x0600,
    EIZO_INPUT_PORT_D2         = 0x0601,
    EIZO_INPUT_PORT_COMPOSITE1 = 0x0700,
    EIZO_INPUT_PORT_COMPOSITE2 = 0x0701,
    EIZO_INPUT_PORT_TV1        = 0x0800,
    EIZO_INPUT_PORT_TV2        = 0x0801,
    EIZO_INPUT_PORT_USBC       = EIZO_INPUT_PORT_DP2,
};

enum eizo_debug_mode : bool {
    EIZO_DEBUG_MODE_DISABLED = false,
    EIZO_DEBUG_MODE_ENABLED = true,
};

enum eizo_aging_mode : bool {
    EIZO_AGING_MODE_DISABLED = false,
    EIZO_AGING_MODE_ENABLED = true,
};

enum eizo_usb_power_save : bool {
    EIZO_USB_POWER_SAVE_DISABLED = false,
    EIZO_USB_POWER_SAVE_ENABLED = true,
};

enum eizo_usb_power_delivery : bool {
    EIZO_USB_POWER_DELIVERY_NORMAL = false,
    EIZO_USB_POWER_DELIVERY_HIGH = true,
};

enum eizo_power_led : bool {
    EIZO_POWER_LED_DISABLED = false,
    EIZO_POWER_LED_ENABLED = true,
};

enum eizo_profile : uint16_t {
    EIZO_PROFILE_TEXT             = 0,
    EIZO_PROFILE_PICTURE          = 1,
    EIZO_PROFILE_MOVIE            = 2,
    EIZO_PROFILE_CUSTOM           = 3,
    EIZO_PROFILE_SRGB             = 4,
    EIZO_PROFILE_EXT              = 5,
    EIZO_PROFILE_VIEWER           = 6,
    EIZO_PROFILE_CAL              = 7,
    EIZO_PROFILE_DICOM_CB         = 8,
    EIZO_PROFILE_DICOM_BB         = 9,
    EIZO_PROFILE_EMU              = 10,
    EIZO_PROFILE_CAL1             = 11,
    EIZO_PROFILE_CAL2             = 12,
    EIZO_PROFILE_CAL3             = 13,
    EIZO_PROFILE_CAD              = 14,
    EIZO_PROFILE_CIE              = 15,
    EIZO_PROFILE_FL               = 16,
    EIZO_PROFILE_GAME             = 17,
    EIZO_PROFILE_REC709           = 18,
    EIZO_PROFILE_EBU              = 19,
    EIZO_PROFILE_SMPTE_C          = 20,
    EIZO_PROFILE_DCI              = 21,
    EIZO_PROFILE_USER1            = 22,
    EIZO_PROFILE_USER2            = 23,
    EIZO_PROFILE_USER3            = 24,
    EIZO_PROFILE_RESERVED1        = 25,
    EIZO_PROFILE_RESERVED2        = 26,
    EIZO_PROFILE_RESERVED3        = 27,
    EIZO_PROFILE_CG_CAL1          = 28,
    EIZO_PROFILE_CG_CAL2          = 29,
    EIZO_PROFILE_CG_CAL3          = 30,
    EIZO_PROFILE_EYE_CARE         = 31,
    EIZO_PROFILE_ADOBE_RGB        = 32,
    EIZO_PROFILE_CINEMA           = 33,
    EIZO_PROFILE_PAPER            = 34,
    EIZO_PROFILE_HYBRID_GAMMA     = 35,
    EIZO_PROFILE_ECO              = 36,
    EIZO_PROFILE_ALT              = 37,
    EIZO_PROFILE_DAY              = 38,
    EIZO_PROFILE_NIGHT            = 39,
    EIZO_PROFILE_FPS1             = 40,
    EIZO_PROFILE_FPS2             = 41,
    EIZO_PROFILE_RTS              = 42,
    EIZO_PROFILE_WEB              = 43,
    EIZO_PROFILE_GAME_DARK_SCENE  = 44,
    EIZO_PROFILE_GAME_LIGHT_SCENE = 45,
    EIZO_PROFILE_WEB_SRGB         = 46,
    EIZO_PROFILE_USER             = 47,
    EIZO_PROFILE_REC2020          = 48,
};

enum eizo_profile_flags : uint16_t {
    EIZO_PROFILE_FLAGS_BRIGHTNESS          = 0x0001,
    EIZO_PROFILE_FLAGS_CONTRAST            = 0x0002,
    EIZO_PROFILE_FLAGS_COLOR_TEMPERATURE   = 0x0004,
    EIZO_PROFILE_FLAGS_GAMMA               = 0x0008,
    EIZO_PROFILE_FLAGS_SATURATION          = 0x0010,
    EIZO_PROFILE_FLAGS_HUE                 = 0x0020,
    EIZO_PROFILE_FLAGS_GAIN                = 0x0040,
    EIZO_PROFILE_FLAGS_EX_CONTRAST         = 0x0080,
    EIZO_PROFILE_FLAGS_SHARPNESS           = 0x0100,
    EIZO_PROFILE_FLAGS_BLACK_LEVEL         = 0x0200,
    EIZO_PROFILE_FLAGS_6_COLORS_ADJUSTMENT = 0x0400,
    EIZO_PROFILE_FLAGS_RESET               = 0x0800,
};

// Unknown values
enum eizo_input_color_format : uint16_t {
    EIZO_INPUT_COLOR_FORMAT_AUTO,
    EIZO_INPUT_COLOR_FORMAT_YUV_422,
    EIZO_INPUT_COLOR_FORMAT_YUV_444,
    EIZO_INPUT_COLOR_FORMAT_YUV,
    EIZO_INPUT_COLOR_FORMAT_RGB,
    EIZO_INPUT_COLOR_FORMAT_YCC422_10BIT,
    EIZO_INPUT_COLOR_FORMAT_YCC422_12BIT,
    EIZO_INPUT_COLOR_FORMAT_YCC444_10BIT,
    EIZO_INPUT_COLOR_FORMAT_YCC444_12BIT,
    EIZO_INPUT_COLOR_FORMAT_RGB444_10BIT,
    EIZO_INPUT_COLOR_FORMAT_RGB444_12BIT,
};

enum eizo_input_range : uint16_t {
    EIZO_INPUT_RANGE_AUTO = 0,
    EIZO_INPUT_RANGE_FULL = 1,
    EIZO_INPUT_RANGE_LIMITED = 2,
};

enum eizo_display_port_version : uint16_t {
    EIZO_DISPLAY_PORT_VERSION_1_1 = 0,
    EIZO_DISPLAY_PORT_VERSION_1_2 = 1,
};

enum eizo_power : bool {
    EIZO_POWER_OFF = false,
    EIZO_POWER_ON = true,
};

enum eizo_window_highlight : bool {
    EIZO_WINDOW_HIGHLIGHT_DISABLED = false,
    EIZO_WINDOW_HIGHLIGHT_ENABLED = true,
};

enum eizo_window : uint16_t {
    EIZO_WINDOW_1 = 0,
    EIZO_WINDOW_2 = 1,
    EIZO_WINDOW_3 = 2,
    EIZO_WINDOW_4 = 3,

    EIZO_WINDOW_PBP_2_LEFT  = EIZO_WINDOW_1,
    EIZO_WINDOW_PBP_2_RIGHT = EIZO_WINDOW_2,
};

enum eizo_compatibility_mode : bool {
    EIZO_COMPATIBILITY_MODE_DISABLED = false,
    EIZO_COMPATIBILITY_MODE_ENABLED = true,
};

enum eizo_super_resolution : uint16_t {
    EIZO_SUPER_RESOLUTION_DISABLED = 0,
    EIZO_SUPER_RESOLUTION_STANDARD = 1,
    EIZO_SUPER_RESOLUTION_ENHANCED = 2,
};

enum eizo_contrast_enhancer : uint16_t {
    EIZO_CONTRAST_ENHANCER_DISABLED = 0,
    EIZO_CONTRAST_ENHANCER_STANDARD = 1,
    EIZO_CONTRAST_ENHANCER_ENHANCED = 2,
};

enum eizo_auto_ecoview : bool {
    EIZO_AUTO_ECOVIEW_DISABLED = false,
    EIZO_AUTO_ECOVIEW_ENABLED = true,
};

enum eizo_ecoview_ambient_light : uint16_t {
    EIZO_ECOVIEW_AMBIENT_LIGHT_DARK = 0,
    EIZO_ECOVIEW_AMBIENT_LIGHT_STANDARD = 1,
    EIZO_ECOVIEW_AMBIENT_LIGHT_BRIGHT = 2,
};

enum eizo_ecoview_optimizer_v2 : bool {
    EIZO_ECOVIEW_OPTIMIZER_V2_DISABLED = false,
    EIZO_ECOVIEW_OPTIMIZER_V2_ENABLED = true,
};

enum eizo_ecoview_sense_time : uint16_t {
    EIZO_ECOVIEW_SENSE_TIME_5_SEC  = 0,
    EIZO_ECOVIEW_SENSE_TIME_30_SEC = 1,
    EIZO_ECOVIEW_SENSE_TIME_1_MIN  = 2,
    EIZO_ECOVIEW_SENSE_TIME_3_MIN  = 3,
    EIZO_ECOVIEW_SENSE_TIME_5_MIN  = 4,
    EIZO_ECOVIEW_SENSE_TIME_10_MIN = 5,
    EIZO_ECOVIEW_SENSE_TIME_15_MIN = 6,
    EIZO_ECOVIEW_SENSE_TIME_30_MIN = 7,
    EIZO_ECOVIEW_SENSE_TIME_45_MIN = 8,
    EIZO_ECOVIEW_SENSE_TIME_60_MIN = 9,
};

enum eizo_ecoview_sense_power_state : uint16_t {
    EIZO_ECOVIEW_SENSE_POWER_STATE_RESERVED = 0,
    EIZO_ECOVIEW_SENSE_POWER_STATE_NO_SIGNAL = 1,
    EIZO_ECOVIEW_SENSE_POWER_STATE_AB_SENSE = 2,
    EIZO_ECOVIEW_SENSE_POWER_STATE_DC_OFF = 3,
    EIZO_ECOVIEW_SENSE_POWER_STATE_EXPECT = 255,
};

enum eizo_picture_expansion : uint16_t {
    EIZO_PICTURE_EXPANSION_DOT_BY_DOT = 0,
    EIZO_PICTURE_EXPANSION_ASPECT_RATIO = 1,
    EIZO_PICTURE_EXPANSION_FULLSCREEN = 2,
};

enum eizo_overdrive : uint16_t {
    EIZO_OVERDRIVE_DISABLED = 0,
    EIZO_OVERDRIVE_STANDARD = 1,
    EIZO_OVERDRIVE_ENHANCED = 2,
};

enum eizo_power_save : bool {
    EIZO_POWER_SAVE_DISABLED = false,
    EIZO_POWER_SAVE_ENABLED = true,
};

enum eizo_osd_indicator : uint16_t {
    EIZO_OSD_INDICATOR_SHOW = 0x4000,
    EIZO_OSD_INDICATOR_HIDE = 0x8000,
};

enum eizo_osd_rotation : uint16_t {
    EIZO_OSD_ROTATION_0 = 0,
    EIZO_OSD_ROTATION_90 = 1,
    EIZO_OSD_ROTATION_270 = 2,
};

enum eizo_osd_key_lock : uint16_t {
    EIZO_OSD_KEY_LOCK_NONE = 0,
    EIZO_OSD_KEY_LOCK_MENU = 1,
    EIZO_OSD_KEY_LOCK_ALL = 2,
};

enum eizo_osd_all_key_lock : bool {
    EIZO_OSD_ALL_KEY_LOCK_DISABLED = false,
    EIZO_OSD_ALL_KEY_LOCK_ENABLED = true,
};

enum eizo_osd_language : uint16_t {
    EIZO_OSD_LANGUAGE_ENGLISH = 0,
    EIZO_OSD_LANGUAGE_GERMAN = 1,
    EIZO_OSD_LANGUAGE_FRENCH = 2,
    EIZO_OSD_LANGUAGE_SPANISH = 3,
    EIZO_OSD_LANGUAGE_ITALIAN = 4,
    EIZO_OSD_LANGUAGE_SWEDISH = 5,
    EIZO_OSD_LANGUAGE_JAPANESE = 6,
    EIZO_OSD_LANGUAGE_CHINESE_SIMPLIFIED = 7,
    EIZO_OSD_LANGUAGE_CHINESE_TRADITIONAL = 8,
};

enum eizo_destination : uint16_t {
    EIZO_DESTINATION_JAPAN = 0,
    EIZO_DESTINATION_INTERNATIONAL = 1,
};

enum eizo_split_display_mode : uint16_t {
    EIZO_SPLIT_DISPLAY_MODE_SINGLE = 0,
    EIZO_SPLIT_DISPLAY_MODE_PICTURE_BY_PICTURE = 1,
    EIZO_SPLIT_DISPLAY_MODE_PICTURE_IN_PICTURE = 2,
};

enum eizo_picture_by_picture_layout : uint16_t {
    EIZO_PICTURE_BY_PICTURE_LAYOUT_1 = 0,
    EIZO_PICTURE_BY_PICTURE_LAYOUT_2 = 1,
    EIZO_PICTURE_BY_PICTURE_LAYOUT_3 = 2,
    EIZO_PICTURE_BY_PICTURE_LAYOUT_4 = 3,
    EIZO_PICTURE_BY_PICTURE_LAYOUT_5 = 4,
    EIZO_PICTURE_BY_PICTURE_LAYOUT_6 = 5,
};

enum eizo_button : uint8_t {
    EIZO_BUTTON_1 = 0b00000001,
    EIZO_BUTTON_2 = 0b00000010,
    EIZO_BUTTON_3 = 0b00000100,
    EIZO_BUTTON_4 = 0b00001000,
    EIZO_BUTTON_5 = 0b00010000,
    EIZO_BUTTON_6 = 0b00100000,
    EIZO_BUTTON_7 = 0b01000000,
    EIZO_BUTTON_8 = 0b10000000,
};

enum eizo_boot_logo : bool {
    EIZO_BOOT_LOGO_DISABLED = false,
    EIZO_BOOT_LOGO_ENABLED = true,
};

enum eizo_s2431w_signal : uint16_t {
    EIZO_S2431W_SIGNAL_VIDEO = 0,
    EIZO_S2431W_SIGNAL_COLOR_FIELD = 1,
    EIZO_S2431W_SIGNAL_1PX_CHECKERBOARD = 4,
    EIZO_S2431W_SIGNAL_2PX_CHECKERBOARD = 5,
    EIZO_S2431W_SIGNAL_8PX_CHECKERBOARD = 6,
    EIZO_S2431W_SIGNAL_32PX_CHECKERBOARD = 7,
    EIZO_S2431W_SIGNAL_HORIZONTAL_STRIPE = 8,
    EIZO_S2431W_SIGNAL_VERTICAL_STRIPE = 9,
    EIZO_S2431W_SIGNAL_BLUE_2PX_STEP_RAMP = 10,
    EIZO_S2431W_SIGNAL_BLUE_8PX_STEP_RAMP = 11,
    EIZO_S2431W_SIGNAL_GREEN_2PX_STEP_RAMP = 12,
    EIZO_S2431W_SIGNAL_GREEN_8PX_STEP_RAMP = 13,
    EIZO_S2431W_SIGNAL_RED_8PX_STEP_RAMP = 14,
    EIZO_S2431W_SIGNAL_RED_2PX_STEP_RAMP = 15,
    EIZO_S2431W_SIGNAL_GREY_2PX_STEP_RAMP = 16,
    EIZO_S2431W_SIGNAL_GREY_8PX_STEP_RAMP = 17,
    EIZO_S2431W_SIGNAL_RGB_16PX_LINE_STRIPE_RAMP = 18,
    EIZO_S2431W_SIGNAL_COARSE_GRID = 19,
    EIZO_S2431W_SIGNAL_FINE_GRID = 20,
    EIZO_S2431W_SIGNAL_RGB_SUBPIXEL = 22,
    EIZO_S2431W_SIGNAL_VERTICAL_RAMP = 31,
};

enum eizo_test_picture : uint16_t {
    EIZO_TEST_PICTURE_WHITE_FULL_SCREEN = 2,
    EIZO_TEST_PICTURE_WHITE_BLACK_BORDER = 6,
    EIZO_TEST_PICTURE_RED_GRADIENT = 9,
    EIZO_TEST_PICTURE_GREEN_GRADIENT = 10,
    EIZO_TEST_PICTURE_BLUE_GRADIENT = 11,
    EIZO_TEST_PICTURE_WHITE_GRADIENT = 12,
    EIZO_TEST_PICTURE_BLACK_SQUARE = 19,
};

enum eizo_vsync_mode : uint16_t {
    EIZO_VSYNC_MODE_DISABLED = 0,
    EIZO_VSYNC_MODE_VERTICAL = 32,
    EIZO_VSYNC_MODE_HORIZONTAL = 96,
};

struct eizo_factory_panel_luminance {
    uint16_t white;
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct eizo_auto_ecoview_settings {
    uint16_t                        max_point_brightness;
    enum eizo_ecoview_ambient_light max_point_ambient_light;
    uint16_t                        min_point_brightness;
    enum eizo_ecoview_ambient_light min_point_ambient_light;
    uint16_t                        inflection_point_brightness;
    uint16_t                        inflection_point_ambient_light;
    uint16_t                        adjust_margin_brightness;
    bool                            is_adjustable_brightness;
};

struct eizo_auto_ecoview_settings_v2 {
    uint8_t  bright_point_brightness;
    uint16_t bright_point_ambient_light;
    uint8_t  dark_point_brightness;
    uint16_t dark_point_ambient_light;
    uint16_t inflection_point_ambient_light;
};

struct eizo_6axis_colors {
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t cyan;
    uint32_t magenta;
    uint32_t yellow;
};

enum eizo_result
eizo_get_brightness(eizo_handle_t handle, uint16_t *value);

enum eizo_result
eizo_set_brightness(eizo_handle_t handle, uint16_t value);

enum eizo_result
eizo_get_contrast(eizo_handle_t handle, uint16_t *value);

enum eizo_result
eizo_set_contrast(eizo_handle_t handle, uint16_t value);

enum eizo_result
eizo_get_usage_time(eizo_handle_t handle, long *time);

enum eizo_result
eizo_set_usage_time(eizo_handle_t handle, long time);

enum eizo_result
eizo_get_available_custom_key_lock(
    eizo_handle_t handle,
    uint8_t **ptr,
    size_t *len);
