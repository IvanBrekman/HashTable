//
// Created by IvanBrekman on 03.05.2022
//

#include <cstring>
#include <ctime>

#define  ADD_CRC32

#include "config.hpp"
#include "loader.hpp"

#include "analyzer.hpp"

char* random_word(char* word, int len) {
    ASSERT_IF(VALID_PTR(word), "Invalid word ptr", nullptr);
    ASSERT_IF(len > 0, "Incorrect len value. Should be (> 0)", nullptr);

    for (int i = 0; i < len; i++) {
        word[i] = rand() % ('z' - 'a' + 1) + 'a';
    }

    return word;
}

ull constant_hash(char* string) {
    return 1;
}

ull first_letter_hash(char* string) {
    return *string;
}

ull symbol_sum_hash(char* string) {
    ull sum = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        sum += string[i];
    }

    return sum;
}

ull string_len_hash(char* string) {
    return strlen(string);
}

ull roll_hash(char* string) {
    ull hash = string[0];
    while (*string) {
        hash = ((hash >> 1) | (hash << 63)) ^ *(string++);
    }

    return hash;
}

ull crc32_hash(char* string) {
    return (ull) crc32(string, (DWORD)strlen(string));
}

ull crc32_hash_asm(char* string) {
    ull hash = 0;

    __asm__(
        ".intel_syntax noprefix     \n\t"

        "mov rcx, 4                 \n\t"
        "xor %[ret_val], %[ret_val] \n\t"

        "calc_hash:                 \n\t"
            "mov rax, [%[arg_val]]  \n\t"

            "crc32 %[ret_val], rax  \n\t"
            "add %[arg_val], 8      \n\t"
            "loop calc_hash         \n\t"

        ".att_syntax prefix         \n\t"

        : [ret_val]"=r"(hash)
        : [arg_val]"r"(string)
        : "%rax", "%rcx"
    );

    return hash;
}

CollisionData* get_collision_info(HashTable* table) {
    ASSERT_OK_HASHTABLE(table, "Check before get_collision_info func", 0);

    CollisionData* data = NEW_PTR(CollisionData, 1);
    data->data          = NEW_PTR(int, table->capacity);

    int elems = 0;
    int lists = 0;

    for (int i = 0; i < table->capacity; i++) {
        List* lst = table->data + i;

        data->data[i] = lst->size;

        elems += lst->size;
        lists += !list_empty(lst);
    }

    data->coef = (double)elems / lists;

    return data;
}

int test_table_speed(const char* filename, int repeats, double fi_coef) {
    ASSERT_IF(VALID_PTR(filename), "Invalid filename ptr", 0);

    clock_t sum_time = 0;

    LoadContext* context = nullptr;

    for (int i = 0; i < repeats; i++) {
        HashTable* table = CREATE_TABLE(table, crc32_hash_asm, validate::MEDIUM_VALIDATE, CAPACITY_VALUES[1]);

        clock_t start_time = clock();
                context    = load_strings_to_table(table, filename, 0);
        clock_t end_time   = clock();

        sum_time += end_time - start_time;

        if ((context->inserts * fi_coef) > context->finds) {
            int need_inserts = (int) (context->inserts * fi_coef) - context->finds;

            char* word = (char*) calloc(MAX_RANDOM_WORD_LEN, sizeof(char));
            for ( ; need_inserts > 0; need_inserts--) {
                word = random_word(word, rand() % MAX_RANDOM_WORD_LEN + 1);

                start_time = clock();
                table_find(table, word);
                end_time   = clock();

                context->finds++;
                sum_time += end_time - start_time;
            }
            FREE_PTR(word, char);
        }

        table_dtor(table);

        FREE_PTR(context->storage, char);
        if (i + 1 < repeats) FREE_PTR(context, LoadContext);
    }
    
    printf("=============== Speed test ===============\n");
    printf("repeats:  %d\n\n", repeats);
    printf("time avg: %lf sec\n\n", ((double)sum_time / repeats) / CLOCKS_PER_SEC);
    printf("finds:    %d\n",  context->finds);
    printf("inserts:  %d\n",  context->inserts);
    printf("fi coef:  %lf\n", (double)context->finds / context->inserts);
    printf("==========================================\n\n");

    FREE_PTR(context, LoadContext);

    return 1;
}

int test_func_collision(const char* filename, const HashFunc* hash, const char* save_file, int save_from, int save_to) {
    ASSERT_IF(VALID_PTR(filename), "Invalid filename ptr", 0);
    ASSERT_IF(VALID_PTR(hash),     "Invalid hash ptr",     0);

    HashTable*   table   = CREATE_TABLE(table, hash->func, validate::WEAK_VALIDATE, CAPACITY_VALUES[1]);
    LoadContext* context = load_strings_to_table(table, filename, 1);

    CollisionData* data  = get_collision_info(table);
    
    FILE* dest = open_file(save_file, "a");

    save_to = (save_to == OLD_CAPACITY) ? table->capacity : save_to;

    fprintf(dest, "%s;", hash->func_name);
    for (int i = save_from; i < save_to; i++) {
        List* lst = table->data + i;

        fprintf(dest, "%d", lst->size);
        if (i + 1 < save_to) fprintf(dest, " ");
    }
    fprintf(dest, ";%.3lf\n", data->coef);

    printf("=============== Collision test ===============\n");
    printf("func name:     '%s'\n",   hash->func_name);
    printf("collision coef: %lf\n\n", data->coef);
    printf("finds:          %d\n",  context->finds);
    printf("inserts:        %d\n",  context->inserts);
    printf("fi coef:        %lf\n", (double)context->finds / context->inserts);
    printf("==============================================\n\n");

    FREE_PTR(data->data,       int);
    FREE_PTR(context->storage, char);

    FREE_PTR(data,    CollisionData);
    FREE_PTR(context, LoadContext);

    table_dtor(table);
    close_file(dest);

    return 1;
}
