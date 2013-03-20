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

#Files
EXE=ljmd_CL
ORI_EXE=ljmd-ori
#inputs and Benchmarks
# required input and data files.
INPUTS= argon_108.inp argon_2916.inp argon_78732.inp \
	argon_108.rest argon_2916.rest argon_78732.rest
REFERENCE_RESULTS=argon_108_base.dat argon_108_base.xyz

#Instructions
$(INPUTS):
	ln -s $(INPUT_SRC)/$@ $@
	
$(REFERENCE_RESULTS): $(ORI_EXE) $(INPUTS)
	./$(ORI_EXE) < argon_108.inp
	mv argon_108.dat argon_108_base.dat; mv argon_108.xyz argon_108_base.xyz

$(ORI_EXE): $(ORI_SRC_DIC)/ljmd-c1.c
	$(CC) -o $(TEST_DIR)/$@ $< $(LIB)


##Calls
run: $(EXE) $(INPUTS)
	./$(EXE) cpu < argon_108.inp

optirun: $(EXE) $(INPUTS)
	time optirun ./$(EXE) gpu 1536 < argon_2916.inp

test: $(EXE) $(INPUTS) $(REFERENCE_RESULTS)
	./$(EXE) cpu < argon_108.inp
	mv argon_108.dat argon_108_CL.dat; mv argon_108.xyz argon_108_CL.xyz
	python src/tester.py

clean:
	rm -f $(TEST_DIR)/$(ORI_EXE)
	rm -f $(INPUTS) *.dat *.xyz
	rm -f $(EXE)
