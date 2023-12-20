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
static int
eizo_get_counter(struct eizo_handle *handle, uint16_t *counter) 
{
    struct eizo_counter_report r = {};
    r.report_id = EIZO_REPORT_ID_COUNTER;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(3), &r);
    if (res < 0) {
        fprintf(stderr, "HIDIOCGFEATURE\n");
        return -1;
    }

    *counter = le16toh(r.counter);
    return 0;
}

[[maybe_unused]]
static int
eizo_get_serial_model(
    struct eizo_handle *handle,
    unsigned long *serial,
    char model[17])
{
    char buf[25];
    buf[0] = EIZO_REPORT_ID_SN_MODEL;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(25), buf);
    if (res < 0) {
        perror("HIDIOCGFEATURE");
        return -1;
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

    return 0;
}

[[maybe_unused]]
int
eizo_get_secondary_descriptor(struct eizo_handle *handle, uint8_t *dst)
{
    struct eizo_descriptor_report r = {};
    r.report_id = EIZO_REPORT_ID_DESCRIPTOR;

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(517), &r);
    if (rc < 0) {
        perror("HIDIOCSFEATURE");
        return -1;
    }

    uint16_t desc_len = 0, pos = 0;
    do {
        rc = ioctl(handle->fd, HIDIOCGFEATURE(517), &r);
        if (rc < 0) {
            perror("HIDIOCGFEATURE");
            return -1;
        }

        uint16_t offset = le16toh(r.offset);
        uint16_t len    = le16toh(r.length);

        if (offset != pos) {
            fprintf(stderr, "%s: Invalid offset %u != %u.\n", __func__, offset, pos);
            return -1;
        }

        if (desc_len == 0) {
            if (len > HID_MAX_DESCRIPTOR_SIZE || len == 0) {
                fprintf(stderr, "%s: Invalid descriptor size %u.\n", __func__, len);
                return -1;
            }
            desc_len = len;
        } else if (desc_len != len) {
            fprintf(stderr, "%s: Invalid length %u at position %u.\n", __func__, len, pos);
            return -1;
        }

        uint16_t cpy = MIN(desc_len - pos, 512);
        memcpy(dst + pos, r.desc, cpy);

        pos += 512;
    } while (pos < desc_len);

    return (int)desc_len;
}

static int
eizo_verify(struct eizo_handle *handle, enum eizo_usage usage)
{
    struct eizo_verify_report r = {};
    r.report_id = EIZO_REPORT_ID_VERIFY;

    int rc = ioctl(handle->fd, HIDIOCGFEATURE(8), &r);
    if (rc < 0) {
        perror("HIDIOCGFEATURE");
        return rc;
    }

    uint16_t last_counter = le16toh(r.counter);
    if (last_counter != handle->counter) {
        // Another handle is communicating with the monitor,
        // causing a race condition.
        fprintf(stderr, "%s: unknown handle counter: %u != %u\n",
                __func__, handle->counter, last_counter);
        return -1;
    }

    enum eizo_usage last_usage = eizo_swap_usage(r.usage);
    if (last_usage != usage) {
        // The process using this handle is reading or writing
        // then verifying out of order.
        fprintf(stderr, "%s: unknown last usage: %08x != %08x\n",
                __func__, usage, last_usage);
        return -1;
    }

    if (r.result != 0) {
        // The monitor explicitly rejected the request, usually 
        // because the value was invalid, out of range etc...
        fprintf(stderr, "%s: failed to apply request.\n", __func__);
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int
eizo_get_value(struct eizo_handle *handle, enum eizo_usage usage, uint8_t *value, size_t len)
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
        errno = EINVAL;
        return -1;
    }

    r.usage = eizo_swap_usage(usage);
    r.counter = htole16(handle->counter);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(cap), &r);
    if (rc < 0) {
        perror("HIDIOCSFEATURE");
        return rc;
    }

    rc = ioctl(handle->fd, HIDIOCGFEATURE(cap), &r);
    if (rc < 0) {
        perror("HIDIOCGFEATURE");
        return rc;
    }

    rc = eizo_verify(handle, usage);
    if (rc >= 0) {
        memcpy(value, r.value, len);
    }
    return rc;
}

int
eizo_set_value(struct eizo_handle *handle, enum eizo_usage usage, uint8_t *value, size_t len)
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
        errno = EINVAL;
        return -1;
    }

    r.usage = eizo_swap_usage(usage);
    r.counter = htole16(handle->counter);
    memcpy(r.value, value, len);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(cap), &r);
    if (rc < 0) {
        perror("HIDIOCSFEATURE");
        return rc;
    }

    return eizo_verify(handle, usage);
}

int
eizo_get_ff300009(struct eizo_handle *handle, uint8_t *info)
{
    uint8_t buf[EIZO_FF300009_MAX_SIZE + 1];
    buf[0] = EIZO_REPORT_ID_INFO;

    int size = ioctl(handle->fd, HIDIOCGFEATURE(sizeof(buf)), buf);
    if (size < 0) {
        perror("HIDIOCGFEATURE");
        return -1;
    }

    if (size > 1) {
        memcpy(info, buf + 1, size - 1);
    }

    return size == 0 ? 0 : size - 1;
}


static int
eizo_get_hidraw_descriptor(struct eizo_handle *handle)
{
    int size = -1;

    int res = ioctl(handle->fd, HIDIOCGRDESCSIZE, &size);
    if (res < 0 || size < 0) {
        perror("HIDIOCGRDESCSIZE");
        return -1;
    }

    handle->descriptor.size = size;
    res = ioctl(handle->fd, HIDIOCGRDESC, &handle->descriptor);
    if (res < 0) {
        perror("HIDIOCGRDESC");
        return -1;
    }

    return 0;
}

static int
eizo_get_hidraw_devinfo(struct eizo_handle *handle)
{
    struct hidraw_devinfo devinfo = {};

    int res = ioctl(handle->fd, HIDIOCGRAWINFO, &devinfo);
    if (res < 0) {
        perror("HIDIOCGRAWINFO");
        return -1;
    }

    handle->pid = devinfo.product;
    return 0;
}

struct eizo_handle *
eizo_open_hidraw(const char *path)
{
    struct eizo_handle *h = calloc(1, sizeof *h);
    if (!h) {
        perror("calloc");
        return nullptr;
    }

    h->fd = open(path, O_RDWR | O_CLOEXEC);
    if (h->fd < 0) {
        fprintf(stderr, "%s: Failed to open %s. %s\n", __func__, path, strerror(errno));
        goto err_open;
    }

    int res = eizo_get_hidraw_descriptor(h);
    if (res < 0) {
        fprintf(stderr, "%s: Failed to read hidraw descriptor.\n", __func__);
        goto err_hidraw;
    }

    res = eizo_get_hidraw_devinfo(h);
    if (res < 0) {
        fprintf(stderr, "%s: Failed to read hidraw devinfo.\n", __func__);
        goto err_hidraw;
    }

    eizo_get_counter(h, &h->counter);
    eizo_get_serial_model(h, &h->serial, h->model);
    return h;
err_hidraw:
    close(h->fd);
err_open:
    free(h);
    return nullptr;
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

static int
eizo_get_uint16(struct eizo_handle *handle, enum eizo_usage usage, uint16_t *value)
{
    union {
        uint16_t value;
        uint8_t buf[2];
    } u;
    int res = eizo_get_value(handle, usage, u.buf, 2);
    if (res >= 0) {
        *value = le16toh(u.value);
    }
    return res;
}

static int
eizo_set_uint16(struct eizo_handle *handle, enum eizo_usage usage, uint16_t value)
{
    union {
        uint16_t value;
        uint8_t buf[2];
    } u;
    u.value = htole16(value);
    return eizo_set_value(handle, usage, u.buf, 2);
}

int
eizo_get_brightness(struct eizo_handle *handle, uint16_t *value)
{
    return eizo_get_uint16(handle, EIZO_USAGE_BRIGHTNESS, value);
}

int
eizo_set_brightness(struct eizo_handle *handle, uint16_t value)
{
    return eizo_set_uint16(handle, EIZO_USAGE_BRIGHTNESS, value);
}

int
eizo_get_contrast(struct eizo_handle *handle, uint16_t *value)
{
    return eizo_get_uint16(handle, EIZO_USAGE_CONTRAST, value);
}

int
eizo_set_contrast(struct eizo_handle *handle, uint16_t value)
{
    return eizo_set_uint16(handle, EIZO_USAGE_CONTRAST, value);
}

int
eizo_get_usage_time(struct eizo_handle *handle, long *time)
{
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    int res = eizo_get_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
    if (res >= 0) {
        long t = (long)le16toh(u.hour);
        t *= 60;
        t += (long)u.buf[2];
        *time = t;
    }
    return res;
}

int
eizo_set_usage_time(struct eizo_handle *handle, long time)
{
    if (time < 0) {
        errno = ERANGE;
        return -1;
    }
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    u.buf[2] = time % 60;
    time /= 60;
    if (time > UINT16_MAX) {
        errno = ERANGE;
        return -1;
    }
    u.hour = htole16(time);
    return eizo_set_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
}
