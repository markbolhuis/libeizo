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

    printf("custom key lock size: %ld\n", size);
    long i = 0;
    while (i < (size - 2)) {
        uint32_t key = 0;
        key |= data[i++] << 16;
        key |= data[i++] << 8;
        key |= data[i++];

        printf("0x%06x", key);
        if (i >= size) {
            printf("\n");
            break;
        }

        long len = data[i++];
        printf(" %3ld", len);

        if ((i + len) > size) {
            len = size - i;
        }

        for (long j = i; j < (i + len); ++j) {
            printf(" %02x", data[j]);
        }

        printf("\n");
        i += len;
    }
    if (i < size) {
        printf("Rem:");
        for (long j = i; j < size; ++j) {
            printf(" %02x", data[j]);
        }
        printf("\n");
    }

end:
    free(data);
}

void
eizo_dbg_dump_edid(struct eizo_handle *handle)
{
    uint8_t buf[256];

    int rc = eizo_get_value(handle, EIZO_USAGE_EDID, buf, 256);
    if (rc < 0) {
        return;
    }

    printf("edid ");
    eizo_print_hex(buf, 256);
}

void
eizo_dbg_dump_gain_definition(struct eizo_handle *handle)
{
    uint8_t def[75];

    int rc = eizo_get_value(handle, EIZO_USAGE_GAIN_DEFINITION_1, def, 1);
    if (rc < 0) {
        return;
    }
    printf("gain definition: %u\n", def[0]);

    rc = eizo_get_value(handle, EIZO_USAGE_GAIN_DEFINITION_2, def, 75);
    if (rc < 0) {
        return;
    }

    static const char *names[] = {
        "NONE", "4000K", "4500K", "5000K", "5500K", "6000K", "6500K", "7000K",
        "7500K", "8000K", "8500K", "9000K", "9300K", "9500K", "10000K",
        "10500K", "11000K", "11500K", "12000K", "12500K", "13000K", "13500K",
        "14000K", "14500K", "15000K",
    };

    for (int i = 0; i < 25; ++i) {
        uint32_t color = 0;
        color |= def[i * 3] << 16;
        color |= def[i * 3 + 1] << 8;
        color |= def[i * 3 + 2];

        printf("%-7s#%06x\n", names[i], color);
    }
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
