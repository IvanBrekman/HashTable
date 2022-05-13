//
//  Created by IvanBrekman on 22.04.2022.
//

#include <cstdio>
#include <cstring>

#include "config.hpp"
#include "list.hpp"

int list_ctor(List* lst, int capacity, validate_level_t level, 
              int (*print_func)(list_t* item), int (*cmp_func) (list_t* item1, list_t* item2),
              int (*del_func)  (list_t* item)) {
    ASSERT_IF(VALID_PTR(lst),        "Invalid lst ptr",                           0);
    ASSERT_IF(VALID_PTR(print_func), "Invalid print func ptr",                    0);
    ASSERT_IF(VALID_PTR(cmp_func),   "Invalid cmp func ptr",                      0);
    ASSERT_IF(VALID_PTR(del_func),   "Invalid del func ptr",                      0);
    ASSERT_IF(capacity > 0,          "Incorrect capacity value. Should be (> 0)", 0);

    lst->data     = NEW_PTR(list_t, capacity);

    lst->size     = 0;
    lst->capacity = capacity;
    lst->_vlevel  = level;

    lst->_pf     = print_func;
    lst->_cmp    = cmp_func;
    lst->_del    = del_func;

    ASSERT_OK_LIST(lst, "Check list after create", 0);

    return 1;
}

int list_dtor(List* lst) {
    ASSERT_OK_LIST(lst, "Check list before dtor call", 0);

    lst->size     = poisons::FREED_ELEMENT;
    lst->capacity = poisons::FREED_ELEMENT;

    for (int i = 0; i < lst->size; i++) {
        lst->_del(lst->data + i);
    }

    lst->_pf  = (int (*) (list_t*))         poisons::FREED_PTR;
    lst->_cmp = (int (*) (list_t*, list_t*))poisons::FREED_PTR;
    lst->_del = (int (*) (list_t*))         poisons::FREED_PTR;

    FREE_PTR(lst->data, list_t);

    return 1;
}

list_errors list_error(const List* lst) {
    if (!VALID_PTR(lst))        return list_errors::INVALID_LIST_PTR;
    if (!VALID_PTR(lst->data))  return list_errors::INVALID_DATA_PTR;

    if (lst->size     <  0)     return list_errors::INCORRECT_SIZE_VALUE;
    if (lst->capacity <= 0)     return list_errors::INCORRECT_CAPACITY_VALUE;

    if (lst->size > lst->capacity) return list_errors::SIZE_EXCEEDED_CAPACITY;

    return list_errors::OK;
}

const char* list_error_desc(const list_errors error) {
    switch (error) {
        case list_errors::OK:
            return "ok";
        
        case list_errors::INVALID_LIST_PTR:
            return "Invalid pointer to list.";
        case list_errors::INVALID_DATA_PTR:
            return "Invalid pointer to list.pointers.";
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
    
    lst->data = (list_t*) realloc(lst->data, sizeof(list_t) * new_capacity);
    lst->capacity = new_capacity;

    if (!VALID_PTR(lst->data)) {
        if (lst->_vlevel >= validate_level_t::WEAK_VALIDATE) {
            list_dump(lst, "Not enough memory", 0);
        }

        errno = list_errors::NOT_ENOUGH_MEMORY;
        return 0;
    }

    ASSERT_OK_LIST(lst, "Check list after resize call", 0);
    return 1;
}

int list_find(const List* lst, list_t* item) {
    ASSERT_OK_LIST(lst,        "Check list before find call", NOT_FOUND);
    ASSERT_IF(VALID_PTR(item), "Invalid string ptr",          NOT_FOUND);

    for (int i = 0; i < lst->size; i++) {
        if (lst->_cmp(lst->data + i, item) == 0) return i;
    }

    return NOT_FOUND;
}

int list_push(List* lst, list_t* item) {
    ASSERT_OK_LIST(lst,        "Check list before push call", 0);
    ASSERT_IF(VALID_PTR(item), "Invalid string ptr",          0);

    if (lst->size == lst->capacity) {
        list_resize(lst, lst->capacity * 2);
    }
    ASSERT_IF(lst->size < lst->capacity, "Resize func isn't work", 0);

    lst->data[lst->size++] = *item;

    ASSERT_OK_LIST(lst, "Check list after push_back call", 0);
    return 1;
}

int list_print(const List* lst) {
    ASSERT_OK_LIST(lst, "Check list before print call", 0);

    printf("[ ");
    for (int i = 0; i < lst->size; i++) {
        lst->_pf(lst->data + i);
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

    if (IS_TERMINAL(log)) fprintf(log, "    Data[" PURPLE "%p" NATURAL "]:\n", lst->data);
    else                  fprintf(log, "    Data[%p]:\n",                      lst->data);

    if (!VALID_PTR(lst->data)) fprintf(log, COLORED_OUTPUT("    Cant access data by this ptr\n\n", RED, log));
    else if (lst->_vlevel >= MEDIUM_VALIDATE) {
        for (int i = 0; i < lst->size; i++) {
            fprintf(log, "\t[%5d]: '%s'\n", i, lst->data[i]);
        }
        fprintf(log, "\n");
    } else {
        fprintf(log, "    Data is hidden with current validate_level_t level(%d)\n\n", lst->_vlevel);
    }

    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}
