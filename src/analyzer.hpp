//
// Created by IvanBrekman on 03.05.2022
//

#ifndef ANALYZERHPP
#define ANALYZERHPP

#include "table/hash_table.hpp"

#define CLEAR_SAVE_FILE(filename) {         \
    FILE* fp = open_file(filename, "w");    \
    close_file(fp);                         \
}

struct HashFunc {
    unsigned long long (* func) (item_t* item) = (unsigned long long (*) (item_t*)) poisons::UNINITIALIZED_PTR;
    const char*           func_name            = (const char*) poisons::UNINITIALIZED_PTR;
};

struct CollisionData {
    int*   data = (int*)  poisons::UNINITIALIZED_PTR;
    double coef = (double)poisons::UNINITIALIZED_INT;
};

// ==================== Prototypes ====================
int print_str        (item_t* item);
int cmp_str          (item_t* item1, item_t* item2);
int avx_len32_cmp_str(item_t* item1, item_t* item2);
int del_str          (item_t* item);

char* random_word(char* word, int len);

unsigned long long constant_hash    (item_t* item);
unsigned long long first_letter_hash(item_t* item);
unsigned long long symbol_sum_hash  (item_t* item);
unsigned long long string_len_hash  (item_t* item);
unsigned long long roll_hash        (item_t* item);
unsigned long long crc32_hash       (item_t* item);

unsigned long long asm_len32_crc32_hash(item_t* item);

CollisionData* get_collision_info (HashTable* table);
int            test_table_speed   (const char* filename, int repeats=1, double fi_coef=0);
int            test_func_collision(const char* filename, const HashFunc* hash, const char* save_file, int save_from, int save_to);
// ====================================================

// ==================== CONSTS ====================
const unsigned long long P_VALUE = 257;

const HashFunc ALL_HASH_FUNCS[]  = {
    { constant_hash,        "Constant hash"     },
    { first_letter_hash,    "First symbol hash" },
    { symbol_sum_hash,      "Symbols sum hash"  },
    { string_len_hash,      "String len hash"   },
    { roll_hash,            "Roll hash"         },
    { crc32_hash,           "Crc32 hash"        }
};
// ================================================

#endif // ANALYZERHPP
