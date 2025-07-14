#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;

enum {
    BF_CELL_COUNT = UINT16_MAX,
};

typedef struct {
    u8 cells[BF_CELL_COUNT];
    u16 idx;
}bf_state;

void bracket_skip_back(FILE* f) {
    u16 cur_depth = 0;
    char c = 0;
    while (1) {
        fseek(f, -2, SEEK_CUR);
        fread(&c, 1, 1, f);
        if (c == ']') {
            cur_depth++;
        }
        if (c == '[') {
            if (cur_depth == 0) {
                break;
            }
            cur_depth--;
        }
    }
}
void bracket_skip_forward(FILE* f) {
    u16 cur_depth = 0;
    char c = 0;
    while (1) {
        fread(&c, 1, 1, f);
        if (c == '[') {
            cur_depth++;
        }
        if (c == ']') {
            if (cur_depth == 0) {
                break;
            }
            cur_depth--;
        }
    }
}

int main(int argc, char** argv) {
    if (argc == 1) {
        printf("No input file provided!\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    bf_state state = {0};
    char c = 0;
    while (fread(&c, 1, 1, f) > 0) {
        switch (c) {
        case '>':
            // We allow overflows here.
            state.idx++;
            break;
        case '<':
            // We allow underflows here.
            state.idx--;
            break;
        case '-':
            state.cells[state.idx]--;
            break;
        case '+':
            state.cells[state.idx]++;
            break;
        case '.':
            printf("%c", state.cells[state.idx]);
            break;
        case ',':
        {
            int input = getchar();
            if (input != EOF) {
                state.cells[state.idx] = input;
            }
            break;
        }
        case '[':
            if (state.cells[state.idx] == 0) {
                bracket_skip_forward(f);
            }
            break;
        case ']':
            if (state.cells[state.idx] != 0) {
                bracket_skip_back(f);
            }
            break;
        default:
            break;
        }
    }

    return 0;
}
