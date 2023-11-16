#include <stdio.h>
#include <stdlib.h>

#include "eizo/eizo.h"

int
main(int argc, const char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./eizoctl <hidraw-file>\n");
        return EXIT_FAILURE;
    }

    eizo_handle_t handle = eizo_open_path(argv[1]);
    if (!handle) {
        return EXIT_FAILURE;
    }

    eizo_dbg_dump_descriptor(handle);

    eizo_close(handle);
    return EXIT_SUCCESS;
}

