#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uintmax_t usize;
typedef intmax_t  isize;

#define ACTION(name, fn) \
    if (!strcmp(argv[1], name)) { exit(fn(argc - 1, argv + 1)); }
#define UNUSED(x) (void)(x)

#ifdef _DEBUG
#define _PRINT_DEBUGGING fprintf(stderr, " [%s:%d]", __FILE__, __LINE__);
#define DBG(...)                                         \
    do {                                                 \
        fprintf(stderr, "[%s:%d] ", __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                    \
        fflush(stderr);                                  \
    } while (0)
#define STOP                                             \
    do {                                                 \
        fprintf(stderr, "PRESS ENTER TO CONTINUE...\n"); \
        getchar();                                       \
    } while (0)
#define unreachable PANIC("unreachable code was reached at function %s", __func__);
#else
#define DBG(...)
#define _PRINT_DEBUGGING
#define STOP
#define unreachable                                                 \
    PANIC("unreachable code was reached at function %s", __func__); \
    std::unreachable();
#endif

#define PANIC_CUSTOM(x, e)          \
    do {                            \
        fprintf(stderr, "ERROR: "); \
        x;                          \
        _PRINT_DEBUGGING;           \
        fprintf(stderr, "\n");      \
        e;                          \
    } while (0)
#define PANIC(...) PANIC_CUSTOM(fprintf(stderr, __VA_ARGS__), exit(1))
#define PANIC_FORCABLE(...)                                                                                      \
    PANIC_CUSTOM(                                                                                                \
        fprintf(stderr, __VA_ARGS__),                                                                            \
        if ([&]() -> bool {                                                                                      \
                char* env_force = getenv("FORCE");                                                               \
                return !(env_force &&                                                                            \
                         (!strcmp(env_force, "1") || !strcmp(env_force, "true") || !strcmp(env_force, "TRUE"))); \
            }()) {                                                                                               \
            fprintf(stderr, "(Use FORCE=true to bypass this error)\n");                                          \
            exit(1);                                                                                             \
        })
#define PANIC_FROM_ERRNO(...) \
    PANIC_CUSTOM(fprintf(stderr, __VA_ARGS__); fprintf(stderr, ": %s (errno=%d)", strerror(errno), errno), exit(1))
#define API_ASSERT(x) \
    if (!(x))         \
    PANIC_CUSTOM(fprintf(stderr, "API Abuse found at function %s. Assertion %s failed.\n", __func__, #x), exit(1))
#define todo PANIC("function %s was not yet implemented.", __func__);

#ifdef __cplusplus

#define smalloc(x)                                              \
    ([&]() -> void* {                                           \
        void* ptr = malloc(x);                                  \
        if (!ptr) PANIC("Failed to allocate required memory."); \
        return ptr;                                             \
    }())

#endif

#define NONNULL(x) x
