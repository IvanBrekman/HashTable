#include <cstdio>

#include "config.hpp"
#include "table/hash_table.hpp"

int main(int argc, char** argv) {
    HashTable* table = CREATE_TABLE(table, default_hash, validate::MEDIUM_VALIDATE, 150);

    char* str1 = "asdf";
    char* str2 = "qwe";
    char* str3 = "zxcb";
    char* str4 = "12345";
    char* str5 = "qqq";
    char* str6 = "fds245";

    table_add(table, str1);
    table_add(table, str2);
    table_add(table, str3);
    table_add(table, str4);
    table_add(table, str5);
    table_add(table, str6);

    table_dump(table, "Check");

    table_dtor(table);

    return 0;
}
