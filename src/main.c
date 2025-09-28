#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    const u32 cur_arg = 2;
    bool dump = false;
    const char* dump_path = NULL;
    for (u32 i = cur_arg; i < argc; i++) {
        if (dump && !dump_path) {
            dump_path = argv[i];
        }
        if (strcmp(argv[i], "--dump") == 0) {
            dump = true;
        }
    }

    if (!file_exists(path)) {
        LOG_MSG(error, "Input file '%s' doesn't exist!\n");
        return EXIT_FAILURE;
    }

    // Try to load as bytecode first
    u32 source_size = file_size(path);
    u32 bytecode_buf_size = 0;
    u8* bytecode_buf = load_bf_bytecode(path, &bytecode_buf_size);

    if (!bytecode_buf) {
        // It must be source code, compile it
        u8* bf_buf = file_load(path);
        if (!bf_buf) {
            return EXIT_FAILURE;
        }
        bytecode_buf = compile_bf_bytecode(bf_buf, source_size, &bytecode_buf_size);
        free(bf_buf);
    }

    if (!bytecode_buf) {
        return EXIT_FAILURE;
    }

    if (dump && dump_path) {
        save_bf_bytecode(dump_path, bytecode_buf, bytecode_buf_size);
    }
    exec_bf_bytecode(bytecode_buf, bytecode_buf_size);
    free(bytecode_buf);

    return EXIT_SUCCESS;
}
