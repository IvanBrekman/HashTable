args = `arg="$(filter-out $@,$(MAKECMDGOALS))" && echo $${arg:-${1}}`

OUT_FILE = main.out

LIBS		= libs/baselib.cpp table/list.cpp
OPTIONS		= -o $(OUT_FILE) -I . -Ofast -g -no-pie

# RULES =======================================================================
cr:
	clear
	gcc main.cpp table/hash_table.cpp $(LIBS) $(OPTIONS)
	./$(OUT_FILE) $(args)

c:
	gcc main.cpp table/hash_table.cpp $(LIBS) $(OPTIONS)

r:
	./$(OUT_FILE) $(args)

v:
	clear
	valgrind --tool=memcheck --leak-check=full ./$(OUT_FILE)
# =============================================================================
