#pragma once

#include <stdint.h>

typedef struct eizo_handle *eizo_handle_t;

enum eizo_result : int {
    EIZO_SUCCESS = 0,
    EIZO_INCOMPLETE = 1,
    EIZO_ERROR_UNKNOWN = -1,
    EIZO_ERROR_IO = -2,
    EIZO_ERROR_INVALID_ARGUMENT = -3,
    EIZO_ERROR_NO_MEMORY = -4,
    EIZO_ERROR_RACE_CONDITION = -5,
    EIZO_ERROR_INVALID_USAGE = -6,
    EIZO_ERROR_NOT_PERMITTED = -7,
    EIZO_ERROR_BAD_DATA = -8,
    EIZO_ERROR_OUT_OF_RANGE = -9,
};

constexpr uint16_t EIZO_VID = 0x056d;

enum eizo_pid : uint16_t {
    EIZO_PID_FLEXSCAN_EV3237 = 0x4000,
    EIZO_PID_FLEXSCAN_EV2450 = 0x4001,
    EIZO_PID_FLEXSCAN_EV2455 = 0x4002,
    // EIZO_PID_FORIS_FS2434     = 0x4003,
    // EIZO_PID_COLOREDGE_CS2420 = 0x4008,
    // EIZO_PID_COLOREDGE_CS240  = 0x400b,
    // EIZO_PID_COLOREDGE_CS270  = 0x4012,
    EIZO_PID_FLEXSCAN_EV2750 = 0x4014,
    // EIZO_PID_COLOREDGE_CG2730 = 0x4023,
    // EIZO_PID_COLOREDGE_CS2730 = 0x4024,
    EIZO_PID_FLEXSCAN_EV2451 = 0x4026,
    EIZO_PID_FLEXSCAN_EV2456 = 0x4027,
    EIZO_PID_FLEXSCAN_EV2780 = 0x402b,
    EIZO_PID_FLEXSCAN_EV2785 = 0x4036,
    EIZO_PID_FLEXSCAN_EV3285 = 0x4037,
    // EIZO_PID_COLOREDGE_CG279X = 0x403e,
    EIZO_PID_FLEXSCAN_EV2457 = 0x4044,
    // EIZO_PID_COLOREDGE_CS2731 = 0x4048,
    // EIZO_PID_COLOREDGE_CS2740 = 0x404d,
    // EIZO_PID_COLOREDGE_CS2410 = 0x4058,
    EIZO_PID_FLEXSCAN_EV2760 = 0x4059,
    EIZO_PID_FLEXSCAN_EV2360 = 0x405a,
    EIZO_PID_FLEXSCAN_EV2460 = 0x405b,
    EIZO_PID_FLEXSCAN_EV2495 = 0x405e,
    EIZO_PID_FLEXSCAN_EV2795 = 0x405f,
    EIZO_PID_FLEXSCAN_EV3895 = 0x4065,
    EIZO_PID_FLEXSCAN_EV2480 = 0x406a,
    EIZO_PID_FLEXSCAN_EV2485 = 0x4073,
    // EIZO_PID_COLOREDGE_CG2700S = 0x4079;
    EIZO_PID_FLEXSCAN_EV3240X = 0x4081,

    // Unknown pids
    // EIZO_PID_FLEXSCAN_EV2781
    // EIZO_PID_FLEXSCAN_EV2490
    // EIZO_PID_FLEXSCAN_EV2740X

    // EIZO_PID_COLOREDGE_CG2420,
    // EIZO_PID_COLOREDGE_CG247,
    // EIZO_PID_COLOREDGE_CG247X,
    // EIZO_PID_COLOREDGE_CG248_4K,
    // EIZO_PID_COLOREDGE_CG2700S,
    // EIZO_PID_COLOREDGE_CG2700X,
    // EIZO_PID_COLOREDGE_CG277,
    // EIZO_PID_COLOREDGE_CG3145,
    // EIZO_PID_COLOREDGE_CG3146,
    // EIZO_PID_COLOREDGE_CG318_4K,
    // EIZO_PID_COLOREDGE_CG319X,

    // EIZO_PID_COLOREDGE_CS230,
    // EIZO_PID_COLOREDGE_CS2400R,
    // EIZO_PID_COLOREDGE_CS2400S,

    // EIZO_PID_COLOREDGE_CX241,
    // EIZO_PID_COLOREDGE_CX271
};

struct eizo_info {
    char devnode[16];
    enum eizo_pid pid;
};

enum eizo_result
eizo_enumerate(struct eizo_info *info, size_t *len);

enum eizo_result
eizo_open_hidraw(const char *path, eizo_handle_t *handle);

void
eizo_close(eizo_handle_t handle);

enum eizo_pid
eizo_get_pid(eizo_handle_t handle);

int
eizo_get_fd(eizo_handle_t handle);

unsigned long
eizo_get_serial(eizo_handle_t handle);

const char *
eizo_get_product(eizo_handle_t handle);
