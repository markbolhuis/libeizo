#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include <systemd/sd-device.h>

#include "eizo/handle.h"
#include "eizo/debug.h"
#include "eizo/control.h"

void
print_help()
{
    printf("Usage: ./eizoctl <option> [monitor]\n");
    printf("\n");
    printf("Options:\n");
    printf("\tlist            - List all available monitors.\n");
    printf("\tidentify        - Identify the monitor.\n");
    printf("\tpoll            - Poll the monitor for events.\n");
    printf("\tdescriptor      - Read the eizo specific HID report descriptor.\n");
    printf("\tkey-pairs       - Read the unknown key value pairs behind usage ff300009.\n");
    printf("\tread-all        - Read all values for every usage.\n");
    printf("\teeprom          - Read the contents of the internal EEPROM.\n");
    printf("\tcustom-key-lock - Read the available custom key locks, and the currently used one.\n");
    printf("\tgain-definition - Read all available gain definition values.\n");
    printf("\tedid            - Read the monitor edid.\n");
    printf("\tdebug           - Put the monitor into 'debug' mode.\n");
    printf("\thelp            - Show this help message.\n");
}

struct sd_device *
enumerate(const int n)
{
    [[gnu::cleanup(sd_device_enumerator_unrefp)]]
    sd_device_enumerator *enumerator = nullptr;

    int ret = sd_device_enumerator_new(&enumerator);
    if (ret < 0) {
        return nullptr;
    }

    ret = sd_device_enumerator_add_match_subsystem(enumerator, "hidraw", true);
    if (ret < 0) {
        return nullptr;
    }

    int i = 0;
    for (sd_device *device = sd_device_enumerator_get_device_first(enumerator);
         device;
         device = sd_device_enumerator_get_device_next(enumerator))
    {
        sd_device *parent = nullptr;
        ret = sd_device_get_parent_with_subsystem_devtype(
                device,
                "usb",
                "usb_device",
                &parent);
        if (ret < 0) {
            continue;
        }

        const char *vid_str = nullptr, *pid_str = nullptr;
        int rv = sd_device_get_sysattr_value(parent, "idVendor", &vid_str);
        int rp = sd_device_get_sysattr_value(parent, "idProduct", &pid_str);
        if (rv < 0 || rp < 0) {
            continue;
        }

        unsigned long vid = strtoul(vid_str, nullptr, 16);
        unsigned long pid = strtoul(pid_str, nullptr, 16);
        if (vid != EIZO_VID) {
            continue;
        }

        if (n < 0) {
            const char *devname = nullptr;
            ret = sd_device_get_devname(device, &devname);
            if (ret >= 0) {
                printf("%i: %s %04lx\n", i, devname, pid);
            }
        } else {
            if (i == n) {
                return sd_device_ref(device);
            }
        }
        ++i;
    }

    return nullptr;
}

int
main(int argc, const char *argv[]) 
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "help") == 0) {
        print_help();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "list") == 0) {
        enumerate(-1);
        return EXIT_SUCCESS;
    }

    int i = 0;
    if (argv[2]) {
        char *end = nullptr;
        unsigned long u = strtoul(argv[2], &end, 10);
        if (u > INT_MAX) {
            fprintf(stderr, "Invalid value for 'monitor'\n");
            return EXIT_FAILURE;
        }
        i = (int) u;
    }

    [[gnu::cleanup(sd_device_unrefp)]]
    sd_device *device = enumerate(i);
    if (!device) {
        fprintf(stderr, "Device not found.\n");
        return EXIT_FAILURE;
    }

    const int fd = sd_device_open(device, 0);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device. %s\n", strerror(-fd));
        return EXIT_FAILURE;
    }

    eizo_handle_t handle = nullptr;
    enum eizo_result res = eizo_new(fd, &handle);
    if (res < EIZO_SUCCESS || !handle) {
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "poll") == 0) {
        eizo_dbg_poll(handle);
    } else if (strcmp(argv[1], "descriptor") == 0) {
        eizo_dbg_dump_secondary_descriptor(handle);
    } else if (strcmp(argv[1], "key-pairs") == 0) {
        eizo_dbg_dump_ff300009(handle);
    } else if (strcmp(argv[1], "read-all") == 0) {
        eizo_dbg_dump_all_usages(handle);
    } else if (strcmp(argv[1], "eeprom") == 0) {
        printf("Reading eeprom...\n");
        eizo_dbg_dump_eeprom(handle);
    } else if (strcmp(argv[1], "custom-key-lock") == 0) {
        eizo_dbg_dump_available_custom_key_lock(handle);
        eizo_dbg_dump_custom_key_lock(handle);
    } else if (strcmp(argv[1], "gain-definition") == 0) {
        eizo_dbg_dump_gain_definition(handle);
    } else if (strcmp(argv[1], "edid") == 0) {
        eizo_dbg_dump_edid(handle);
    } else if (strcmp(argv[1], "debug") == 0) {
        eizo_set_debug_mode(handle, EIZO_DEBUG_MODE_ENABLED);
    } else if (strcmp(argv[1], "identify") == 0) {
        eizo_set_osd_indicator(handle, EIZO_OSD_INDICATOR_VISIBLE);
        sleep(5);
        eizo_set_osd_indicator(handle, EIZO_OSD_INDICATOR_HIDDEN);
    } else {
        fprintf(stderr, "Unknown option \"%s\"\n", argv[1]);
    }

    eizo_close(handle);
    return EXIT_SUCCESS;
}

