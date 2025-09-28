#pragma once
/// @file bf.h
/// @author Torphedo
/// @brief Brainfuck implementation

#include <stdbool.h>
#include <assert.h>
#include <common/int.h>
#include <common/file.h>

typedef struct {
    u32 magic;
    u32 size;
    u8 version; // 1
    u8 reserved[7]; // In case we need more fields later
}bf_header;
static_assert(sizeof(bf_header) == 0x10, "BFC header size is wrong!");

enum {
    BF_MAGIC = MAGIC('B', 'F', 'B', ' '),
    BF_LATEST_VERSION = 1,
};

/// @brief Compile Brainfuck source code into bytecode
///
/// @param bf_buf Buffer of source code text
/// @param size The size of the source code buffer
/// @param bytecode_size_out Output to store the size of the bytecode
/// @return Pointer to bytecode buffer, or NULL on error
u8* compile_bf_bytecode(const u8* bf_buf, u32 size, u32* bytecode_size_out);

u8* load_bf_bytecode(const char* path, u32* bytecode_size_out);

bool save_bf_bytecode(const char* path, u8* buf, u32 size);

/// @brief Execute compiled Brainfuck bytecode
///
/// @param buf Bytecode buffer
/// @param size Size of the buffer
void exec_bf_bytecode(const u8* buf, u32 size);

/// @brief Execute Brainfuck source code
///
/// This compiles bytecode for you under the hood.
bool exec_bf_source(const u8* buf, u32 size);
