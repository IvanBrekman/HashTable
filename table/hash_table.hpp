//
// Created by IvanBrekman on 22.04.2022
//

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "config.hpp"
#include "libs/baselib.hpp"
#include "list.hpp"

#ifndef NO_CHECKS
    #define ASSERT_OK_HASHTABLE(obj, reason, ret) do {                      \
        if (obj->_vlevel >= WEAK_VALIDATE && table_error(obj)) {            \
            table_dump(obj, reason);                                        \
            if (obj->_vlevel >= HIGHEST_VALIDATE) {                         \
                LOG_DUMP(obj, reason, table_dump);                          \
            }                                                               \
            ASSERT_IF(0, "verify failed", ret);                             \
        } else if (table_error(obj)) {                                      \
            errno = table_error(obj);                                       \
            return ret;                                                     \
        }                                                                   \
    } while (0)
#else
    #define ASSERT_OK_HASHTABLE(obj, reason, ret) 
#endif

#define TYPE "char*"
#define CREATE_TABLE(var, args...)                                      \
    0;                                                                  \
    do {                                                                \
        HashInfo* info = NEW_PTR(HashInfo, 1);                          \
        info->type = TYPE;                                              \
        info->name = #var;                                              \
        info->file = __FILE__;                                          \
        info->func = __FUNCTION__;                                      \
        info->line = __LINE__;                                          \
                                                                        \
        var = table_ctor(info, args);                                   \
    } while (0) /* For ';' requirement */

typedef unsigned long long  ull;

const long unsigned INIT_CANARY  = 0x5AFEA2EA; // SAFE AREA
const long unsigned FREE_CANARY  = 0xDEADA2EA; // DEAD AREA
const long unsigned NO_INFO_PTR  = 0x5015F0;   // NO INFO

const double  LOAD_FACTOR        = 0.9f;
const int     CAPACITY_VALUES[]  = { 1009, 5003, 21997, 61757, 99991 };

const int     OLD_CAPACITY       = -1;
const int     LIST_CAPACITY      = 10;

struct HashInfo {
    const char* type = nullptr;
    const char* name = nullptr;
    const char* file = nullptr;
    const char* func = nullptr;
          int   line = 0;
};

struct HashTable {
    ull        _lcanary = INIT_CANARY;
    validate   _vlevel  = validate::NO_VALIDATE;
    HashInfo*  _info    = (HashInfo*) poisons::UNINITIALIZED_PTR;

    ull (*_hash) (char* string) = (ull (*) (char*)) poisons::UNINITIALIZED_PTR;

    List*   data        = (List*)poisons::UNINITIALIZED_PTR;
    int     size        = poisons::UNINITIALIZED_INT;
    int     capacity    = poisons::UNINITIALIZED_INT;

    ull        _rcanary = INIT_CANARY;
};

enum hashtable_errors {
    INVALID_TABLE_PTR       = -1,
    INVALID_TABLE_INFO_PTR  = -2,
    INVALID_HASH_FUNC_PTR   = -3,
    INVALID_DATA_PTR        = -4,
    INVALID_LIST            = -5,

    DAMAGED_LCANARY         = -6,
    DAMAGED_RCANARY         = -7,
    FREED_CANARY_VALUE      = -8,

    NOT_ENOUGH_MEMORY_      = -9,
    SIZE_EXCEEDED_CAPACITY_ = -10,

    OK_ = 0,
};

int print_str   (char* string);
ull default_hash(char* string);

HashTable* table_ctor(HashInfo*   info                      = nullptr,
                      ull       (*hash_func) (char* string) = default_hash,
                      validate    level                     = VALIDATE_LEVEL,
                      int         capacity                  = CAPACITY_VALUES[0]
                     );
HashTable* table_dtor(HashTable* table);

hashtable_errors table_error     (const HashTable*       table);
const char*      table_error_desc(const hashtable_errors error_code);

int table_add (      HashTable* table, char* string);
int table_find(const HashTable* table, char* string);

int table_rehash(    HashTable* table, int new_capacity=OLD_CAPACITY);
int get_next_capacity(int capacity);

int table_dump(const HashTable* table, const char* reason, FILE* log=stdout);

#endif // HASHTABLE_H
