//
// Created by IvanBrekman on 03.05.2022
//

#include <cstring>
#include <ctime>
#include <immintrin.h>

#define  ADD_CRC32

#include "config.hpp"
#include "loader.hpp"

#include "analyzer.hpp"

int print_str(item_t* item) {
    ASSERT_IF(VALID_PTR(*item), "Invalid *item ptr", 0);

    printf("'%s'", *item);

    return 1;
}

int cmp_str(item_t* item1, item_t* item2) {
    ASSERT_IF(VALID_PTR(*item1), "Invalid *item1 ptr", 0);
    ASSERT_IF(VALID_PTR(*item2), "Invalid *item2 ptr", 0);

    return strcmp(*item1, *item2);
}

int strcmp_avx_32len(item_t* item1, item_t* item2) {
    __m256i str1 = _mm256_lddqu_si256((const __m256i*) *item1);
    __m256i str2 = _mm256_lddqu_si256((const __m256i*) *item2);

    __m256i res  = _mm256_cmpeq_epi8(str1, str2);

    int cmp_res  = _mm256_movemask_epi8(res);

    return cmp_res != -1;
}

int del_str(item_t* item) {
    ASSERT_IF(VALID_PTR(*item), "Invalid *item ptr", 0);

    *item = (item_t) poisons::FREED_PTR;

    return 1;
}

char* random_word(char* word, int len) {
    ASSERT_IF(VALID_PTR(word), "Invalid word ptr", nullptr);
    ASSERT_IF(len > 0, "Incorrect len value. Should be (> 0)", nullptr);

    for (int i = 0; i < len; i++) {
        word[i] = rand() % ('z' - 'a' + 1) + 'a';
    }

    return word;
}

unsigned long long constant_hash(item_t* item) {
    return 1;
}

unsigned long long first_letter_hash(item_t* item) {
    return **item;
}

unsigned long long symbol_sum_hash(item_t* item) {
    char* string = *item;

    unsigned long long sum = 0;
    for (int i = 0; string[i] != '\0'; i++) {
        sum += string[i];
    }

    return sum;
}

unsigned long long string_len_hash(item_t* item) {
    return strlen(*item);
}

unsigned long long roll_hash(item_t* item) {
    char* string = *item;

    unsigned long long hash = string[0];
    while (*string) {
        hash = ((hash >> 1) | (hash << 63)) ^ *(string++);
    }

    return hash;
}

unsigned long long crc32_hash(item_t* item) {
    return (unsigned long long) crc32(*item, (DWORD)strlen(*item));
}

unsigned long long asm_len32_crc32_hash(item_t* item) {
    char* string = *item;

    unsigned long long hash = 0;

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
        : [arg_val]"S"(string)
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
        HashTable* table = CREATE_TABLE(table, print_str, strcmp_avx_32len, del_str, asm_len32_crc32_hash, validate_level_t::MEDIUM_VALIDATE, CAPACITY_VALUES[1]);

        clock_t start_time = clock();
                context    = load_strings_to_table(table, filename, 0);
        clock_t end_time   = clock();

        sum_time += end_time - start_time;

        if ((context->inserts * fi_coef) > context->finds) {
            int need_inserts = (int) (context->inserts * fi_coef) - context->finds;

            char* word = (char*) calloc(ALLIGENCE, sizeof(char));
                  word = random_word(word, rand() % ALLIGENCE + 1);
            for ( ; need_inserts > 0; need_inserts--) {
                start_time = clock();
                table_find(table, &word);
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

    HashTable*   table   = CREATE_TABLE(table, print_str, cmp_str, del_str, hash->func, validate_level_t::NO_VALIDATE, CAPACITY_VALUES[1]);
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
