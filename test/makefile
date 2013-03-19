#This is the template file that serves as the file
#to be edited to compile the software for different
#platforms

CC=gcc
LIB=-lm


#Directories
SRC_DIR=src
OBJ_DIR=obj
INC_DIR=include
INPUT_SRC=../examples
TEST_DIR=.
ORI_SRC_DIC=$(TEST_DIR)/src

EXE=ljmd-ori
#inputs and Benchmarks
# required input and data files.
INPUTS= argon_108.inp argon_2916.inp argon_78732.inp \
	argon_108.rest argon_2916.rest argon_78732.rest


test: $(EXE) $(INPUTS)
	./$(EXE) < argon_108.inp

$(EXE): $(ORI_SRC_DIC)/ljmd-c1.c
	$(CC) -o $(TEST_DIR)/$@ $< $(LIB)

clean:
	rm -f $(TEST_DIR)/ljmd-ori
	rm -f $(INPUTS) *.dat *.xyz


$(INPUTS):
	ln -s $(INPUT_SRC)/$@ $@
