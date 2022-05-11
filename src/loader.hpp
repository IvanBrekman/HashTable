//
// Created by IvanBrekman on 01.05.2022
//

#ifndef LOADERHPP
#define LOADERHPP

#include "table/hash_table.hpp"

struct LoadContext {
    int inserts = poisons::UNINITIALIZED_INT;
    int finds   = poisons::UNINITIALIZED_INT;
};

char** split(char* string, int* size, char sep=' ');

LoadContext* load_strings_to_table(HashTable* table, const char* filename, int force_insert);

#endif // LOADERHPP
