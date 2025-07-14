#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "int.h"
#include "file.h"
#include "logging.h"
#include "vfile.h"

enum {
    BF_CELL_COUNT = UINT16_MAX,
};

typedef struct {
    u8 cells[BF_CELL_COUNT];
    u16 idx;
}bf_state;

typedef enum {
    BF_INVALID,
    BF_POS_INC,
    BF_POS_DEC,
    BF_CELL_INC,
    BF_CELL_DEC,
    BF_PRINT,
    BF_INPUT,
    BF_LOOP_START,
    BF_LOOP_END,
}bf_opcode;

// 1 byte opcode + 2 bytes offset in case of loop
#define BF_MAX_OPCODE_SIZE (3)

u16 bracket_skip_forward(u8* buf, u32 pos, u32 bufsize) {
    u16 cur_depth = 0;
    u8 c = 0;
    while (pos < bufsize) {
        c = buf[pos++];
        if (c == BF_LOOP_START) {
            cur_depth++;
        }
        if (c == BF_LOOP_END) {
            if (cur_depth == 0) {
                break;
            }
            cur_depth--;
        }
    }

    return pos;
}

bf_opcode bf_char_to_opcode(char c) {
    switch (c) {
    case '>':
        return BF_POS_INC;
    case '<':
        return BF_POS_DEC;
    case '-':
        return BF_CELL_DEC;
    case '+':
        return BF_CELL_INC;
    case '.':
        return BF_PRINT;
    case ',':
        return BF_INPUT;
    case '[':
        return BF_LOOP_START;
    case ']':
        return BF_LOOP_END;
    default:
        return BF_INVALID;
    }
}

void exec_bf_bytecode(u8* buf, u32 size) {
    bf_state state = {0};
    u32 bytecode_pos = 0;
    while (bytecode_pos < size + 1) {
        const bf_opcode opcode = buf[bytecode_pos++];
        switch (opcode) {
        case BF_POS_INC:
            state.idx++;
            break;
        case BF_POS_DEC:
            state.idx--;
            break;
        case BF_CELL_DEC:
            state.cells[state.idx]--;
            break;
        case BF_CELL_INC:
            state.cells[state.idx]++;
            break;
        case BF_PRINT:
            printf("%c", state.cells[state.idx]);
            break;
        case BF_INPUT: {
            const int input = getchar();
            if (input != EOF) {
                state.cells[state.idx] = (input & 0xFF);
            }
            break;
        }
        case BF_LOOP_START: {
            if (state.cells[state.idx] != 0) {
                bytecode_pos += 2;
                break;
            }
            const s16 old_pos = bytecode_pos;
            const s16 new_pos = *(s16*)&buf[bytecode_pos];
            bytecode_pos = new_pos;

            break;
        }
        case BF_LOOP_END: {
            if (state.cells[state.idx] == 0) {
                bytecode_pos += 2;
                break;
            }
            const s16 old_pos = bytecode_pos;
            const s16 new_pos = *(s16*)&buf[bytecode_pos];
            bytecode_pos = new_pos;

            break;
        }
        default:
            LOG_MSG(warning, "Invalid opcode 0x%x\n", opcode);
            break;
        }
    }
}

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("No input file provided!\n");
        return EXIT_FAILURE;
    }

    const char* path = argv[1];
    if (!file_exists(path)) {
        return EXIT_FAILURE;
    }

    const u32 size = file_size(path);
    u8* bf_buf = file_load(path);
    const u32 bytecode_buf_size = size * BF_MAX_OPCODE_SIZE;
    u8* bytecode_buf = calloc(1, bytecode_buf_size);

    bf_state state = {0};
    int result = EXIT_FAILURE;
    if (!bf_buf || !bytecode_buf) {
        goto exit;
    }

    vfile vf = vfile_open(bytecode_buf, bytecode_buf_size);
    u32 bytecode_pos = 0;
    for (u32 i = 0; i < size; i++) {
        const char c = bf_buf[i];
        const u8 opcode = bf_char_to_opcode(c);

        VFILE_WRITE(u8, &vf, opcode);
        if (opcode == BF_LOOP_START || opcode == BF_LOOP_END) {
            vfile_seek(&vf, sizeof(u16));
        }
    }
    vf.size = vf.pos;
    vf.pos = 0;

    while (!vfile_eof(vf)) {
        const bf_opcode opcode = VFILE_READ(u8, &vf);

        if (opcode != BF_LOOP_START) {
            continue;
        }
        const u16 pos = bracket_skip_forward(vf.ptr, vf.pos, vf.size);
        if (pos == vf.size) {
            LOG_MSG(error, "Couldn't find matching loop bracket!\n");
            vfile_seek(&vf, sizeof(u16));
            continue;
        }

        LOG_MSG(info, "Bracket @ 0x%x matches one @ 0x%x!\n", vf.pos - 1, pos);
        const s16 offset = (s16)pos - (s16)vf.pos;
        const u16 cur_pos = vf.pos;
        const u16 target_pos = pos;

        s16* cur_ptr = (s16*)(vf.ptr + cur_pos);
        s16* target_ptr = (s16*)(vf.ptr + target_pos);

        *cur_ptr = target_pos + sizeof(u16);
        *target_ptr = cur_pos + sizeof(u16);
        printf("source points -> 0x%x, target points -> 0x%x\n", *cur_ptr, *target_ptr);
    }

    exec_bf_bytecode(bytecode_buf, vf.size);

exit:
    result = EXIT_SUCCESS;
exit_fail:
    free(bf_buf);
    free(bytecode_buf);

    return result;
}
