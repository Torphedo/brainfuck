#include <stdio.h>
#include <stdlib.h>

#include <common/int.h>
#include <common/logging.h>
#include <common/file.h>

#include "bf.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("No input file provided!\n");
        return EXIT_FAILURE;
    }

    const char* path = argv[1];
    const u32 size = file_size(path);
    u8* bf_buf = file_load(path);
    if (!bf_buf) {
        return EXIT_FAILURE;
    }

    if (!exec_bf_source(bf_buf, size)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
