//
// Created by IvanBrekman on 01.05.2022
//

#ifndef LOADERHPP
#define LOADERHPP

#include "table/hash_table.hpp"

const int NBYTES       = 8;
const int ALLIGENCE    = 32;
const int STORAGE_SIZE = 30000;

struct LoadContext {
    int inserts = poisons::UNINITIALIZED_INT;
    int finds   = poisons::UNINITIALIZED_INT;

    char* storage = (char*) poisons::UNINITIALIZED_PTR;
};

LoadContext* load_strings_to_table(HashTable* table, const char* filename, int force_insert);

#endif // LOADERHPP
