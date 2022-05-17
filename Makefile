args = $(filter-out $@,$(MAKECMDGOALS))

OUT_FILE 		 = main.out
OUT_FILE_ASM1	 = list_find.o
OUT_FILE_ASM2    = table_find.o

ASM_FILE1		 = table/list_find.asm
ASM_FILE2		 = table/table_find.asm

PROFILE_OUT_DIR  = logs/
PROFILE_OUT_FILE = profile.out
PROFILE_OUT 	 = $(PROFILE_OUT_DIR)/$(PROFILE_OUT_FILE).$(ID)

LIBS		= libs/baselib.cpp table/list.cpp
SRC_FILES   =  src/loader.cpp    src/analyzer.cpp

OPTIONS		= -o $(OUT_FILE) -I . -O2 -g -no-pie -march=native

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

cra:
	clear
	
	nasm $(ASM_FILE1) -f elf64 -o $(OUT_FILE_ASM1)
	nasm $(ASM_FILE2) -f elf64 -o $(OUT_FILE_ASM2)

	gcc -c main.cpp -c table/hash_table.cpp -c src/loader.cpp -c src/analyzer.cpp -c libs/baselib.cpp -c table/list.cpp \
		-I . -O2 -g -no-pie -march=native
	gcc  main.o	  hash_table.o   loader.o   analyzer.o   baselib.o   list.o   $(OUT_FILE_ASM1)   $(OUT_FILE_ASM2) -o $(OUT_FILE)

	rm ./main.o ./hash_table.o ./loader.o ./analyzer.o ./baselib.o ./list.o ./$(OUT_FILE_ASM1) ./$(OUT_FILE_ASM2)

	./$(OUT_FILE)

v:
	clear
	valgrind --tool=memcheck --leak-check=full ./$(OUT_FILE)

vc:
	valgrind --tool=callgrind --callgrind-out-file=$(PROFILE_OUT) ./$(OUT_FILE)
	
	kcachegrind $(PROFILE_OUT)

pyc:
	python3 data/clear_text.py data/text/$(args)
# =============================================================================
