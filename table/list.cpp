//
//  Created by IvanBrekman on 22.04.2022.
//

#include <cstdio>
#include <cstring>
#include <immintrin.h>

#include "config.hpp"
#include "list.hpp"

int strcmp_avx(char* string1, char* string2) {
    __m256i str1 = _mm256_lddqu_si256((const __m256i*) string1);
    __m256i str2 = _mm256_lddqu_si256((const __m256i*) string2);

    __m256i res  = _mm256_cmpeq_epi8(str1, str2);

    int cmp_res  = _mm256_movemask_epi8(res);

    return cmp_res != -1;
}

int list_ctor(List* lst, int capacity, validate level) {
    ASSERT_IF(VALID_PTR(lst), "Invalid lst ptr",                           0);
    ASSERT_IF(capacity > 0,   "Incorrect capacity value. Should be (> 0)", 0);

    lst->pointers = NEW_PTR(char*, capacity);

    lst->size     = 0;
    lst->capacity = capacity;
    lst->_vlevel  = level;

    ASSERT_OK_LIST(lst, "Check list after create", 0);

    return 1;
}

int list_dtor(List* lst) {
    ASSERT_OK_LIST(lst, "Check list before dtor call", 0);

    lst->size     = poisons::FREED_ELEMENT;
    lst->capacity = poisons::FREED_ELEMENT;

    FREE_PTR(lst->pointers, char*);

    return 1;
}

list_errors list_error(const List* lst) {
    if (!VALID_PTR(lst))           return list_errors::INVALID_LIST_PTR;
    if (!VALID_PTR(lst->pointers)) return list_errors::INVALID_POINTERS_PTR;

    if (lst->size     <  0)        return list_errors::INCORRECT_SIZE_VALUE;
    if (lst->capacity <= 0)        return list_errors::INCORRECT_CAPACITY_VALUE;

    if (lst->size > lst->capacity) return list_errors::SIZE_EXCEEDED_CAPACITY;

    if (lst->_vlevel >= validate::MEDIUM_VALIDATE) {
        for (int i = 0; i < lst->size; i++) {
            if (!VALID_PTR(lst->pointers[i])) return list_errors::INVALID_STRING_PTR;
        }
    }

    return list_errors::OK;
}

const char* list_error_desc(const list_errors error) {
    switch (error) {
        case list_errors::OK:
            return "ok";
        
        case list_errors::INVALID_LIST_PTR:
            return "Invalid pointer to list.";
        case list_errors::INVALID_POINTERS_PTR:
            return "Invalid pointer to list.pointers.";
        case list_errors::INVALID_STRING_PTR:
            return "Invalid pointer to list.pointers[i] string.";
        case list_errors::INCORRECT_SIZE_VALUE:
            return "Incorrect size value. Should be (>= 0).";
        case list_errors::INCORRECT_CAPACITY_VALUE:
            return "Incorrect capacity value. Should be (> 0).";
        case list_errors::SIZE_EXCEEDED_CAPACITY:
            return "List size more than list capacity";
        
        default:
            return "Unknown error.";
    }
}

int list_size(const List* lst) {
    ASSERT_OK_LIST(lst, "Check list before size call", 0);

    return lst->size;
}

int list_empty(const List* lst) {
    ASSERT_OK_LIST(lst, "Check list before empty call", 0);

    return lst->size == 0;
}

int list_resize(List* lst, int new_capacity) {
    ASSERT_OK_LIST(lst,                  "Check list before resize call",            0);
    ASSERT_IF(lst->size <= new_capacity, "new_capacity less than current list size", 0);
    
    lst->pointers = (char**) realloc(lst->pointers, sizeof(char*) * new_capacity);
    lst->capacity = new_capacity;

    if (!VALID_PTR(lst->pointers)) {
        if (lst->_vlevel >= validate::WEAK_VALIDATE) {
            list_dump(lst, "Not enough memory", 0);
        }

        errno = list_errors::NOT_ENOUGH_MEMORY;
        return 0;
    }

    ASSERT_OK_LIST(lst, "Check list after resize call", 0);
    return 1;
}

int list_find(const List* lst, char* string) {
    ASSERT_OK_LIST(lst, "Check list before find call", NOT_FOUND);
    ASSERT_IF(VALID_PTR(string), "Invalid string ptr", NOT_FOUND);

    for (int i = 0; i < lst->size; i++) {
        if (strcmp_avx(lst->pointers[i], string) == 0) return i;
    }

    return NOT_FOUND;
}

int list_push(List* lst, char* string) {
    ASSERT_OK_LIST(lst, "Check list before push call", 0);
    ASSERT_IF(VALID_PTR(string), "Invalid string ptr", 0);

    if (lst->size == lst->capacity) {
        list_resize(lst, lst->capacity * 2);
    }
    ASSERT_IF(lst->size < lst->capacity, "Resize func isn't work", 0);

    lst->pointers[lst->size++] = string;

    ASSERT_OK_LIST(lst, "Check list after push_back call", 0);
    return 1;
}

int list_print(const List* lst) {
    ASSERT_OK_LIST(lst, "Check list before print call", 0);

    printf("[ ");
    for (int i = 0; i < lst->size; i++) {
        printf("'%s'", lst->pointers[i]);
        if (i + 1 < lst->size) printf(", ");
    }
    printf(" ]\n");

    return 1;
}

int list_dump(const List* lst, const char* reason, FILE* log) {
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr",    0);
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);

    fprintf(log, COLORED_OUTPUT("|-------------------------          List  Dump          -------------------------|\n", ORANGE, log));
    FPRINT_DATE(log);
    fprintf(log, COLORED_OUTPUT("%s\n", BLUE, log), reason);

    list_errors error = list_error(lst);

    fprintf(log, "    List state: %d ", error);
    if (error) fprintf(log, COLORED_OUTPUT("(%s)\n", RED,   log), list_error_desc(error));
    else       fprintf(log, COLORED_OUTPUT("(%s)\n", GREEN, log), list_error_desc(error));

    if (!VALID_PTR(lst)) {
        fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
        fprintf(log, "\n\n");
        return 0;
    }

    fprintf(log, "    Size:     %d\n",   lst->size);
    fprintf(log, "    Capacity: %d\n\n", lst->capacity);

    if (IS_TERMINAL(log)) fprintf(log, "    Data[" PURPLE "%p" NATURAL "]:\n", lst->pointers);
    else                  fprintf(log, "    Data[%p]:\n",                      lst->pointers);

    if (!VALID_PTR(lst->pointers)) fprintf(log, COLORED_OUTPUT("    Cant access data by this ptr\n\n", RED, log));
    else if (lst->_vlevel >= MEDIUM_VALIDATE) {
        for (int i = 0; i < lst->size; i++) {
            fprintf(log, "\t[%5d]: '%s'\n", i, lst->pointers[i]);
        }
        fprintf(log, "\n");
    } else {
        fprintf(log, "    Data is hidden with current validate level(%d)\n\n", lst->_vlevel);
    }

    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}
