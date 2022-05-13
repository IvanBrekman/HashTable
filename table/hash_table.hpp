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

typedef char* item_t;

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
    unsigned long long _lcanary = INIT_CANARY;
    validate_level_t   _vlevel  = validate_level_t::NO_VALIDATE;
    HashInfo*          _info    = (HashInfo*) poisons::UNINITIALIZED_PTR;

    int                (*_pf)   (item_t* item)                 = (int (*) (item_t*))                poisons::UNINITIALIZED_PTR;
    int                (*_cmp)  (item_t* item1, item_t* item2) = (int (*) (item_t*, item_t*))       poisons::UNINITIALIZED_PTR;
    int                (*_del)  (item_t* item)                 = (int (*) (item_t*))                poisons::UNINITIALIZED_PTR;
    unsigned long long (*_hash) (item_t* item)                 = (unsigned long long (*) (item_t*)) poisons::UNINITIALIZED_PTR;

    List*   data        = (List*)poisons::UNINITIALIZED_PTR;
    int     size        = poisons::UNINITIALIZED_INT;
    int     capacity    = poisons::UNINITIALIZED_INT;

    unsigned long long _rcanary = INIT_CANARY;
};

enum hashtable_errors {
    INVALID_TABLE_PTR       = -1,
    INVALID_TABLE_INFO_PTR  = -2,
    INVALID_PRINT_ITEM_PTR  = -3,
    INVALID_CMP_ITEMS_PTR   = -4,
    INVALID_HASH_FUNC_PTR   = -5,
    INVALID_DATA_PTR_       = -6,
    INVALID_LIST            = -7,

    DAMAGED_LCANARY         = -8,
    DAMAGED_RCANARY         = -9,
    FREED_CANARY_VALUE      = -10,

    NOT_ENOUGH_MEMORY_      = -12,
    SIZE_EXCEEDED_CAPACITY_ = -13,

    OK_ = 0,
};

int                 default_print(item_t* item);
int                 default_cmp  (item_t* item1, item_t* item2);
int                 default_del  (item_t* item);
unsigned long long  default_hash (item_t* item);

HashTable* table_ctor(HashInfo*            info                                     = nullptr,
                      int                (*print_func)(item_t* item)                = default_print,
                      int                (*cmp_func)  (item_t* item1, item_t* item2)= default_cmp,
                      int                (*del_func)  (item_t* item)                = default_del,
                      unsigned long long (*hash_func) (item_t* item)                = default_hash,
                      validate_level_t     level                                    = VALIDATE_LEVEL,
                      int                  capacity                                 = CAPACITY_VALUES[0]
                     );
HashTable* table_dtor(HashTable* table);

hashtable_errors table_error     (const HashTable*       table);
const char*      table_error_desc(const hashtable_errors error_code);

int table_add (      HashTable* table, item_t* item);
int table_find(const HashTable* table, item_t* item);

int table_rehash(    HashTable* table, int new_capacity=OLD_CAPACITY);
int get_next_capacity(int capacity);

int table_dump(const HashTable* table, const char* reason, FILE* log=stdout);

#endif // HASHTABLE_H
