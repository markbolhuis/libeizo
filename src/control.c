#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/param.h>

#include "eizo/handle.h"
#include "eizo/control.h"
#include "internal.h"

static enum eizo_result
eizo_get_uint16(struct eizo_handle *handle, enum eizo_usage usage, uint16_t *value)
{
    union {
        uint16_t value;
        uint8_t buf[2];
    } u;
    enum eizo_result res = eizo_get_value(handle, usage, u.buf, 2);
    if (res >= EIZO_SUCCESS) {
        *value = le16toh(u.value);
    }
    return res;
}

static enum eizo_result
eizo_set_uint16(struct eizo_handle *handle, enum eizo_usage usage, uint16_t value)
{
    union {
        uint16_t value;
        uint8_t buf[2];
    } u;
    u.value = htole16(value);
    return eizo_set_value(handle, usage, u.buf, 2);
}

enum eizo_result
eizo_get_brightness(struct eizo_handle *handle, int *value)
{
    uint16_t val = 0;
    enum eizo_result res = eizo_get_uint16(handle, EIZO_USAGE_BRIGHTNESS, &val);
    if (res >= EIZO_SUCCESS) {
        *value = (int)val;
    }
    return res;
}

enum eizo_result
eizo_set_brightness(struct eizo_handle *handle, int value)
{
    if (value < 0 || value > 200) {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }
    return eizo_set_uint16(handle, EIZO_USAGE_BRIGHTNESS, (uint16_t)value);
}

enum eizo_result
eizo_get_contrast(struct eizo_handle *handle, int *value)
{
    uint16_t val = 0;
    enum eizo_result res = eizo_get_uint16(handle, EIZO_USAGE_CONTRAST, &val);
    if (res >= EIZO_SUCCESS) {
        *value = (int)val;
    }
    return res;
}

enum eizo_result
eizo_set_contrast(struct eizo_handle *handle, int value)
{
    if (value < 0 || value > 140) {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }
    return eizo_set_uint16(handle, EIZO_USAGE_CONTRAST, (uint16_t)value);
}

enum eizo_result
eizo_get_usage_time(struct eizo_handle *handle, long *time)
{
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    enum eizo_result res = eizo_get_value(
        handle,
        EIZO_USAGE_USAGE_TIME,
        u.buf, 3);
    if (res >= EIZO_SUCCESS) {
        long t = (long)le16toh(u.hour);
        t *= 60;
        t += (long)u.buf[2];
        *time = t;
    }
    return res;
}

enum eizo_result
eizo_set_usage_time(struct eizo_handle *handle, long time)
{
    if (time < 0) {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    u.buf[2] = time % 60;
    time /= 60;
    if (time > UINT16_MAX) {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }
    u.hour = htole16(time);
    return eizo_set_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
}

enum eizo_result
eizo_get_available_custom_key_lock(
    struct eizo_handle *handle,
    uint8_t **ptr,
    size_t *len)
{
    union {
        struct {
            uint16_t offset;
            uint16_t size;
        };
        uint8_t buf[64];
    } u;

    enum eizo_result res = eizo_get_value(
        handle,
        EIZO_USAGE_EV_AVAILABLE_CUSTOM_KEY_LOCK_OFFSET_SIZE,
        u.buf, 4);
    if (res < EIZO_SUCCESS) {
        fprintf(stderr, "%s: Failed to get offset and size\n", __func__);
        return res;
    }

    size_t size = le16toh(u.size);
    if (size == 0) {
        *len = 0;
        *ptr = nullptr;
        return EIZO_SUCCESS;
    }

    size_t offset = le16toh(u.offset);
    if (offset != 0) {
        memset(u.buf, 0, 4);
        res = eizo_set_value(
            handle,
            EIZO_USAGE_EV_AVAILABLE_CUSTOM_KEY_LOCK_OFFSET_SIZE,
            u.buf, 4);
        if (res < EIZO_SUCCESS) {
            fprintf(stderr, "%s: Failed to reset offset.\n", __func__);
            return res;
        }
    }

    uint8_t *data = malloc(size);
    if (!data) {
        fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        return EIZO_ERROR_NO_MEMORY;
    }

    for (size_t i = 0; i < size; i += 62) {
        res = eizo_get_value(
            handle,
            EIZO_USAGE_EV_AVAILABLE_CUSTOM_KEY_LOCK_DATA,
            u.buf, 64);
        if (res < EIZO_SUCCESS) {
            fprintf(stderr, "%s: Failed to get data at %ld.\n", __func__, i);
            goto end;
        }

        offset = le16toh(u.offset);
        if (offset != i) {
            fprintf(stderr, "%s: Offset %ld != %ld.\n", __func__, offset, i);
            res = EIZO_ERROR_BAD_DATA;
            goto end;
        }

        size_t cpy = MIN(size - i, 62);
        memcpy(data + i, u.buf + 2, cpy);
    }

    *ptr = data;
    *len = size;
    return EIZO_SUCCESS;
end:
    free(data);
    return res;
}

enum eizo_result
eizo_set_debug_mode(struct eizo_handle *handle, enum eizo_debug_mode mode) {
    uint8_t buf[1];
    buf[0] = mode == EIZO_DEBUG_MODE_ENABLED ? 1 : 0;
    return eizo_set_value(handle, EIZO_USAGE_DEBUG_MODE, buf, 1);
}

enum eizo_result
eizo_set_osd_indicator(struct eizo_handle *handle, enum eizo_osd_indicator indicator)
{
    uint16_t val = 0x8000;
    if (indicator) {
        val = 0x4000;
    }

    return eizo_set_uint16(handle, EIZO_USAGE_OSD_INDICATOR, val);
}
