//
// Created by ivanbrekman on 22.09.2021.
//

#ifndef BASELIB_H
#define BASELIB_H

#include "config.hpp"

#ifndef VALIDATE_LEVEL
    #define VALIDATE_LEVEL 0
#endif

#ifndef EXECUTE_WAITINGS
    #define EXECUTE_WAITINGS 0
#endif

#ifndef LOG_PRINTF
    #define LOG_PRINTF 0
#endif

#ifndef LOG_GRAPH
    #define LOG_GRAPH 0
#endif

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>

#define MAX(arg1, arg2) (arg1) > (arg2) ? (arg1) : (arg2)
#define MIN(arg1, arg2) (arg1) < (arg2) ? (arg1) : (arg2)

#define dbg(code)      do { printf("%s:%d\n", __FILE__, __LINE__); code } while (0)
#define LOCATION(var)  { TYPE, #var, __FILE__, __FUNCTION__, __LINE__ }
#define WAIT_INPUT     do { if (EXECUTE_WAITINGS == 1 && LOG_PRINTF > 0) { printf(BLUE "Press any button...\n" NATURAL); getchar(); } } while(0)

#define COLORED_OUTPUT(str, color, file) IS_TERMINAL(file) ? (color str NATURAL) : str
#define IS_TERMINAL(file)                (file == stdin) || (file == stdout) || (file == stderr)
#define INT_ADDRESS(ptr)                 (int)((char*)(ptr) - (char*)0)
#define NEW_PTR(type, amount)            (type*) calloc_s(amount, sizeof(type))
#define BAD_OUTPUT                       COLORED_OUTPUT("(BAD)", RED, log)

#define IS_DIGIT(symbol)  ('0' <= symbol && symbol <= '9')
#define IS_LATIN(symbol) (('a' <= symbol && symbol <= 'z') || ('A' <= symbol && symbol <= 'Z'))

const double FLOAT_COMPARE_PRESICION = 0.0001;

const int MAX_FILEPATH_SIZE       =  50;
const int MAX_SPRINTF_STRING_SIZE = 500;
const int NOT_FOUND               = -0xFAC5;

void* calloc_s(size_t __nmemb, size_t __size);

FILE* open_file(const char* filename, const char mode[]);
int  close_file(FILE* file);

/*
Default define to ASSERT_OK. Use it to customize macros for each project.
!Note!  To use this macro check that open_file function is defined
        or use another function to open file.

#define ASSERT_OK(obj, type, reason, ret) do {                                      \
    if (VALIDATE_LEVEL >= WEAK_VALIDATE && type ## _error(obj)) {                   \
        type ## _dump(obj, reason);                                                 \
        if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                   \
            LOG_DUMP(obj, reason, type ## _dump);                                   \
        }                                                                           \
        ASSERT_IF(0, "verify failed", ret);                                         \
    } else if (type ## _error(obj)) {                                               \
        errno = type ## _error(obj);                                                \
        return ret;                                                                 \
    }                                                                               \
} while (0)

*/

#define LOG_DUMP(obj, reason, func) do {                                            \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* log = open_file("logs/log.txt", "a");                                 \
        func(obj, reason, log);                                                     \
        close_file(log);                                                            \
    }                                                                               \
} while (0)

#define LOG_DUMP_GRAPH(obj, reason, func) do {                                      \
    if (LOG_GRAPH == 1) {                                                           \
        FILE* gr_log = open_file("logs/log.html", "a");                             \
        func(obj, reason, gr_log);                                                  \
        close_file(gr_log);                                                         \
    }                                                                               \
} while (0)

#define PRINT_WARNING(text) do {                                                    \
    printf(__FILE__ ":%d " ORANGE text NATURAL, __LINE__);                          \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* wlog = open_file("logs/log.txt", "a");                                \
        fprintf(wlog, __FILE__ ":%d " text, __LINE__);                              \
        close_file(wlog);                                                           \
    }                                                                               \
} while (0)

#define APRINT_WARNING(text, args...) do {                                          \
    printf(__FILE__ ":%d " ORANGE text NATURAL, __LINE__, args);                    \
    if (VALIDATE_LEVEL >= HIGHEST_VALIDATE) {                                       \
        FILE* wlog = open_file("logs/log.txt", "a");                                \
        fprintf(wlog, __FILE__ ":%d " text, __LINE__, args);                        \
        close_file(wlog);                                                           \
    }                                                                               \
} while (0)

#define SPR_SYSTEM(format...) do {                                                  \
    char* command = (char*) calloc_s(MAX_SPRINTF_STRING_SIZE, sizeof(char));        \
    sprintf(command, format);                                                       \
                                                                                    \
    system(command);                                                                \
                                                                                    \
    FREE_PTR(command, char);                                                        \
} while (0)

#define SPR_FPUTS(file, format...) do {                                             \
    char* string = (char*) calloc_s(MAX_SPRINTF_STRING_SIZE, sizeof(char));         \
    sprintf(string, format);                                                        \
                                                                                    \
    fputs(string, file);                                                            \
                                                                                    \
    FREE_PTR(string, char);                                                         \
} while (0)

#define FREE_PTR(ptr, type) do {                \
    free((ptr));                                \
    (ptr) = (type*)poisons::FREED_PTR;          \
} while (0)

#define LOGN(level, code) do {                  \
    if (LOG_PRINTF >= level) {                  \
        code                                    \
    }                                           \
} while (0)

#define LOG1(code)  LOGN(1, code)
#define LOG2(code)  LOGN(2, code)

#ifndef NO_CHECKS
    #define VALID_PTR(ptr) !isbadreadptr((const void*)(ptr))

    #define ASSERT_IF(cond, text, ret) do {                                             \
        assert((cond) && text);                                                         \
        if (!(cond)) {                                                                  \
            PRINT_WARNING(text "\n");                                                   \
            errno = -1;                                                                 \
            return ret;                                                                 \
        }                                                                               \
    } while(0)
#else
    #define VALID_PTR(ptr) (                                    \
        ((void*)ptr != nullptr)                           &&    \
        ((void*)ptr != (void*)poisons::UNINITIALIZED_PTR) &&    \
        ((void*)ptr != (void*)poisons::FREED_PTR)               \
    )

    #define ASSERT_IF(cond, text, ret) 
#endif

// Colors----------------------------------------------------------------------
#define BLACK       "\033[1;30m"
#define RED         "\033[1;31m"
#define GREEN       "\033[1;32m"
#define ORANGE      "\033[1;33m"
#define BLUE        "\033[1;34m"
#define PURPLE      "\033[1;35m"
#define CYAN        "\033[1;36m"
#define GRAY        "\033[1;37m"

#define BLACK_UNL   "\033[4;30m"
#define RED_UNL     "\033[4;31m"
#define GREEN_UNL   "\033[4;32m"
#define ORANGE_UNL  "\033[4;33m"
#define BLUE_UNL    "\033[4;34m"
#define PURPLE_UNL  "\033[4;35m"
#define CYAN_UNL    "\033[4;36m"
#define GRAY_UNL    "\033[4;37m"

#define NATURAL     "\033[0m"
// ----------------------------------------------------------------------------

enum validate_level_t {
    NO_VALIDATE      = 0, // No checks in program
    WEAK_VALIDATE    = 1, // Checks only fields with  O(1) complexity
    MEDIUM_VALIDATE  = 2, // Checks filed, which need O(n) complexity
    STRONG_VALIDATE  = 3, // All checks (hash and others)
    HIGHEST_VALIDATE = 4  // Error will write in log file
};

enum poisons {
    UNINITIALIZED_PTR = 0xBAD111,
    UNINITIALIZED_INT = -1 * (0xBAD666),

    FREED_ELEMENT     = -1 * (0xBAD667),
    FREED_PTR         = 0xF2EE
};

#define PRINT_DATE(color) do {                                      \
    char* date = (char*) calloc_s(40, sizeof(char));                \
    printf((color "Time: %s\n" NATURAL), datetime(date));           \
    FREE_PTR(date, char);                                           \
} while (0)

#define FPRINT_DATE(file) do {                                      \
    char* date = (char*) calloc_s(40, sizeof(char));                \
    fprintf(file, "Time: %s\n", datetime(date));                    \
    FREE_PTR(date, char);                                           \
} while (0)

int      file_size(const char* filename);
char* get_raw_text(const char* filename);
char* delete_spaces(      char* string);

int isbadreadptr(const void* ptr);
char* datetime(char* calendar_date);

int is_integer(double number);
int is_number(char* string);
int digits_number(int number, int radix=10);
int   extract_bit(int number, int bit);

char* bin4(int number);
char* to_string(int number);

int print_int_array(int* array, int size, const char* sep=", ", const char* end="\n");

#ifdef ADD_CRC32

typedef unsigned int  DWORD;
typedef unsigned char BYTE;
 
static DWORD crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

DWORD crc32(const void* buf, DWORD size);
DWORD crc32(const void* buf, DWORD size) {
    const BYTE* p   = (const BYTE*) buf;
          DWORD crc = 0xFFFFFFFF;
 
    while (size--) {
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }
 
    return crc ^ ~0U;
}

#endif

#endif // BASELIB_H
