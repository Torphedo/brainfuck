#include <stdlib.h>
#include <stdio.h>

#include <common/file.h>
#include <common/logging.h>
#include <common/vfile.h>

#include "bf.h"

enum {
    BF_CELL_COUNT = UINT16_MAX,
    // 1 byte opcode + 2 bytes for jump destination on loop ends
    BF_MAX_OPCODE_SIZE = 3,
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

u8* compile_bf_bytecode(const u8* bf_buf, u32 size, u32* bytecode_size_out) {
    const u32 bytecode_buf_size = size * BF_MAX_OPCODE_SIZE;
    u8* bytecode_buf = calloc(1, bytecode_buf_size);
    if (!bytecode_buf) {
        return NULL;
    }

    vfile vf = vfile_open(bytecode_buf, bytecode_buf_size);
    u32 bytecode_pos = 0;
    for (u32 i = 0; i < size; i++) {
        const char c = bf_buf[i];
        const u8 opcode = bf_char_to_opcode(c);
        if (opcode == BF_INVALID) {
            continue; // Invalid characters are probably comments
        }

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

        const u16 cur_pos = vf.pos;
        const u16 target_pos = pos;
        s16* cur_ptr = (s16*)(vf.ptr + cur_pos);
        s16* target_ptr = (s16*)(vf.ptr + target_pos);

        *cur_ptr = target_pos + sizeof(u16);
        *target_ptr = cur_pos + sizeof(u16);
    }

    *bytecode_size_out = vf.size;
    return bytecode_buf;
}

u8* load_bf_bytecode(const char* path, u32* bytecode_size_out) {
    const s64 size = file_size(path);
    const u32 min_size = sizeof(bf_header) + 1; // Header + an opcode
    if (size < min_size) {
        LOG_MSG(error, "'%s' is too small to be BF bytecode (only %d bytes, should be at least %d)\n", size, min_size);
        return NULL;
    }

    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }

    u8* buf = NULL;
    bf_header header = {0};
    fread(&header, sizeof(header), 1, f);
    if (header.magic != BF_MAGIC) {
        goto exit;
    }

    if (header.version > BF_LATEST_VERSION) {
        LOG_MSG(error, "BF bytecode file is too new (v%d), I only support up to v%d!\n", header.version, BF_LATEST_VERSION);
        goto exit;
    }

    const u32 size_left = size - sizeof(bf_header);
    if (header.size > size_left) {
        LOG_MSG(error, "Header says there's %d bytes of bytecode, but there's only %d bytes of room in the file!\n", header.size, size_left);
        goto exit;
    }

    buf = calloc(1, header.size);
    if (buf) {
        fread(buf, header.size, 1, f);
        *bytecode_size_out = header.size;
    }

exit:
    fclose(f);
    return buf;
}

bool save_bf_bytecode(const char* path, u8* buf, u32 size) {
    const bf_header header = {
        .magic = BF_MAGIC,
        .version = BF_LATEST_VERSION,
        .size = size,
    };
    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }
    fwrite(&header, sizeof(header), 1, f);
    fwrite(buf, size, 1, f);
    fclose(f);

    return true;
}

void exec_bf_bytecode(const u8* buf, u32 size) {
    bf_state state = {0};
    u32 bytecode_pos = 0;
    while (bytecode_pos < size) {
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
        case BF_LOOP_END:
        case BF_LOOP_START: {
            const s16 old_pos = bytecode_pos;
            s16 new_pos = *(s16*)&buf[bytecode_pos];
            if (state.cells[state.idx] == 0) {
                if (opcode == BF_LOOP_END) {
                    new_pos = old_pos + 2;
                }
            } else if (opcode == BF_LOOP_START) {
                new_pos = old_pos + 2;
            }

            bytecode_pos = new_pos;
            break;
        }
        default:
            LOG_MSG(warning, "Invalid opcode 0x%x\n", opcode);
            break;
        }
    }
}

bool exec_bf_source(const u8* buf, u32 size) {
    u32 bytecode_buf_size = 0;
    u8* bytecode_buf = compile_bf_bytecode(buf, size, &bytecode_buf_size);
    if (!bytecode_buf) {
        return false;
    }

    exec_bf_bytecode(bytecode_buf, bytecode_buf_size);
    free(bytecode_buf);
    return true;
}
