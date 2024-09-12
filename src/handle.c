#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/hidraw.h>

#include "eizo/handle.h"
#include "internal.h"

struct eizo_handle {
    int fd;
    uint16_t counter;
    enum eizo_pid pid;
    unsigned long serial;
    char model[17];
    struct {
        uint8_t desc;
        uint8_t set;
        uint8_t get;
        uint8_t set_v2;
        uint8_t get_v2;
        uint8_t counter;
        uint8_t verify;
        uint8_t sn_prod;
        uint8_t key_value;
    } rid;
};

static enum eizo_result
eizo_get_counter(struct eizo_handle *handle, uint16_t *counter) 
{
    struct eizo_counter_report r = {};
    r.report_id = handle->rid.counter;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(3), &r);
    if (res < 0) {
        return EIZO_ERROR_IO;
    }

    *counter = le16toh(r.counter);
    return EIZO_SUCCESS;
}

static enum eizo_result
eizo_get_serial_model(
    struct eizo_handle *handle,
    unsigned long *serial,
    char model[17])
{
    char buf[25];
    buf[0] = (char)handle->rid.sn_prod;

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

enum eizo_result
eizo_get_secondary_descriptor(
    struct eizo_handle *handle,
    uint8_t *dst,
    int *size)
{
    struct eizo_descriptor_report r = {};
    r.report_id = handle->rid.desc;

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
            return EIZO_ERROR_BAD_DATA;
        }

        if (desc_len == 0) {
            if (len > HID_MAX_DESCRIPTOR_SIZE || len == 0) {
                fprintf(stderr, "%s: Invalid descriptor size %u.\n", __func__, len);
                return EIZO_ERROR_BAD_DATA;
            }
            desc_len = len;
        } else if (desc_len != len) {
            fprintf(stderr, "%s: Invalid length %u at position %u.\n", __func__, len, pos);
            return EIZO_ERROR_BAD_DATA;
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
    r.report_id = handle->rid.verify;

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
        r.report_id = handle->rid.get;
        cap = 39;
    } else if (len <= 512) {
        r.report_id = handle->rid.get_v2;
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
        r.report_id = handle->rid.set;
        cap = 39;
    } else if (len <= 512) {
        r.report_id = handle->rid.set_v2;
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
    buf[0] = handle->rid.key_value;

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
eizo_parse_hidraw_descriptor(struct eizo_handle *handle)
{
    int size = -1;

    int res = ioctl(handle->fd, HIDIOCGRDESCSIZE, &size);
    if (res < 0 || size < 0) {
        return EIZO_ERROR_IO;
    }

    struct hidraw_report_descriptor desc;
    desc.size = (uint32_t)size;

    res = ioctl(handle->fd, HIDIOCGRDESC, &desc);
    if (res < 0) {
        return EIZO_ERROR_IO;
    }

    struct eizo_control control[16];
    size_t clen = 16;

    res = eizo_parse_descriptor(desc.value, desc.size, control, &clen);
    if (res < 0) {
        fprintf(stderr, "%s: failed to parse descriptor. %d\n", __func__, res);
        return res;
    }

    unsigned mask = 0;
    for (size_t i = 0; i < clen; ++i) {
        switch (control[i].usage) {
            case EIZO_USAGE_SECONDARY_DESCRIPTOR:
                handle->rid.desc = control[i].report_id;
                mask |= 1;
                break;
            case EIZO_USAGE_SET_VALUE:
                handle->rid.set = control[i].report_id;
                mask |= 2;
                break;
            case EIZO_USAGE_GET_VALUE:
                handle->rid.get = control[i].report_id;
                mask |= 4;
                break;
            case EIZO_USAGE_SET_VALUE_V2:
                handle->rid.set_v2 = control[i].report_id;
                mask |= 8;
                break;
            case EIZO_USAGE_GET_VALUE_V2:
                handle->rid.get_v2 = control[i].report_id;
                mask |= 16;
                break;
            case EIZO_USAGE_HANDLE_COUNTER:
                handle->rid.counter = control[i].report_id;
                mask |= 32;
                break;
            case EIZO_USAGE_VERIFY_LAST_REQUEST:
                handle->rid.verify = control[i].report_id;
                mask |= 64;
                break;
            case EIZO_USAGE_SERIAL_PRODUCT_STRING_2:
                handle->rid.sn_prod = control[i].report_id;
                mask |= 128;
                break;
            case EIZO_USAGE_UNKNOWN_KEY_VALUE_PAIRS_2:
                handle->rid.key_value = control[i].report_id;
                mask |= 256;
                break;
            default:
                break;
        }
    }

    if (mask != 511) {
        fprintf(stderr, "%s: failed to find all required usages. 0b%09b\n", __func__, mask);
        return EIZO_ERROR_BAD_DATA;
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

    res = eizo_parse_hidraw_descriptor(h);
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
