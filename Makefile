args = `arg="$(filter-out $@,$(MAKECMDGOALS))" && echo $${arg:-${1}}`

OUT_FILE = main.out

PROFILE_OUT_DIR  = logs/
PROFILE_OUT_FILE = profile.out
PROFILE_OUT 	 = $(PROFILE_OUT_DIR)/$(PROFILE_OUT_FILE).$(ID)

LIBS		= libs/baselib.cpp table/list.cpp
SRC_FILES   =  src/loader.cpp    src/analyzer.cpp

OPTIONS		= -o $(OUT_FILE) -I . -Ofast -g -no-pie -march=native

ID 			= $(shell python3 logs/id_script.py $(PROFILE_OUT_FILE) --dir=$(PROFILE_OUT_DIR))


# RULES =======================================================================
cr:
	clear
	gcc main.cpp table/hash_table.cpp $(SRC_FILES) $(LIBS) $(OPTIONS)
	./$(OUT_FILE) $(args)

c:
	gcc main.cpp table/hash_table.cpp $(SRC_FILES) $(LIBS) $(OPTIONS)

r:
	./$(OUT_FILE) $(args)

v:
	clear
	valgrind --tool=memcheck --leak-check=full ./$(OUT_FILE)

vc:
	valgrind --tool=callgrind --callgrind-out-file=$(PROFILE_OUT) ./$(OUT_FILE)
	
	kcachegrind $(PROFILE_OUT)

pyc:
	python3 data/clear_text.py $(args)
# =============================================================================
