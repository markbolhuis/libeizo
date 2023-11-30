#pragma once

#include <stdint.h>
#include <endian.h>

struct eizo_handle;

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

struct __attribute__((packed)) eizo_descriptor_report {
    uint8_t  report_id;
    uint16_t offset;
    uint16_t length;
    uint8_t  desc[512];
};
static_assert(sizeof(struct eizo_descriptor_report) == 517);

struct __attribute__((packed)) eizo_value_report {
    uint8_t  report_id;
    uint32_t usage;
    uint16_t counter;
    uint8_t  value[512];
};
static_assert(sizeof(struct eizo_value_report) == 519);

struct __attribute__((packed)) eizo_counter_report {
    uint8_t  report_id;
    uint16_t counter;
};
static_assert(sizeof(struct eizo_counter_report) == 3);

struct __attribute__((packed)) eizo_verify_report {
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

ssize_t
eizo_get_descriptor(struct eizo_handle *handle, uint8_t *dst);

int
eizo_get_ff300009(struct eizo_handle *handle, uint8_t *info);
