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

    char* data = get_raw_text(filename);

    LoadContext* ctx = NEW_PTR(LoadContext, 1);
    *ctx = { 0, 0, data };

    for (int i = 0; data[i] != '\0'; i++) {
        if (isspace(data[i])) {
            data[i] = '\0';
        } else {
            char* word = data + i;

            while (!isspace(data[i]) && data[i] != '\0') i++;
            data[i] = '\0';

            if (force_insert || table_find(table, word) == NOT_FOUND) {
                table_add(table, word);
                ctx->inserts++;
            }

            ctx->finds += (!force_insert);
        }
    }

    ASSERT_OK_HASHTABLE(table, "Check load to table", nullptr);

    return ctx;
}
