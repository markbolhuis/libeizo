#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/hidraw.h>

#include "eizo/eizo.h"
#include "internal.h"

struct eizo_handle {
    int fd;
    uint16_t counter;
    enum eizo_pid pid;
    unsigned long serial;
    char model[17];
    struct hidraw_report_descriptor descriptor;
};

[[maybe_unused]]
static enum eizo_result
eizo_get_counter(struct eizo_handle *handle, uint16_t *counter) 
{
    struct eizo_counter_report r = {};
    r.report_id = EIZO_REPORT_ID_COUNTER;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(3), &r);
    if (res < 0) {
        return EIZO_ERROR_IO;
    }

    *counter = le16toh(r.counter);
    return EIZO_SUCCESS;
}

[[maybe_unused]]
static enum eizo_result
eizo_get_serial_model(
    struct eizo_handle *handle,
    unsigned long *serial,
    char model[17])
{
    char buf[25];
    buf[0] = EIZO_REPORT_ID_SN_MODEL;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(25), buf);
    if (res < 0) {
        return EIZO_ERROR_IO;
    }

    int i;
    for (i = 0; i < 16; ++i) {
        if (buf[9 + i] == ' ') {
            break;
        }
        model[i] = buf[9 + i];
    }
    model[i] = '\0';

    buf[9] = '\0';
    char *end = nullptr;
    unsigned long sn = strtoul(buf + 1, &end, 10);

    if (end == buf + 9) {
        *serial = sn;
    } else {
        fprintf(stderr, "%s: failed to convert serial string to ulong.\n",
                __func__);
    }

    return EIZO_SUCCESS;
}

[[maybe_unused]]
enum eizo_result
eizo_get_secondary_descriptor(
    struct eizo_handle *handle,
    uint8_t *dst,
    int *size)
{
    struct eizo_descriptor_report r = {};
    r.report_id = EIZO_REPORT_ID_DESCRIPTOR;

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(517), &r);
    if (rc < 0) {
        return EIZO_ERROR_IO;
    }

    uint16_t desc_len = 0, pos = 0;
    do {
        rc = ioctl(handle->fd, HIDIOCGFEATURE(517), &r);
        if (rc < 0) {
            return EIZO_ERROR_IO;
        }

        uint16_t offset = le16toh(r.offset);
        uint16_t len    = le16toh(r.length);

        if (offset != pos) {
            fprintf(stderr, "%s: Invalid offset %u != %u.\n", __func__, offset, pos);
            return EIZO_ERROR_UNKNOWN;
        }

        if (desc_len == 0) {
            if (len > HID_MAX_DESCRIPTOR_SIZE || len == 0) {
                fprintf(stderr, "%s: Invalid descriptor size %u.\n", __func__, len);
                return EIZO_ERROR_UNKNOWN;
            }
            desc_len = len;
        } else if (desc_len != len) {
            fprintf(stderr, "%s: Invalid length %u at position %u.\n", __func__, len, pos);
            return EIZO_ERROR_UNKNOWN;
        }

        uint16_t cpy = MIN(desc_len - pos, 512);
        memcpy(dst + pos, r.desc, cpy);

        pos += 512;
    } while (pos < desc_len);

    *size = (int)desc_len;
    return EIZO_SUCCESS;
}

static enum eizo_result
eizo_verify(struct eizo_handle *handle, enum eizo_usage usage)
{
    struct eizo_verify_report r = {};
    r.report_id = EIZO_REPORT_ID_VERIFY;

    int rc = ioctl(handle->fd, HIDIOCGFEATURE(8), &r);
    if (rc < 0) {
        return EIZO_ERROR_IO;
    }

    if (le16toh(r.counter) != handle->counter) {
        return EIZO_ERROR_RACE_CONDITION;
    }

    if (eizo_swap_usage(r.usage) != usage) {
        return EIZO_ERROR_INVALID_USAGE;
    }

    if (r.result != 0) {
        return EIZO_ERROR_NOT_PERMITTED;
    }

    return EIZO_SUCCESS;
}

enum eizo_result
eizo_get_value(
    struct eizo_handle *handle,
    enum eizo_usage usage,
    uint8_t *value,
    size_t len)
{
    struct eizo_value_report r = {};
    unsigned long cap;

    if (len <= 32) {
        r.report_id = EIZO_REPORT_ID_GET;
        cap = 39;
    } else if (len <= 512) {
        r.report_id = EIZO_REPORT_ID_GET_V2;
        cap = 519;
    } else {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }

    r.usage = eizo_swap_usage(usage);
    r.counter = htole16(handle->counter);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(cap), &r);
    if (rc < 0) {
        return EIZO_ERROR_IO;
    }

    rc = ioctl(handle->fd, HIDIOCGFEATURE(cap), &r);
    if (rc < 0) {
        return EIZO_ERROR_IO;
    }

    enum eizo_result res = eizo_verify(handle, usage);
    if (res >= EIZO_SUCCESS) {
        memcpy(value, r.value, len);
    }
    return res;
}

enum eizo_result
eizo_set_value(
    struct eizo_handle *handle,
    enum eizo_usage usage,
    uint8_t *value,
    size_t len)
{
    struct eizo_value_report r = {};
    unsigned long cap;

    if (len <= 32) {
        r.report_id = EIZO_REPORT_ID_SET;
        cap = 39;
    } else if (len <= 512) {
        r.report_id = EIZO_REPORT_ID_SET_V2;
        cap = 519;
    } else {
        return EIZO_ERROR_INVALID_ARGUMENT;
    }

    r.usage = eizo_swap_usage(usage);
    r.counter = htole16(handle->counter);
    memcpy(r.value, value, len);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(cap), &r);
    if (rc < 0) {
        return EIZO_ERROR_IO;
    }

    return eizo_verify(handle, usage);
}

enum eizo_result
eizo_get_ff300009(struct eizo_handle *handle, uint8_t *info, int *size)
{
    uint8_t buf[EIZO_FF300009_MAX_SIZE + 1];
    buf[0] = EIZO_REPORT_ID_INFO;

    int s = ioctl(handle->fd, HIDIOCGFEATURE(sizeof(buf)), buf);
    if (s < 0) {
        return EIZO_ERROR_IO;
    }

    if (s > 1) {
        memcpy(info, buf + 1, s - 1);
    }
    *size = s == 0 ? 0 : s - 1;
    return EIZO_SUCCESS;
}


static enum eizo_result
eizo_get_hidraw_descriptor(struct eizo_handle *handle)
{
    int size = -1;

    int res = ioctl(handle->fd, HIDIOCGRDESCSIZE, &size);
    if (res < 0 || size < 0) {
        return EIZO_ERROR_IO;
    }

    handle->descriptor.size = size;
    res = ioctl(handle->fd, HIDIOCGRDESC, &handle->descriptor);
    if (res < 0) {
        return EIZO_ERROR_IO;
    }

    return EIZO_SUCCESS;
}

static enum eizo_result
eizo_get_hidraw_devinfo(struct eizo_handle *handle)
{
    struct hidraw_devinfo devinfo = {};

    int res = ioctl(handle->fd, HIDIOCGRAWINFO, &devinfo);
    if (res < 0) {
        return EIZO_ERROR_IO;
    }

    handle->pid = devinfo.product;
    return EIZO_SUCCESS;
}

enum eizo_result
eizo_open_hidraw(const char *path, struct eizo_handle **handle)
{
    struct eizo_handle *h = calloc(1, sizeof *h);
    if (!h) {
        return EIZO_ERROR_NO_MEMORY;
    }

    enum eizo_result res;

    h->fd = open(path, O_RDWR | O_CLOEXEC);
    if (h->fd < 0) {
        fprintf(stderr, "%s: Failed to open %s. %s\n", __func__, path, strerror(errno));
        res = EIZO_ERROR_IO;
        goto err_open;
    }

    res = eizo_get_hidraw_descriptor(h);
    if (res < EIZO_SUCCESS) {
        fprintf(stderr, "%s: Failed to read hidraw descriptor.\n", __func__);
        goto err_hidraw;
    }

    res = eizo_get_hidraw_devinfo(h);
    if (res < EIZO_SUCCESS) {
        fprintf(stderr, "%s: Failed to read hidraw devinfo.\n", __func__);
        goto err_hidraw;
    }

    eizo_get_counter(h, &h->counter);
    eizo_get_serial_model(h, &h->serial, h->model);
    *handle = h;
    return EIZO_SUCCESS;

err_hidraw:
    close(h->fd);
err_open:
    free(h);
    return res;
}

void 
eizo_close(struct eizo_handle *handle) 
{
    close(handle->fd);
    free(handle);
}

enum eizo_pid
eizo_get_pid(struct eizo_handle *handle)
{
    return handle->pid;
}

int
eizo_get_fd(struct eizo_handle *handle)
{
    return handle->fd;
}

unsigned long
eizo_get_serial(struct eizo_handle *handle)
{
    return handle->serial;
}

const char *
eizo_get_model(struct eizo_handle *handle)
{
    return handle->model;
}

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
eizo_get_brightness(struct eizo_handle *handle, uint16_t *value)
{
    return eizo_get_uint16(handle, EIZO_USAGE_BRIGHTNESS, value);
}

enum eizo_result
eizo_set_brightness(struct eizo_handle *handle, uint16_t value)
{
    return eizo_set_uint16(handle, EIZO_USAGE_BRIGHTNESS, value);
}

enum eizo_result
eizo_get_contrast(struct eizo_handle *handle, uint16_t *value)
{
    return eizo_get_uint16(handle, EIZO_USAGE_CONTRAST, value);
}

enum eizo_result
eizo_set_contrast(struct eizo_handle *handle, uint16_t value)
{
    return eizo_set_uint16(handle, EIZO_USAGE_CONTRAST, value);
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
        } __attribute__((packed));
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
            res = EIZO_ERROR_UNKNOWN;
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
