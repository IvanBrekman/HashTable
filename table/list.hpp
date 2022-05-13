//
//  Created by IvanBrekman on 22.04.2022.
//

#ifndef LIST_LISTH
#define LIST_LISTH

#include "config.hpp"
#include "libs/baselib.hpp"

#ifndef NO_CHECKS
    #define ASSERT_OK_LIST(obj, reason, ret) do {                   \
        if (obj->_vlevel >= WEAK_VALIDATE && list_error(obj)) {     \
            list_dump(obj, reason);                                 \
            if (obj->_vlevel >= HIGHEST_VALIDATE) {                 \
                LOG_DUMP(obj, reason, list_dump);                   \
            }                                                       \
            ASSERT_IF(0, "verify failed", ret);                     \
        } else if (list_error(obj)) {                               \
            errno = list_error(obj);                                \
            return ret;                                             \
        }                                                           \
    } while (0)
#else
    #define ASSERT_OK_LIST(obj, reason, ret) 
#endif

struct List {
    validate_level_t _vlevel = validate_level_t::NO_VALIDATE;

    char** pointers = (char**) poisons::UNINITIALIZED_PTR;

    int    size     = poisons::UNINITIALIZED_INT;
    int    capacity = poisons::UNINITIALIZED_INT;
};

enum list_errors {
    INVALID_LIST_PTR            = -1,
    INVALID_POINTERS_PTR        = -2,
    INVALID_STRING_PTR          = -3,
    
    INCORRECT_SIZE_VALUE        = -4,
    INCORRECT_CAPACITY_VALUE    = -5,

    SIZE_EXCEEDED_CAPACITY      = -6,

    NOT_ENOUGH_MEMORY           = -7,

    OK = 0
};

int strcmp_avx(char* string1, char* string2);

int list_ctor(List* lst, int capacity, validate_level_t level);
int list_dtor(List* lst);

list_errors list_error     (const List*       lst);
const char* list_error_desc(const list_errors error);

int list_size  (const List* lst);
int list_empty (const List* lst);

int list_resize(      List* lst, int new_capacity);

int list_find  (const List* lst, char* string);
int list_push  (      List* lst, char* string);

int list_print (const List* lst);
int list_dump  (const List* lst, const char* reason, FILE* log=stdout);

#endif // LIST_LISTH
