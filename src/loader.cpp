//
// Created by IvanBrekman on 01.05.2022
//

#include <cstring>
#include <cctype>

#include "config.hpp"

#include "loader.hpp"

LoadContext* load_strings_to_table(HashTable* table, const char* filename, int force_insert) {
    ASSERT_OK_HASHTABLE(table,     "Check before load strings_to_table func", nullptr);
    ASSERT_IF(VALID_PTR(filename), "Invalid file ptr",                        nullptr);

    char* data    = get_raw_text(filename);

    char* storage = NEW_PTR(char, STORAGE_SIZE * ALLIGENCE);
    //char* storage = (char*) aligned_alloc(ALLIGENCE, STORAGE_SIZE * ALLIGENCE * sizeof(char));
    
    int   st_ind  = 0;

    LoadContext* ctx = NEW_PTR(LoadContext, 1);
    *ctx = { 0, 0, storage };

    for (int i = 0; data[i] != '\0'; i++) {
        if (isspace(data[i])) {
            data[i] = '\0';
        } else {
            char* word  = data + i;
            int   start = i;

            while (!isspace(data[i]) && data[i] != '\0') i++;
            data[i] = '\0';

            ASSERT_IF(st_ind <= STORAGE_SIZE, "There isn't enough space in storage. Increase STORAGE_SIZE constant", nullptr);

            memcpy(storage + st_ind * ALLIGENCE, word, i - start);
            word = storage + st_ind * ALLIGENCE;
            st_ind += (i - start) / 32 + 1;

            if (force_insert || table_find(table, word) == NOT_FOUND) {
                table_add(table, word);
                ctx->inserts++;
            }

            ctx->finds += (!force_insert);
        }
    }

    FREE_PTR(data, char);

    ASSERT_OK_HASHTABLE(table, "Check load to table", nullptr);

    return ctx;
}
