//
// Created by IvanBrekman on 22.04.2022
//

#include <cstring>

#include "config.hpp"
#include "libs/baselib.hpp"
#include "hash_table.hpp"

unsigned long long default_hash(item_t* item) {
    return (unsigned long long) item;
}

HashTable* table_ctor(HashInfo* info, int (*print_func)(item_t* item), int                (*cmp_func)  (item_t* item1, item_t* item2),
                                      int (*del_func)  (item_t* item), unsigned long long (*hash_func) (item_t* item),
                      validate_level_t level, int capacity) {
    ASSERT_IF(VALID_PTR(print_func), "Invalid print func ptr",                   nullptr);
    ASSERT_IF(VALID_PTR(cmp_func),   "Invalid cmp func ptr",                     nullptr);
    ASSERT_IF(VALID_PTR(del_func),   "Invalid del func ptr",                     nullptr);
    ASSERT_IF(VALID_PTR(hash_func),  "Invalid hash func ptr",                    nullptr);
    ASSERT_IF(capacity > 0,          "Incorrect capacity value (should be > 0)", nullptr);

    HashTable* table = NEW_PTR(HashTable, 1);

    table->_lcanary = INIT_CANARY;
    table->_rcanary = INIT_CANARY;

    table->size     = 0;
    table->capacity = capacity;
    table->data     = NEW_PTR(List, capacity);

    for (int i = 0; i < capacity; i++) {
        list_ctor(table->data + i, LIST_CAPACITY, level, print_func, cmp_func, del_func);
    }

    table->_pf     = print_func;
    table->_cmp    = cmp_func;
    table->_del    = del_func;
    table->_hash   = hash_func;
    table->_vlevel = level;

    if (VALID_PTR(info)) table->_info = info;
    else                 table->_info = (HashInfo*)NO_INFO_PTR;

    ASSERT_OK_HASHTABLE(table, "Check correctness table_ctor", nullptr);
    return table;
}

HashTable* table_dtor(HashTable* table) {
    ASSERT_OK_HASHTABLE(table, "Check before table_dtor", nullptr);

    for (int i = 0; i < table->capacity; i++) {
        list_dtor(table->data + i);
    }

    table->_lcanary = FREE_CANARY;
    table->_rcanary = FREE_CANARY;

    table->size     = poisons::FREED_ELEMENT;
    table->capacity = poisons::FREED_ELEMENT;

    table->_pf      = (int (*) (item_t*))         poisons::FREED_PTR;
    table->_cmp     = (int (*) (item_t*, item_t*))poisons::FREED_PTR;
    table->_del     = (int (*) (item_t*))         poisons::FREED_PTR;

    table->_hash    = (unsigned long long (*) (item_t*))poisons::FREED_PTR;

    if (VALID_PTR(table->_info)) FREE_PTR(table->_info, HashInfo);
    FREE_PTR(table->data, List);

    free(table);

    return nullptr;
}

hashtable_errors table_error(const HashTable* table) {
    if (!VALID_PTR(table))          return hashtable_errors::INVALID_TABLE_PTR;
    if (!VALID_PTR(table->_pf))     return hashtable_errors::INVALID_PRINT_ITEM_PTR;
    if (!VALID_PTR(table->_cmp))    return hashtable_errors::INVALID_CMP_ITEMS_PTR;
    if (!VALID_PTR(table->_hash))   return hashtable_errors::INVALID_HASH_FUNC_PTR;
    if (!VALID_PTR(table->data))    return hashtable_errors::INVALID_DATA_PTR_;

    if (!VALID_PTR(table->_info) && table->_info != (HashInfo*)NO_INFO_PTR)
                                    return hashtable_errors::INVALID_TABLE_INFO_PTR;

    if (table->_lcanary != INIT_CANARY)     return hashtable_errors::DAMAGED_LCANARY;
    if (table->_rcanary != INIT_CANARY)     return hashtable_errors::DAMAGED_RCANARY;

    if (table->_lcanary == FREE_CANARY || 
        table->_rcanary == FREE_CANARY)     return hashtable_errors::FREED_CANARY_VALUE;
    
    if (table->size > table->capacity)      return hashtable_errors::SIZE_EXCEEDED_CAPACITY_;

    if (table->_vlevel >= validate_level_t::HIGHEST_VALIDATE) {
        for (int i = 0; i < table->capacity; i++) {
            if(list_error(table->data + i)) return hashtable_errors::INVALID_LIST;
        }
    }

    return hashtable_errors::OK_;
}

const char* table_error_desc(const hashtable_errors error_code) {
    switch (error_code)
    {
        case hashtable_errors::OK_:
            return "ok";
        case hashtable_errors::INVALID_TABLE_PTR:
            return "Invalid Hash table pointer";
        case hashtable_errors::INVALID_TABLE_INFO_PTR:
            return "Invalid Hash table information pointer (struct with table info)";
        case hashtable_errors::INVALID_PRINT_ITEM_PTR:
            return "Invalid pointer to print item function";
        case hashtable_errors::INVALID_CMP_ITEMS_PTR:
            return "Invalid pointer to compare items function";
        case hashtable_errors::INVALID_HASH_FUNC_PTR:
            return "Invalid pointer to calculated hash function";
        case hashtable_errors::INVALID_DATA_PTR_:
            return "Invalid table data pointer";
        case hashtable_errors::INVALID_LIST:
            return "Error in one of table lists. See more about error at the end";
        case hashtable_errors::DAMAGED_LCANARY:
            return "Incorrect left canary value. Hash table is DAMAGED!";
        case hashtable_errors::DAMAGED_RCANARY:
            return "Incorrect right canary value. Hash table is DAMAGED!";
        case hashtable_errors::FREED_CANARY_VALUE:
            return "Detected specific 'freed' value in canaries. This is pointer to KILLED table!";
        case hashtable_errors::NOT_ENOUGH_MEMORY_:
            return "Not enough memory to increase capacity";
        case hashtable_errors::SIZE_EXCEEDED_CAPACITY_:
            return "Hash table size more than Hash table capacity";
        
        default:
            return "Unknown error";
    }
}

int table_add(HashTable* table, item_t* item) {
    ASSERT_OK_HASHTABLE(table,   "Check before add function", 0);
    ASSERT_IF(VALID_PTR(item),   "Invalid item ptr",          0);

    unsigned long long hash = table->_hash(item) % table->capacity;
    ASSERT_IF(hash < table->capacity, "Hash exceeded table capacity. Hash should be < table->capacity", 0);

    int res = list_push(table->data + hash, item);

    table->size += (res >= 0);
    if ((double)table->size > LOAD_FACTOR * table->capacity) {
        table_rehash(table, get_next_capacity(table->capacity));
    }

    return res >= 0;
}

int table_find(const HashTable* table, item_t* item) {
    ASSERT_OK_HASHTABLE(table,   "Check before find function", NOT_FOUND);
    ASSERT_IF(VALID_PTR(item),   "Invalid item ptr",           NOT_FOUND);

    unsigned long long hash = table->_hash(item) % table->capacity;
    ASSERT_IF(hash < table->capacity, "Hash exceeded table capacity. Hash should be < table->capacity", NOT_FOUND);

    return list_find(table->data + hash, item);
}

int table_rehash(HashTable* table, int new_capacity) {
    ASSERT_OK_HASHTABLE(table, "Check before table_rehash func", 0);

    LOG2(PRINT_WARNING("Table rehash\n"););


    HashTable* new_table = CREATE_TABLE(new_table,   table->_pf, table->_cmp, table->_del, table->_hash, table->_vlevel,
                                        new_capacity == OLD_CAPACITY ? table->capacity : new_capacity);
    
    for (int i = 0; i < table->capacity; i++) { 
        List* lst = table->data + i;

        for (int j = 0; j < lst->size; j++) {
            table_add(new_table, (list_t*) lst->data[j]);
        }
    }

    // Free table->data
    for (int i = 0; i < table->capacity; i++) {
        list_dtor(table->data + i);
    }
    FREE_PTR(table->data, List);
    // ================

    table->data     = new_table->data;
    table->capacity = new_capacity == OLD_CAPACITY ? table->capacity : new_capacity;

    // Free new_table
    if (VALID_PTR(new_table->_info)) FREE_PTR(new_table->_info, HashInfo);
    free(new_table);
    // ==============

    ASSERT_OK_HASHTABLE(table, "Check rehash func work", 0);

    return 1;
}

int get_next_capacity(int capacity) {
    int size = sizeof(CAPACITY_VALUES) / sizeof(CAPACITY_VALUES[0]);

    for (int i = 0; i < size; i++) {
        if (capacity < CAPACITY_VALUES[i]) return CAPACITY_VALUES[i];
    }

    int next_cap = CAPACITY_VALUES[size - 1];
    while (capacity > next_cap) next_cap *= 2;

    return next_cap;
}

int table_dump(const HashTable* table, const char* reason, FILE* log) {
    ASSERT_IF(VALID_PTR(log),    "Invalid log ptr",    0);
    ASSERT_IF(VALID_PTR(reason), "Invalid reason ptr", 0);

    fprintf(log, COLORED_OUTPUT("|-------------------------       Hash Table  Dump       -------------------------|\n", ORANGE, log));
    FPRINT_DATE(log);
    fprintf(log, COLORED_OUTPUT("%s\n", BLUE, log), reason);

    hashtable_errors error = table_error(table);

    fprintf(log, "    Table state: %d ", error);
    if (error) fprintf(log, COLORED_OUTPUT("(%s)\n", RED,   log), table_error_desc(error));
    else       fprintf(log, COLORED_OUTPUT("(%s)\n", GREEN, log), table_error_desc(error));

    if (!VALID_PTR(table)) {
        fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
        fprintf(log, "\n\n");
        return 0;
    }

    if (VALID_PTR(table->_info)) {
        fprintf(log, "    HashTable <" PURPLE "%s" NATURAL ">[" CYAN "%p" NATURAL "] " "\"" ORANGE_UNL "%s" NATURAL "\" from %s:%d, " CYAN "%s" NATURAL " function\n\n",
            VALID_PTR(table->_info->type) ? table->_info->type : COLORED_OUTPUT("unknown", RED, log),
                                            table,
            VALID_PTR(table->_info->name) ? table->_info->name : COLORED_OUTPUT("unknown", RED, log),
            VALID_PTR(table->_info->file) ? table->_info->file : COLORED_OUTPUT("unknown", RED, log),
                                            table->_info->line,
            VALID_PTR(table->_info->func) ? table->_info->func : COLORED_OUTPUT("unknown", RED, log)
        );
    } else {
        fprintf(log, "    ");
        fprintf(log, COLORED_OUTPUT("Can't get hash table information from %p\n\n", GRAY_UNL, log), table->_info);
    }

    fprintf(log, "    Left  canary:   %llX %s\n",   table->_lcanary, table->_lcanary == INIT_CANARY ? "" : BAD_OUTPUT);
    fprintf(log, "    Right canary:   %llX %s\n\n", table->_rcanary, table->_rcanary == INIT_CANARY ? "" : BAD_OUTPUT);
    fprintf(log, "    Print func ptr: %p %s\n",   table->_pf,   VALID_PTR(table->_pf)   ? COLORED_OUTPUT("(ok)", GREEN, log) : BAD_OUTPUT);
    fprintf(log, "    Cmp   func ptr: %p %s\n",   table->_cmp,  VALID_PTR(table->_cmp)  ? COLORED_OUTPUT("(ok)", GREEN, log) : BAD_OUTPUT);
    fprintf(log, "    Hash  func ptr: %p %s\n\n", table->_hash, VALID_PTR(table->_hash) ? COLORED_OUTPUT("(ok)", GREEN, log) : BAD_OUTPUT);

    fprintf(log, "    Size:     %d\n",   table->size);
    fprintf(log, "    Capacity: %d\n\n", table->capacity);

    if (IS_TERMINAL(log)) fprintf(log, "    Data[" PURPLE "%p" NATURAL "]:\n", table->data);
    else                  fprintf(log, "    Data[%p]:\n",                      table->data);

    if (!VALID_PTR(table->data)) fprintf(log, COLORED_OUTPUT("    Cant access data by this ptr\n\n", RED, log));
    else if (table->_vlevel >= validate_level_t::MEDIUM_VALIDATE) {
        for (int i = 0; i < table->capacity; i++) {
            List* lst = table->data + i;

            if (list_empty(lst)) {
                int start = i;
                while (list_empty(lst) && ++i < table->capacity) {
                    lst = table->data + i;
                }

                i--;
                fprintf(log, "\t[%5d]: %s\n", start,                COLORED_OUTPUT("empty", CYAN, log));
                if (i - start > 1) fprintf(log, "\t  ...  \n");
                if (i - start > 0) fprintf(log, "\t[%5d]: %s\n", i, COLORED_OUTPUT("empty", CYAN, log));
            } else {
                fprintf(log, "\t[%5d](%d): ", i, lst->size);
                list_print(table->data + i);
            }
            WAIT_INPUT;
        }
        fprintf(log, "\n");
    } else {
        fprintf(log, "    Data is hidden with current validate level(%d)\n\n", table->_vlevel);
    }

    fprintf(log, COLORED_OUTPUT("|---------------------Compilation  Date %s %s---------------------|", ORANGE, log),
            __DATE__, __TIME__);
    fprintf(log, "\n\n");

    return 1;
}
