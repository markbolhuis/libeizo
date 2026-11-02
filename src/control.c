#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

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
        return EIZO_ERROR_OUT_OF_RANGE;
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
        return EIZO_ERROR_OUT_OF_RANGE;
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
    enum eizo_result res = eizo_get_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
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
        return EIZO_ERROR_OUT_OF_RANGE;
    }
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    u.buf[2] = time % 60;
    time /= 60;
    if (time > UINT16_MAX) {
        return EIZO_ERROR_OUT_OF_RANGE;
    }
    u.hour = htole16(time);
    return eizo_set_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
}

enum eizo_result
eizo_get_available_custom_key_lock_raw(struct eizo_handle *handle, uint8_t **ptr, size_t *len)
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
eizo_set_debug_mode(struct eizo_handle *handle, enum eizo_debug_mode mode)
{
    return eizo_set_value(handle, EIZO_USAGE_DEBUG_MODE, (uint8_t[]) { mode ? 1 : 0 }, 1);
}

enum eizo_result
eizo_get_debug_mode(struct eizo_handle *handle, enum eizo_debug_mode *mode)
{
	uint8_t val[1];
	enum eizo_result ret = eizo_get_value(handle, EIZO_USAGE_DEBUG_MODE, val, 1);
	if (ret < EIZO_SUCCESS) {
		return ret;
	}
	if (val[0] > 1) {
		return EIZO_ERROR_BAD_DATA;
	}
	*mode = val[0] == 1;
	return ret;
}

enum eizo_result
eizo_set_osd_indicator(struct eizo_handle *handle, enum eizo_osd_indicator indicator)
{
    return eizo_set_uint16(handle, EIZO_USAGE_OSD_INDICATOR, indicator ? 0x4000 : 0x8000);
}

enum eizo_result
eizo_get_input_port(struct eizo_handle *handle, enum eizo_input_port *port)
{
    uint16_t value = 0;
    enum eizo_result res =
        eizo_get_uint16(handle, EIZO_USAGE_INPUT_PORT, &value);
    if (res >= EIZO_SUCCESS) {
        *port = (enum eizo_input_port)value;
    }
    return res;
}

enum eizo_result
eizo_set_input_port(struct eizo_handle *handle, enum eizo_input_port port)
{
    return eizo_set_uint16(handle, EIZO_USAGE_INPUT_PORT, (uint16_t)port);
}

static const struct {
    enum eizo_input_port port;
    const char *name;
} input_ports[] = {
    { EIZO_INPUT_PORT_UNDEFINED, "undefined" },
    { EIZO_INPUT_PORT_DSUB1, "dsub1" },
    { EIZO_INPUT_PORT_DSUB2, "dsub2" },
    { EIZO_INPUT_PORT_DVI1, "dvi1" },
    { EIZO_INPUT_PORT_DVI2, "dvi2" },
    { EIZO_INPUT_PORT_DP1, "dp1" },
    { EIZO_INPUT_PORT_DP2, "dp2" },
    { EIZO_INPUT_PORT_HDMI1, "hdmi1" },
    { EIZO_INPUT_PORT_HDMI2, "hdmi2" },
    { EIZO_INPUT_PORT_SDI1, "sdi1" },
    { EIZO_INPUT_PORT_SDI2, "sdi2" },
    { EZIO_INPUT_PORT_SDIAB, "sdiab" },
    { EZIO_INPUT_PORT_SDIBA, "sdiba" },
    { EIZO_INPUT_PORT_D1, "d1" },
    { EIZO_INPUT_PORT_D2, "d2" },
    { EIZO_INPUT_PORT_COMPOSITE1, "composite1" },
    { EIZO_INPUT_PORT_COMPOSITE2, "composite2" },
    { EIZO_INPUT_PORT_TV1, "tv1" },
    { EIZO_INPUT_PORT_TV2, "tv2" },
    { EIZO_INPUT_PORT_USBC, "usbc" },
};

const char *
eizo_input_port_to_string(enum eizo_input_port port)
{
    size_t count = sizeof(input_ports) / sizeof(input_ports[0]);
    for (size_t i = 0; i < count; ++i) {
        if (input_ports[i].port == port) {
            return input_ports[i].name;
        }
    }
    return "unknown";
}

enum eizo_result
eizo_input_port_from_string(const char *name, enum eizo_input_port *port)
{
    if (name == NULL || port == NULL) {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }
    size_t count = sizeof(input_ports) / sizeof(input_ports[0]);
    for (size_t i = 0; i < count; ++i) {
        if (input_ports[i].port != EIZO_INPUT_PORT_UNDEFINED &&
            strcmp(input_ports[i].name, name) == 0) {
            *port = input_ports[i].port;
            return EIZO_SUCCESS;
        }
    }
    return EIZO_ERROR_INVALID_ARGUMENT;
}
