#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/hidraw.h>

#include "eizo.h"
#include "utils.h"

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

struct eizo_handle {
    int fd;
    uint16_t counter;
    char serial[9];
    char model[17];
    struct hidraw_devinfo devinfo;
    struct hidraw_report_descriptor descriptor;
};

static inline enum eizo_usage
eizo_get_usage(const uint8_t *ptr)
{
    return ((uint32_t)ptr[2]) << 0
         | ((uint32_t)ptr[3]) << 8
         | ((uint32_t)ptr[0]) << 16
         | ((uint32_t)ptr[1]) << 24;
}

static inline void
eizo_put_usage(uint8_t *buf, enum eizo_usage usage)
{
    buf[2] = (uint8_t)(usage >> 0);
    buf[3] = (uint8_t)(usage >> 8);
    buf[0] = (uint8_t)(usage >> 16);
    buf[1] = (uint8_t)(usage >> 24);
}

[[maybe_unused]]
static int
eizo_get_counter(struct eizo_handle *handle, uint16_t *counter) 
{
    uint8_t buf[3];
    buf[0] = EIZO_REPORT_ID_COUNTER;

    int res = ioctl(handle->fd, HIDIOCGFEATURE(3), buf);
    if (res < 0) {
        fprintf(stderr, "HIDIOCGFEATURE\n");
    } else {
        *counter = get_unaligned_le16(buf + 1);
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
static ssize_t
eizo_get_descriptor(struct eizo_handle *handle, uint8_t *dst)
{
    uint8_t buf[517];
    buf[0] = EIZO_REPORT_ID_DESCRIPTOR;
    memset(buf + 1, 0, 516);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(517), buf);
    if (rc < 0) {
        perror("HIDIOCSFEATURE");
        return -1;
    }

    uint16_t desc_len = 0, pos = 0;
    do {
        rc = ioctl(handle->fd, HIDIOCGFEATURE(517), buf);
        if (rc < 0) {
            perror("HIDIOCGFEATURE");
            return -1;
        }

        uint16_t offset = get_unaligned_le16(buf + 1);
        uint16_t len    = get_unaligned_le16(buf + 3);

        if (offset != pos) {
            fprintf(stderr, "%s: Invalid offset %u != %u.", __func__, offset, pos);
            return -1;
        }

        if (desc_len == 0) {
            if (len > HID_MAX_DESCRIPTOR_SIZE || len == 0) {
                fprintf(stderr, "%s: Invalid descriptor size %u.", __func__, len);
                return -1;
            }
            desc_len = len;
        } else if (desc_len != len) {
            fprintf(stderr, "%s: Invalid length %u at position %u.", __func__, len, pos);
            return -1;
        }

        uint16_t cpy = MIN(desc_len - pos, 512);
        memcpy(dst + pos, buf + 5, cpy);

        pos += 512;
    } while (pos < desc_len);

    return (ssize_t)desc_len;
}

static int
eizo_verify(struct eizo_handle *handle, enum eizo_usage usage)
{
    uint8_t buf[8];
    buf[0] = EIZO_REPORT_ID_VERIFY;
    memset(buf + 1, 0, 7);

    int rc = ioctl(handle->fd, HIDIOCGFEATURE(8), buf);
    if (rc < 0) {
        perror("HIDIOCGFEATURE");
        return rc;
    }

    enum eizo_usage last_usage = eizo_get_usage(buf + 1);
    uint16_t last_counter = get_unaligned_le16(buf + 5);

    if (last_counter != handle->counter) {
        // Another handle is communicating with the monitor,
        // causing a race condition.
        fprintf(stderr, "%s: unknown handle counter: %u != %u\n",
                __func__, handle->counter, last_counter);
        return -1;
    }

    if (last_usage != usage) {
        // The process using this handle is reading or writing
        // then verifying out of order.
        fprintf(stderr, "%s: unknown last usage: %08x != %08x\n",
                __func__, usage, last_usage);
        return -1;
    }

    if (buf[7] != 0) {
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
    uint8_t buf[519];
    size_t cap;

    if (len <= 32) {
        buf[0] = EIZO_REPORT_ID_GET;
        cap = 39;
    } else if (len <= 512) {
        buf[0] = EIZO_REPORT_ID_GET_V2;
        cap = 519;
    } else {
        errno = EINVAL;
        return -1;
    }

    eizo_put_usage(buf + 1, usage);
    put_unaligned_le16(buf + 5, handle->counter);
    memset(buf + 7, 0, cap - 7);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(cap), buf);
    if (rc < 0) {
        perror("HIDIOCSFEATURE");
        return rc;
    }

    rc = ioctl(handle->fd, HIDIOCGFEATURE(cap), buf);
    if (rc < 0) {
        perror("HIDIOCGFEATURE");
        return rc;
    }

    rc = eizo_verify(handle, usage);
    if (rc >= 0) {
        memcpy(value, buf + 7, len);
    }
    return rc;
}

int
eizo_set_value(struct eizo_handle *handle, enum eizo_usage usage, uint8_t *value, size_t len)
{
    uint8_t buf[519];
    size_t cap;

    if (len <= 32) {
        buf[0] = EIZO_REPORT_ID_SET;
        cap = 39;
    } else if (len <= 512) {
        buf[0] = EIZO_REPORT_ID_SET_V2;
        cap = 519;
    } else {
        errno = EINVAL;
        return -1;
    }

    eizo_put_usage(buf + 1, usage);
    put_unaligned_le16(buf + 5, handle->counter);
    memcpy(buf + 7, value, len);
    memset(buf + 7 + len, 0, cap - 7 - len);

    int rc = ioctl(handle->fd, HIDIOCSFEATURE(cap), buf);
    if (rc < 0) {
        perror("HIDIOCSFEATURE");
        return rc;
    }

    return eizo_verify(handle, usage);
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

int
eizo_get_brightness(struct eizo_handle *handle, uint16_t *value)
{
    alignas(uint16_t) uint8_t buf[2];
    int res = eizo_get_value(handle, EIZO_USAGE_BRIGHTNESS, buf, 2);
    if (res == 0) {
        *value = get_le16(buf);
    }
    return res;
}

int
eizo_set_brightness(struct eizo_handle *handle, uint16_t value)
{
    alignas(uint16_t) uint8_t buf[2];
    put_le16(buf, value);
    return eizo_set_value(handle, EIZO_USAGE_BRIGHTNESS, buf, 2);
}

int
eizo_get_contrast(struct eizo_handle *handle, uint16_t *value)
{
    alignas(uint16_t) uint8_t buf[2];
    int res = eizo_get_value(handle, EIZO_USAGE_CONTRAST, buf, 2);
    if (res == 0) {
        *value = get_le16(buf);
    }
    return res;
}

int
eizo_set_contrast(struct eizo_handle *handle, uint16_t value)
{
    alignas(uint16_t) uint8_t buf[2];
    put_le16(buf, value);
    return eizo_set_value(handle, EIZO_USAGE_CONTRAST, buf, 2);
}

int
eizo_get_usage_time(struct eizo_handle *handle, struct eizo_usage_time *usage)
{
    alignas(uint16_t) uint8_t buf[3];
    int res = eizo_get_value(handle, EIZO_USAGE_USAGE_TIME, buf, 3);
    if (res == 0) {
        usage->hour = get_le16(buf + 0);
        usage->minute = buf[2];
    }
    return res;
}

int
eizo_set_usage_time(struct eizo_handle *handle, struct eizo_usage_time usage)
{
    alignas(uint16_t) uint8_t buf[3];
    put_le16(buf + 0, usage.hour);
    buf[2] = usage.minute;
    return eizo_set_value(handle, EIZO_USAGE_USAGE_TIME, buf, 3);
}

// region Debug

void
eizo_dbg_dump_descriptor(struct eizo_handle *handle)
{
    uint8_t desc[HID_MAX_DESCRIPTOR_SIZE];

    ssize_t n = eizo_get_descriptor(handle, desc);
    if (n < 0) {
        fprintf(stderr, "%s: reading the descriptor failed.\n", __func__);
        return;
    }

    printf("descriptor size: %ld", n);

    for (ssize_t i = 0; i < n; ++i) {
        if (i % 16 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
        printf("%02x", desc[i]);
    }

    printf("\n");
}

// endregion
