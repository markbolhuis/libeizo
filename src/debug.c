#include <stdint.h>
#include <stdio.h>

#include <linux/hid.h>

#include "eizo/debug.h"

extern ssize_t
eizo_get_descriptor(struct eizo_handle *handle, uint8_t *desc);

extern int
eizo_get_ff300009(struct eizo_handle *handle, uint8_t *info);

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

void
eizo_dbg_dump_ff300009(struct eizo_handle *handle)
{
    uint8_t info[256];

    int rc = eizo_get_ff300009(handle, info);
    if (rc < 0) {
        fprintf(stderr, "%s: reading ff300009 info failed.\n", __func__);
        return;
    }

    int i = 0;
    while (i < 256) {
        int key = info[i++];
        printf("%02x", key);
        if (key == 0xff || i == 256) {
            printf("\n");
            break;
        }

        int len = info[i++];
        printf(" %02x", len);

        if ((i + len) > 256) {
            len = 256 - i;
        }

        for (int j = i; j < (i + len); ++j) {
            printf(" %02x", info[j]);
        }

        printf("\n");
        i += len;
    }
}
