#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "eizo/handle.h"
#include "eizo/debug.h"

void
print_help()
{
    printf("Usage: ./eizoctl <option> <hidraw>\n");
    printf("\n");
    printf("Options:\n");
    printf("\tpoll            - Poll the monitor for events.\n");
    printf("\tdescriptor      - Read the eizo specific HID report descriptor.\n");
    printf("\tkey-pairs       - Read the unknown key value pairs behind usage ff300009.\n");
    printf("\tread-all        - Read all values for every usage.\n");
    printf("\teeprom          - Read the contents of the internal EEPROM.\n");
    printf("\tcustom-key-lock - Read the available custom key locks, and the currently used one.\n");
    printf("\thelp            - Show this help message.\n");
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

    if (argc < 3) {
        return EXIT_FAILURE;
    }

    eizo_handle_t handle = nullptr;
    enum eizo_result res = eizo_open_hidraw(argv[2], &handle);
    if (res < EIZO_SUCCESS || handle == nullptr) {
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
    } else {
        fprintf(stderr, "Unknown option \"%s\"\n", argv[1]);
    }

    eizo_close(handle);
    return EXIT_SUCCESS;
}

