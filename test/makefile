#This is the template file that serves as the file
#to be edited to compile the software for different
#platforms

CC=gcc
LIB=-lm


#Directories
SRC_DIR=src
OBJ_DIR=obj
INC_DIR=-I../include/ -D__PROFILING
INPUT_SRC=../examples
TEST_DIR=.
ORI_SRC_DIC=$(TEST_DIR)/src
OPENMP=-openmp
OPT= -O3 $(OPENMP) -Wall


#Files
EXE=ljmd-cl
ORI_EXE=ljmd-ori
#inputs and Benchmarks
# required input and data files.
INPUT_NAMES	= argon_108
INPUT_INP	= $(patsubst %,%.inp,$(INPUT_NAMES))
INPUT_REST	= $(INPUT_INP:.inp=.rest)
INPUTS		= $(INPUT_INP) $(INPUT_REST)
REF_RESULTS	= $(INPUT_INP:.inp=.dat.base) $(INPUT_INP:.inp=.xyz.base)

#
EXECUTABLES	= $(EXE) $(EXE).opti $(ORI_EXE).opti
#Instructions
$(INPUTS):
	ln -s $(INPUT_SRC)/$@ $@

$(REF_RESULTS): $(ORI_EXE) $(INPUTS)
	for file in $(INPUT_NAMES); do \
	  ./$(ORI_EXE) < $$file.inp;  \
	  mv $$file.dat $$file.dat.base;  \
	  mv $$file.xyz $$file.xyz.base;  \
	done

$(ORI_EXE): $(ORI_SRC_DIC)/ljmd-c1.c
	$(CC) -o $(TEST_DIR)/$@ $< $(LIB) $(INC_DIR)

$(ORI_EXE).opti: $(ORI_SRC_DIC)/ljmd-c1.c
	$(CC) $(OPT) -o $(TEST_DIR)/$@ $< $(LIB) $(INC_DIR)


##Calls
run: $(EXE) $(INPUTS)
	./$(EXE) cpu < argon_108.inp

test: $(EXECUTABLES) $(INPUTS) $(REF_RESULTS)
	for exe in $(EXECUTABLES); do \
	   for file in $(INPUT_NAMES); do \
	       ./$$exe cpu < $$file.inp;  \
	       mv $$file.dat $$file.dat.$$exe;  \
	       mv $$file.xyz $$file.xyz.$$exe;  \
	   done;  \
	done
	python src/tester.py

clean:
	rm -f $(EXECUTABLES) $(ORI_EXE)
	rm -f $(INPUT_NAMES)*
