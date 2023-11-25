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

struct eizo_handle {
    int fd;
    uint16_t counter;
    char serial[9];
    char model[17];
    struct hidraw_devinfo devinfo;
    struct hidraw_report_descriptor descriptor;
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

[[maybe_unused]]
static int
eizo_get_counter(struct eizo_handle *handle, uint16_t *counter) 
{
    struct eizo_counter_report r = {};
    r.report_id = EIZO_REPORT_ID_COUNTER;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(3), &r);
    if (res < 0) {
        fprintf(stderr, "HIDIOCGFEATURE\n");
    } else {
        *counter = le16toh(r.counter);
    }

    return res;
}

[[maybe_unused]]
static int
eizo_get_serial_model(struct eizo_handle *handle, char serial[9], char model[17]) 
{
    char buf[25];
    buf[0] = EIZO_REPORT_ID_SN_MODEL;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(25), buf);
    if (res < 0) {
        perror("HIDIOCGFEATURE");
        return -1;
    }

    strncpy(serial, buf + 1, 8);
    serial[8] = '\0';

    strncpy(model, buf + 9, 16);
    model[16] = '\0';

    return 0;
}

[[maybe_unused]]
ssize_t
eizo_get_descriptor(struct eizo_handle *handle, uint8_t *dst)
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

    return (ssize_t)desc_len;
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
    size_t cap;

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
    size_t cap;

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
    // Assume 256 bytes for now, which seems to be the limit for this report.
    uint8_t buf[257];
    buf[0] = EIZO_REPORT_ID_INFO;

    int rc = ioctl(handle->fd, HIDIOCGFEATURE(257), buf);
    if (rc < 0) {
        perror("HIDIOCGFEATURE");
        return -1;
    }

    memcpy(info, buf + 1, rc);
    return rc;
}


struct eizo_handle *
eizo_open_path(const char *path)
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

    int res = ioctl(h->fd, HIDIOCGRDESCSIZE, &h->descriptor.size);
    if (res < 0) {
        perror("HIDIOCGRDESCSIZE");
        goto err_ioctl;
    }

    res = ioctl(h->fd, HIDIOCGRDESC, &h->descriptor);
    if (res < 0) {
        perror("HIDIOCGRDESC");
        goto err_ioctl;
    }

    res = ioctl(h->fd, HIDIOCGRAWINFO, &h->devinfo);
    if (res < 0) {
        perror("HIDIOCGRAWINFO");
        goto err_ioctl;
    }

    eizo_get_counter(h, &h->counter);
    return h;
err_ioctl:
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
    return handle->devinfo.product;
}

int
eizo_get_fd(struct eizo_handle *handle)
{
    return handle->fd;
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
eizo_get_usage_time(struct eizo_handle *handle, struct eizo_usage_time *usage)
{
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    int res = eizo_get_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
    if (res >= 0) {
        usage->hour = le16toh(u.hour);
        usage->minute = u.buf[2];
    }
    return res;
}

int
eizo_set_usage_time(struct eizo_handle *handle, struct eizo_usage_time usage)
{
    union {
        uint16_t hour;
        uint8_t buf[3];
    } u;
    u.hour = htole16(usage.hour);
    u.buf[2] = usage.minute;
    return eizo_set_value(handle, EIZO_USAGE_USAGE_TIME, u.buf, 3);
}
