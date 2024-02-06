#include <stdio.h>
#include <stdlib.h>

#include "eizo/handle.h"
#include "eizo/debug.h"

int
main(int argc, const char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: ./eizoctl <hidraw-file>\n");
        return EXIT_FAILURE;
    }

    eizo_handle_t handle = nullptr;
    enum eizo_result res = eizo_open_hidraw(argv[1], &handle);
    if (res < EIZO_SUCCESS || handle == nullptr) {
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

