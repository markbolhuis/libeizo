#include <stdint.h>
#include <stdio.h>

#include <linux/hid.h>

#include "eizo/debug.h"

extern ssize_t
eizo_get_descriptor(struct eizo_handle *handle, uint8_t *desc);

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
