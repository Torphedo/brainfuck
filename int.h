#ifndef INT_H
#define INT_H
#ifdef __cplusplus
extern "C" {
#endif
/// @file int.h

#include <stdint.h>

/// Shorthand for unsigned 8-bit integer
typedef uint8_t u8;
/// Shorthand for unsigned 16-bit integer
typedef uint16_t u16;
/// Shorthand for unsigned 32-bit integer
typedef uint32_t u32;
/// Shorthand for unsigned 64-bit integer
typedef uint64_t u64;

/// 16-bit character type for reading wchar_t strings built on Windows
typedef u16 c16;

/// Shorthand for signed 8-bit integer
typedef int8_t s8;
/// Shorthand for signed 16-bit integer
typedef int16_t s16;
/// Shorthand for signed 32-bit integer
typedef int32_t s32;
/// Shorthand for signed 64-bit integer
typedef int64_t s64;

/// Round a number up to any boundary
#define ALIGN_UP(x, bound) ((x) + ((bound) - ((x) % (bound))))

// sys/param.h defines these on some platforms, (included in platform.h)
#ifndef MAX
/// Return the larger of 2 values
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
/// Return the smaller of 2 values
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
/// Returns low or high bound if @p val is out of bounds, otherwise return @p val
#define CLAMP(low, val, high) (((val) < (low)) ? (low) : MIN((val), (high)))

/// Can only be used on arrays with compile-time known sizes
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*(arr)))

#ifdef __cplusplus
}
#endif
#endif // INT_H
