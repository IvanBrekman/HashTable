//
// Created by IvanBrekman on 01.05.2022
//

#include <cstring>

#include "config.hpp"
#include "libs/file_funcs.hpp"

#include "loader.hpp"

char** split(char* string, int* size, char sep) {
    ASSERT_IF(VALID_PTR(string), "Invalid string ptr", nullptr);
    ASSERT_IF(VALID_PTR(size),   "Invalid size ptr",   nullptr);

    int len = (int)strlen(string);

    int  buf_size = 0;
    char** buffer = NEW_PTR(char*, len);

    int start = 0, end = 0;
    for (int i = 0; ; i++, start++, end++) {
        if (string[i] == sep || string[i] == '\0') {
            if (start == end) {        
                if (string[i] == '\0') break;
                continue;
            }

            char* word = NEW_PTR(char, end - start + 1);
            memcpy(word, string + start, end - start);

            buffer[buf_size++] = word;

            start = end;
        } else {
            start--;
        }

        if (string[i] == '\0') break;
    }

    *size = buf_size;
    return buffer;
}

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
