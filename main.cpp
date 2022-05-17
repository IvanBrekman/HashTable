#include <cstdio>

#include "config.hpp"
#include "src/analyzer.hpp"

const char* PYTHON_SCRIPT   = "python3 data/graph/draw_graph.py info.txt graph --dir=data/graph/";

const char* LOAD_FILE       = "data/text/shakespear_cleared.txt";
const char* LOAD_SET_FILE   = "data/text/shakespear_cleared++.txt";
const char* SAVE_FILE       = "data/graph/info.txt";

int main(int argc, char** argv) {
    const char* filename = argc > 1 ? argv[1] : LOAD_FILE;

    test_table_speed(filename, 10, 100);

#ifdef TEST_SPEED
    return 0;
#endif
    
    CLEAR_SAVE_FILE(SAVE_FILE);

    test_func_collision(LOAD_SET_FILE, ALL_HASH_FUNCS + 0, SAVE_FILE, 0, 256);
    test_func_collision(LOAD_SET_FILE, ALL_HASH_FUNCS + 1, SAVE_FILE, 0, 256);
    test_func_collision(LOAD_SET_FILE, ALL_HASH_FUNCS + 2, SAVE_FILE, 0, OLD_CAPACITY);
    test_func_collision(LOAD_SET_FILE, ALL_HASH_FUNCS + 3, SAVE_FILE, 0, 256);
    test_func_collision(LOAD_SET_FILE, ALL_HASH_FUNCS + 4, SAVE_FILE, 0, OLD_CAPACITY);
    test_func_collision(LOAD_SET_FILE, ALL_HASH_FUNCS + 5, SAVE_FILE, 0, OLD_CAPACITY);

    if (AUTO_DRAW) {
        printf("Please wait, graphs are drawing...\n");
        int res = system(PYTHON_SCRIPT);
        
        if (res == 0) printf("Complete.\n\n");
        else          printf("Error in drawing.\n\n");
    }

    return 0;
}
