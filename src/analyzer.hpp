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
    ull (* func) (char* item) = (ull (*) (char*)) poisons::UNINITIALIZED_PTR;
    const char* func_name     = (const char*)     poisons::UNINITIALIZED_PTR;
};

struct CollisionData {
    int*   data = (int*)  poisons::UNINITIALIZED_PTR;
    double coef = (double)poisons::UNINITIALIZED_INT;
};

// ==================== Prototypes ====================
char* random_word(char* word, int len);

ull constant_hash    (char* string);
ull first_letter_hash(char* string);
ull symbol_sum_hash  (char* string);
ull string_len_hash  (char* string);
ull roll_hash        (char* string);
ull crc32_hash       (char* string);
ull asm_crc32_hash   (char* string);

CollisionData* get_collision_info (HashTable* table);
int            test_table_speed   (const char* filename, int repeats=1, double fi_coef=0);
int            test_func_collision(const char* filename, const HashFunc* hash, const char* save_file, int save_from, int save_to);
// ====================================================

// ==================== CONSTS ====================
const ull P_VALUE = 257;

const HashFunc ALL_HASH_FUNCS[] = {
    { constant_hash,     "Constant hash"     },
    { first_letter_hash, "First symbol hash" },
    { symbol_sum_hash,   "Symbols sum hash"  },
    { string_len_hash,   "String len hash"   },
    { roll_hash,         "Roll hash"         },
    { crc32_hash,        "Crc32 hash"        },
    { asm_crc32_hash,    "Assembler crc32"   }
};
// ================================================

#endif // ANALYZERHPP
