//
// Created by IvanBrekman on 01.05.2022
//

#include <cstring>

#include "config.hpp"
#include "libs/file_funcs.hpp"

#include "loader.hpp"

LoadContext* load_strings_to_table(HashTable* table, const char* filename, int force_insert) {
    ASSERT_OK_HASHTABLE(table,     "Check before load strings_to_table func", nullptr);
    ASSERT_IF(VALID_PTR(filename), "Invalid file ptr",                        nullptr);

    Text data = get_text_from_file(filename, 1, 1);

    LoadContext* ctx = NEW_PTR(LoadContext, 1);
    *ctx = { 0, 0 };

    for (size_t i = 0; i < data.lines; i++) {
        int    size = 0;
        char** splitted_words = split(data.text[i].ptr, &size);

        for (int  i = 0; i < size; i++) {
            if (force_insert || table_find(table, splitted_words[i]) == NOT_FOUND) {
                char* word_cpy = strdup(splitted_words[i]);

                table_add(table, word_cpy);
                FREE_PTR(splitted_words[i], char);

                ctx->inserts++;
            }
            ctx->finds += (!force_insert);
        }

        FREE_PTR(splitted_words, char*);
    }
    free_text(&data);

    ASSERT_OK_HASHTABLE(table, "Check load to table", nullptr);

    return ctx;
}
