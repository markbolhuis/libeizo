#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include <poll.h>
#include <unistd.h>

#include <linux/hid.h>

#include "eizo/eizo.h"
#include "eizo/debug.h"
#include "internal.h"

static void
eizo_print_hex(uint8_t *data, size_t size)
{
    printf("size %lu", size);
    for (size_t i = 0; i < size; ++i) {
        if (i % 16 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
        printf("%02x", data[i]);
    }
    printf("\n");
}

void
eizo_dbg_dump_secondary_descriptor(struct eizo_handle *handle)
{
    uint8_t desc[HID_MAX_DESCRIPTOR_SIZE];

    int n = eizo_get_secondary_descriptor(handle, desc);
    if (n < 0) {
        fprintf(stderr, "%s: reading the descriptor failed.\n", __func__);
        return;
    }

    printf("secondary descriptor ");
    eizo_print_hex(desc, (size_t)n);
}

void
eizo_dbg_dump_ff300009(struct eizo_handle *handle)
{
    uint8_t info[EIZO_FF300009_MAX_SIZE];

    int size = eizo_get_ff300009(handle, info);
    if (size < 0) {
        fprintf(stderr, "%s: reading ff300009 info failed.\n", __func__);
        return;
    }

    printf("info size: %d\n", size);

    int i = 0;
    while (i < size) {
        int key = info[i++];
        printf("%02x", key);
        if (key == EIZO_FF300009_KEY_END || i == size) {
            printf("\n");
            break;
        }

        int len = info[i++];
        printf(" %02x", len);

        if ((i + len) > size) {
            len = size - i;
        }

        for (int j = i; j < (i + len); ++j) {
            printf(" %02x", info[j]);
        }

        printf("\n");
        i += len;
    }
}

void
eizo_dbg_dump_custom_key_lock(struct eizo_handle *handle)
{
    union {
        struct {
            uint16_t offset;
            uint16_t size;
        } __attribute__((packed));
        uint8_t buf[64];
    } u;

    int rc = eizo_get_value(handle,
                            EIZO_USAGE_EV_CUSTOM_KEY_LOCK_OFFSET_SIZE,
                            u.buf, 4);
    if (rc < 0) {
        fprintf(stderr, "%s: Failed to get offset and size\n", __func__);
        return;
    }

    long size   = le16toh(u.size);
    long offset = le16toh(u.offset);

    if (offset != 0) {
        memset(u.buf, 0, 4);
        rc = eizo_set_value(handle,
                            EIZO_USAGE_EV_CUSTOM_KEY_LOCK_OFFSET_SIZE,
                            u.buf, 4);
        if (rc < 0) {
            fprintf(stderr, "%s: Failed to reset offset.\n", __func__);
            return;
        }
    }

    uint8_t *data = malloc(size);
    if (!data) {
        fprintf(stderr, "%s: %s\n", __func__, strerror(errno));
        return;
    }

    for (long i = 0; i < size; i += 62) {
        rc = eizo_get_value(handle,
                            EIZO_USAGE_EV_CUSTOM_KEY_LOCK_DATA,
                            u.buf, 64);
        if (rc < 0) {
            fprintf(stderr, "%s: Failed to get data at %ld.\n", __func__, i);
            goto end;
        }

        offset = le16toh(u.offset);
        if (offset != i) {
            fprintf(stderr, "%s: Offset %ld != %ld.\n", __func__, offset, i);
            goto end;
        }

        memcpy(data + i, u.buf + 2, 62);
    }

    printf("custom key lock ");
    eizo_print_hex(data, (size_t)size);
end:
    free(data);
}

int
eizo_dbg_poll(struct eizo_handle *handle)
{
    struct eizo_value_report r = {};

    struct pollfd pfds[1];
    pfds[0].fd = eizo_get_fd(handle);
    pfds[0].events = POLLIN;
    pfds[0].revents = 0;

    int rc;
    while (true) {
        rc = poll(pfds, 1, -1);
        if (rc <= 0) {
            break;
        }

        if (pfds[0].revents & POLLIN) {
            ssize_t n = read(pfds[0].fd, &r, sizeof(r));

            if (n >= 7) {
                enum eizo_usage usage = eizo_swap_usage(r.usage);
                uint16_t counter = le16toh(r.counter);

                printf("%u %08x %u ", r.report_id, usage, counter);

                n -= 7;

                for (ssize_t i = 0; i < n; ++i) {
                    printf("%02x", r.value[i]);
                }
                printf("\n");
            } else {
                fprintf(stderr, "%s: read < 7\n", __func__);
            }
        }
    }

    return rc;
}
