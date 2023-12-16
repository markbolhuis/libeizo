#include <stdio.h>
#include <stdlib.h>

#include "eizo/eizo.h"
#include "eizo/debug.h"

int
main(int argc, const char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./eizoctl <hidraw-file>\n");
        return EXIT_FAILURE;
    }

    eizo_handle_t handle = eizo_open_hidraw(argv[1]);
    if (!handle) {
        return EXIT_FAILURE;
    }

    eizo_dbg_dump_secondary_descriptor(handle);
    printf("---------\n");
    eizo_dbg_dump_ff300009(handle);
    printf("---------\n");
    eizo_dbg_poll(handle);

    eizo_close(handle);
    return EXIT_SUCCESS;
}

