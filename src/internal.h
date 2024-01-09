#pragma once

#include <stdint.h>
#include <endian.h>

struct eizo_handle;

// Assume 256 bytes for now, which seems to be the limit for this report.
#define EIZO_FF300009_MAX_SIZE 256

enum eizo_report_id : uint8_t {
    EIZO_REPORT_ID_DESCRIPTOR = 1,
    EIZO_REPORT_ID_SET        = 2,
    EIZO_REPORT_ID_GET        = 3,
    EIZO_REPORT_ID_SET_V2     = 4,
    EIZO_REPORT_ID_GET_V2     = 5,
    EIZO_REPORT_ID_COUNTER    = 6,
    EIZO_REPORT_ID_VERIFY     = 7,
    EIZO_REPORT_ID_SN_MODEL   = 8,
    EIZO_REPORT_ID_INFO       = 9,
};

enum eizo_ff300009_key : uint8_t {
    EIZO_FF300009_KEY_RESOLUTION = 0x4c,
    EIZO_FF300009_KEY_END = 0xff,
};

enum eizo_usage : uint32_t {
    EIZO_USAGE_VOLUME                       = 0x000c00e0,

    EIZO_USAGE_BRIGHTNESS                   = 0x00820010,
    EIZO_USAGE_CONTRAST                     = 0x00820012,
    EIZO_USAGE_GAIN_RED                     = 0x00820016,
    EIZO_USAGE_GAIN_GREEN                   = 0x00820018,
    EIZO_USAGE_GAIN_BLUE                    = 0x0082001a,
    EIZO_USAGE_HORIZONTAL_FREQUENCY         = 0x008200ac,
    EIZO_USAGE_VERTICAL_FREQUENCY           = 0x008200ae,
    EIZO_USAGE_SETTINGS                     = 0x008200B0,

    EIZO_USAGE_COLOR_TEMPERATURE            = 0xff000007,
    EIZO_USAGE_OSD_INDICATOR                = 0xff00000f,
    EIZO_USAGE_PROFILE                      = 0xff000015,
    EIZO_USAGE_EEP_ADDRESS                  = 0xff000030,
    EIZO_USAGE_EEP_DATA                     = 0xff000031,
    EIZO_USAGE_USAGE_TIME                   = 0xff000037,
    EIZO_USAGE_USAGE_TIME2                  = 0xff000047,
    EIZO_USAGE_RC_SELF_DIAGNOSIS            = 0xff00004e,
    EIZO_USAGE_RC_SELF_COMPENSATION_TARGET  = 0xff00004f,
    EIZO_USAGE_GAMMA                        = 0xff000066,
    EIZO_USAGE_RC_SELF_COMPENSATION_BRIGHTNESS_PARAM = 0xff000077,
    EIZO_USAGE_RC_SELF_COMPENSATION_COLOR_BALANCE = 0xff000078,
    EIZO_USAGE_MONO_MODEL_LUT_SELECT        = 0xff00007b,
    EIZO_USAGE_DESTINATION                  = 0xff00007c,
    EIZO_USAGE_6_COLORS_LIGHTNESS           = 0xff00007f,
    EIZO_USAGE_TEST_RED                     = 0xff000089,
    EIZO_USAGE_TEST_GREEN                   = 0xff00008a,
    EIZO_USAGE_TEST_BLUE                    = 0xff00008b,
    EIZO_USAGE_TEST                         = 0xff00008c,
    EIZO_USAGE_PICTURE_EXPANSION            = 0xff0000a5,
    EIZO_USAGE_SATURATION                   = 0xff0000b3,
    EIZO_USAGE_HUE                          = 0xff0000b4,
    EIZO_USAGE_POWER                        = 0xff0000b8,
    EIZO_USAGE_AUTO_ECOVIEW                 = 0xff0000b9,
    EIZO_USAGE_MODEL_STRING                 = 0xff0000c3,
    EIZO_USAGE_EMERGENCY_POWER              = 0xff0000c5,
    EIZO_USAGE_INPUT_SIGNAL_MODE            = 0xff0000c9,
    EIZO_USAGE_HORIZONTAL_RESOLUTION        = 0xff0000ca,
    EIZO_USAGE_VERTICAL_RESOLUTION          = 0xff0000cb,
    EIZO_USAGE_POWER_LED                    = 0xff0000d3,
    EIZO_USAGE_BOOT_LOGO                    = 0xff0000d4,
    EIZO_USAGE_FIRMWARE_VERSION             = 0xff0000d8,
    EIZO_USAGE_ENABLE_DC5V_OUTPUT           = 0xff0000d9,
    EIZO_USAGE_UDI                          = 0xff0000e5,
    EIZO_USAGE_FLAVOR                       = 0xff0000f2,
    EIZO_USAGE_MODE                         = 0xff0000f3,
    EIZO_USAGE_MODE_SKIP                    = 0xff0000f4,
    EIZO_USAGE_IIS_MODE                     = 0xff0000f5,
    EIZO_USAGE_COLOR_ROUTE_COUNT            = 0xff0000f6,
    EIZO_USAGE_IIS_AREA_COUNT               = 0xff0000f7,
    EIZO_USAGE_IIS_AREA                     = 0xff0000f8,
    EIZO_USAGE_MODE_OF_OUTPUT_SEGMENT       = 0xff0000fa,

    EIZO_USAGE_BRIGHT_REG_OP                = 0xff010007,
    EIZO_USAGE_ECOVIEW_SENSOR               = 0xff01000c,
    EIZO_USAGE_MONITOR_DIRECTION            = 0xff010031,
    EIZO_USAGE_TEMPERATURE_1                = 0xff01003a,
    EIZO_USAGE_SIGNAL_TYPE_SELECTION        = 0xff01003b,
    EIZO_USAGE_BUTTON                       = 0xff01003d,
    EIZO_USAGE_SPLIT_DISPLAY_MODE           = 0xff010040,
    EIZO_USAGE_OSD_KEY_LOCK                 = 0xff010044,
    EIZO_USAGE_OSD_ALL_KEY_LOCK             = 0xff010045,
    EIZO_USAGE_INPUT_PORT                   = 0xff010048,
    EIZO_USAGE_FIX_COLOR_BANDING            = 0xff010049,
    EIZO_USAGE_OVERDRIVE                    = 0xff01004a,
    EIZO_USAGE_GAMUT_INDEX                  = 0xff01004f,
    EIZO_USAGE_SYSTEM_CHROMATICITY          = 0xff010050,
    EIZO_USAGE_SYSTEM_CHROMATICITY_ROLLBACK = 0xff010051,
    EIZO_USAGE_COLOR_DISABLE                = 0xff010052,
    EIZO_USAGE_AUTO_INPUT                   = 0xff010053,
    EIZO_USAGE_POWER_SAVE                   = 0xff010054,
    EIZO_USAGE_TEMPERATURE_2                = 0xff010055,
    EIZO_USAGE_SUBPIXEL_DRIVE               = 0xff010056,
    EIZO_USAGE_AUTO_ECOVIEW_SETTINGS        = 0xff010057,
    EIZO_USAGE_EV_AVAILABLE_CUSTOM_KEY_LOCK_OFFSET_SIZE = 0xff010058,
    EIZO_USAGE_EV_AVAILABLE_CUSTOM_KEY_LOCK_DATA = 0xff010059,
    EIZO_USAGE_EV_CUSTOM_KEY_LOCK           = 0xff01005a,
    EIZO_USAGE_SUPER_RESOLUTION             = 0xff01005b,
    EIZO_USAGE_DUE_BRIGHT_IMPROVE_SETTING   = 0xff01005d,
    EIZO_USAGE_SIGNAL_INFORMATION           = 0xff010060,
    EIZO_USAGE_PIXEL_ERROR_ANALYZER         = 0xff010061,
    EIZO_USAGE_CLIPPING                     = 0xff010064,
    EIZO_USAGE_CANDELA_1                    = 0xff010065,
    EIZO_USAGE_CANDELA_PARAMETER            = 0xff010066,
    EIZO_USAGE_MAX_CANDELA                  = 0xff010067,
    EIZO_USAGE_USB_POWER_SAVE               = 0xff010068,
    EIZO_USAGE_DUE_PRIORITY                 = 0xff010069,
    EIZO_USAGE_REGISTERED_LICENSE           = 0xff01006a,
    EIZO_USAGE_RANGE_EXTENSION_SETTING      = 0xff01006b,
    EIZO_USAGE_XYZ_FORMAT                   = 0xff01006c,
    EIZO_USAGE_2ND_POWER_OFF_TIME           = 0xff01006d,
    EIZO_USAGE_POWER_SAVE_TIME              = 0xff01006e,
    EIZO_USAGE_KEY_PUSH_COUNT               = 0xff01006f,
    EIZO_USAGE_USB_POWER_DELIVERY           = 0xff010072,
    EIZO_USAGE_USB_SELECTION                = 0xff010073,
    EIZO_USAGE_KVM_SWITCH                   = 0xff010074,
    EIZO_USAGE_PIP_SHORTCUT_KEY_VISIBLE     = 0xff010075,
    EIZO_USAGE_RC_SIGNAL_SELECT_SUPPORT_MODE = 0xff010076,
    EIZO_USAGE_HYBRID_GAMMA_PIXEL           = 0xff010077,
    EIZO_USAGE_AUTO_ECOVIEW_SETTINGS_V2     = 0xff010078,
    EIZO_USAGE_RC_CORRELATION_SENSOR_SN     = 0xff01009b,
    EIZO_USAGE_RC_CORRELATION_DATA          = 0xff01009c,
    EIZO_USAGE_UNIFORMITY_ENABLE            = 0xff01009d,
    EIZO_USAGE_SAVE                         = 0xff0100a0,
    EIZO_USAGE_FACTORY_RESET                = 0xff0100a1,
    EIZO_USAGE_COPY_CALIBRATION_DATA        = 0xff0100a5,
    EIZO_USAGE_IFS_CALIBRATION_WINDOW       = 0xff0100a6,
    EIZO_USAGE_BLACK_INSERTION              = 0xff0100c5,
    EIZO_USAGE_PSEUDO_INTERLACE             = 0xff0100c6,
    EIZO_USAGE_RANGE_EXTENSION              = 0xff0100c7,
    EIZO_USAGE_SIGNAL_FORMAT                = 0xff0100ca,
    EIZO_USAGE_SIGNAL_RESOLUTION            = 0xff0100cb,
    EIZO_USAGE_BLACK_LEVEL                  = 0xff0100cd,
    EIZO_USAGE_SAFE_AREA_MARKER             = 0xff0100ce,
    EIZO_USAGE_3D_LUT_SELECTION             = 0xff0100cf,
    EIZO_USAGE_COLOR_MATRIX_32              = 0xff0100d8,
    EIZO_USAGE_ECOVIEW_OPTIMIZER_V2         = 0xff0100eb,
    EIZO_USAGE_EV_ACTIVE_WINDOW             = 0xff0100f9,
    EIZO_USAGE_EV_PICTURE_BY_PICTURE_LAYOUT = 0xff0100fa,
    EIZO_USAGE_WHOLE_WINDOW_COLOR_SETTING   = 0xff0100fb,
    EIZO_USAGE_WINDOW_HIGHLIGHT             = 0xff0100fc,
    EIZO_USAGE_MONOCHROME_CONVERSION        = 0xff0100fd,
    EIZO_USAGE_COMPATABILITY_MODE           = 0xff0100fe,
    EIZO_USAGE_OSD_ROTATION                 = 0xff0100ff,
    EIZO_USAGE_TARGET_SELECTION             = 0xff010100,
    EIZO_USAGE_LAYOUT                       = 0xff010101,
    EIZO_USAGE_OUTPUT_SEGMENT               = 0xff010103,
    EIZO_USAGE_GENERIC_REPORT               = 0xff010108,
    EIZO_USAGE_LUMINANCE_TYPE               = 0xff010109,
    EIZO_USAGE_SIGNAL_INFO_FRAME            = 0xff01010a,
    EIZO_USAGE_CANDELA_OSD_RANGE_HDR        = 0xff01010b,
    EIZO_USAGE_CANDELA_2                    = 0xff01010c,
    EIZO_USAGE_FRONT_LUT                    = 0xff01010f,
    EIZO_USAGE_DIAGONAL_FREQUENCY           = 0xff010111,
    EIZO_USAGE_FRONT_KEY_SHORTCUT           = 0xff010112,
    EIZO_USAGE_HLG_SYSTEM_GAMMA             = 0xff010114,
    EIZO_USAGE_OSD_INFORMATION              = 0xff010115,
    EIZO_USAGE_BLUE_ONLY                    = 0xff010117,
    EIZO_USAGE_BLACK_LEVEL_LIFT             = 0xff010119,
    EIZO_USAGE_LIMITED_109_SETTING          = 0xff01011a,
    EIZO_USAGE_DSHOT                        = 0xff01011b,
    EIZO_USAGE_QUICK_CHECK                  = 0xff01011d,
    EIZO_USAGE_SYNC_SIGNAL_ENABLE           = 0xff01011e,
    EIZO_USAGE_INSTANT_BRIGHTNESS_BOOSTER   = 0xff010121,

    EIZO_USAGE_DEBUG_MODE                   = 0xff020006,
    EIZO_USAGE_FRONT_LUT_ENABLED            = 0xff02000d,
    EIZO_USAGE_DUE_ENABLED                  = 0xff02000e,
    EIZO_USAGE_MAX_CANDELA_ROLLBACK         = 0xff02001f,
    EIZO_USAGE_COLOR_MATRIX_ENABLE          = 0xff02002b,
    EIZO_USAGE_BACKLIGHT_REPLACE_INFO_1     = 0xff02002e,
    EIZO_USAGE_EDID                         = 0xff020034,
    EIZO_USAGE_SERIAL_STRING                = 0xff020036,
    EIZO_USAGE_EDID_DDC_WRITE               = 0xff020037,
    EIZO_USAGE_BACKLIGHT_REPLACE_INFO_2     = 0xff02003f,
    EIZO_USAGE_AGING_MODE                   = 0xff020044,
    EIZO_USAGE_GAMMA_TC_STATUS              = 0xff020047,
    EIZO_USAGE_GAMMA_TC_PARAMETER           = 0xff020048,
    EIZO_USAGE_DUE_TC_STATUS                = 0xff020049,
    EIZO_USAGE_FACTORY_PANEL_LUMINANCE      = 0xff020055,
    EIZO_USAGE_REAR_LUT                     = 0xff020082,
    EIZO_USAGE_GAIN_DEFINITION_1            = 0xff0200dc,
    EIZO_USAGE_GAIN_DEFINITION_2            = 0xff0200dd,
    EIZO_USAGE_ADJUSTMENT_ID                = 0xff020100,

    EIZO_USAGE_SELF_QC_CALIBRATION          = 0xff030001,
    EIZO_USAGE_MEASURE_AMBIENT_LIGHT        = 0xff030002,
    EIZO_USAGE_SELF_QC_GSC                  = 0xff030003,
    EIZO_USAGE_SELF_QC_LEA                  = 0xff030004,
    EIZO_USAGE_SELF_QC_CAL_SCHEDULE         = 0xff030022,
    EIZO_USAGE_SELF_CALIBRATION_CLOCK_TIME  = 0xff030023,
    EIZO_USAGE_SELF_CORRECTION              = 0xff030025,
    EIZO_USAGE_SELF_NEXT_SCHEDULE           = 0xff030029,
    EIZO_USAGE_SELF_QC_GSC_SCHEDULE         = 0xff030030,
    EIZO_USAGE_SELF_QC_GSC_TARGET_1         = 0xff030031,
    EIZO_USAGE_SELF_QC_GSC_TARGET_2         = 0xff030032,
    EIZO_USAGE_SELF_QC_LEA_MEAS_TIMING      = 0xff030040,
    EIZO_USAGE_SELF_QC_GSC_LAST_TARGET_1    = 0xff030060,
    EIZO_USAGE_SELF_QC_GSC_RESULT_1         = 0xff030061,
    EIZO_USAGE_SELF_QC_GSC_LAST_TARGET_2    = 0xff030062,
    EIZO_USAGE_SELF_QC_GSC_RESULT_2         = 0xff030063,
    EIZO_USAGE_SELF_CALIBRATION_MUTEX       = 0xff030080,
    EIZO_USAGE_AMBIENT_LIGHT_CANCEL_ON_SELF_CALIBRATION = 0xff030083,
    EIZO_USAGE_SELF_QC_RECALIBRATION_FOR_GSC_ERROR = 0xff030086,
    EIZO_USAGE_SELF_QC_LEA_DATA             = 0xff030090,
    EIZO_USAGE_USAGE_TIME_RTC               = 0xff030092,
    EIZO_USAGE_SELF_SCHEDULE_MENU_LOCK      = 0xff0300a0,

    EIZO_USAGE_ACC_SENSOR_DATA              = 0xff100030,
    EIZO_USAGE_ECOVIEW_SENSE_TIME           = 0xff100044,
    EIZO_USAGE_ECOVIEW_SENSE_POWER_STATE    = 0xff100045,
    EIZO_USAGE_TEMPERATURE_3                = 0xff100070,
    EIZO_USAGE_TEMPERATURE_4                = 0xff100072,
    EIZO_USAGE_HAS_SENSOR                   = 0xff1000f0,

    EIZO_USAGE_WALL_LIGHT_STATUS            = 0xff230020,
    EIZO_USAGE_WALL_LIGHT_BRIGHTNESS        = 0xff230021,
    EIZO_USAGE_SELF_TARGET_PAIRING          = 0xff230027,
    EIZO_USAGE_SELF_TARGET_ENABLE           = 0xff23002a,

    EIZO_USAGE_SECONDARY_DESCRIPTOR         = 0xff300001,
    EIZO_USAGE_SET_VALUE                    = 0xff300002,
    EIZO_USAGE_GET_VALUE                    = 0xff300003,
    EIZO_USAGE_SET_VALUE_V2                 = 0xff300004,
    EIZO_USAGE_GET_VALUE_V2                 = 0xff300005,
    EIZO_USAGE_HANDLE_COUNTER               = 0xff300006,
    EIZO_USAGE_VERIFY_LAST_REQUEST          = 0xff300007,
    EIZO_USAGE_SERIAL_PRODUCT_STRING        = 0xff300008,

    // This is a list of known usages that some monitors
    // support but their value is not known.
    // EIZO_USAGE_OSD_LANGUAGE
    // EIZO_USAGE_INPUT_RANGE
    // EIZO_USAGE_INPUT_COLOR_FORMAT
    // EIZO_USAGE_BACKLIGHT
    // EIZO_USAGE_SOUND_SELECTION
    // EIZO_USAGE_ACQUIRE_MUTEX
    // EIZO_USAGE_RELEASE_MUTEX
    // EIZO_USAGE_ECOVIEW_SENSITIVITY
    // EIZO_USAGE_PICTURE_IN_PICTURE_VISIBLE
    // EIZO_USAGE_TEST_MODE
    // EIZO_USAGE_TEST_PICTURE
    // EIZO_USAGE_MAC_ADDRESS
    // EIZO_USAGE_6AXIS_RED
    // EIZO_USAGE_6AXIS_GREEN
    // EIZO_USAGE_6AXIS_BLUE
    // EIZO_USAGE_6AXIS_CYAN
    // EIZO_USAGE_6AXIS_MAGENTA
    // EIZO_USAGE_6AXIS_YELLOW
};

// These values are only tested on the ev2760
enum eizo_eep_address : uint16_t {
    EIZO_EEP_ADDRESS_BOOT_LOGO    = 0x00bb,
    EIZO_EEP_ADDRESS_OSD_LANGUAGE = 0x011f,
};

struct [[gnu::packed]] eizo_descriptor_report {
    uint8_t  report_id;
    uint16_t offset;
    uint16_t length;
    uint8_t  desc[512];
};
static_assert(sizeof(struct eizo_descriptor_report) == 517);

struct [[gnu::packed]] eizo_value_report {
    uint8_t  report_id;
    uint32_t usage;
    uint16_t counter;
    uint8_t  value[512];
};
static_assert(sizeof(struct eizo_value_report) == 519);

struct [[gnu::packed]] eizo_counter_report {
    uint8_t  report_id;
    uint16_t counter;
};
static_assert(sizeof(struct eizo_counter_report) == 3);

struct [[gnu::packed]] eizo_verify_report {
    uint8_t  report_id;
    uint32_t usage;
    uint16_t counter;
    uint8_t  result;
} ;
static_assert(sizeof(struct eizo_verify_report) == 8);

static inline uint32_t
eizo_swap_usage(uint32_t value)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    return value << 16 | value >> 16;
#elif BYTE_ORDER == BIG_ENDIAN
    return (value & 0x00ff00ff) << 8 | (value & 0xff00ff00) >> 8;
#else
#error "Unknown Endian"
#endif
}

enum eizo_result
eizo_get_secondary_descriptor(
    struct eizo_handle *handle,
    uint8_t *dst,
    int *size);

enum eizo_result
eizo_get_value(
    struct eizo_handle *handle,
    enum eizo_usage usage,
    uint8_t *value,
    size_t len);

enum eizo_result
eizo_set_value(
    struct eizo_handle *handle,
    enum eizo_usage usage,
    uint8_t *value,
    size_t len);

enum eizo_result
eizo_get_ff300009(struct eizo_handle *handle, uint8_t *info, int *size);
