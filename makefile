#! /bin/bash
#This is the template file that serves as the file
#to be edited to compile the software for different
#platforms

CC=icc
LIB=-lm


#Directories
SRC_DIR=src
OBJ_DIR=obj
INC_DIR=include

TEST_DIR=test
ORI_SRC_DIC=$(TEST_DIR)/src

.PHONY : clean test


#Files
EXE=ljmd-cl
CODE_FILES	= ljmd-cl.c OpenCL_utils.c
HEADER_FILES	= OpenCL_utils.h OpenCL_data.h opencl_kernels_as_string.h

OBJECTS	=$(patsubst %,$(OBJ_DIR)/%,$(CODE_FILES:.c=.o))
INCLUDES=$(patsubst %,$(INC_DIR)/%,$(HEADER_FILES))

#Compilation Flags
OPENCL_PATH=/opt/intel/opencl-1.2-3.0.56860
OPENCL_LIBS=-L$(OPENCL_PATH)/lib64 -lOpenCL
INCLUDE_PATH= -I$(INC_DIR) -I/usr/include/x86_64-linux-gnu/ -I$(OPENCL_PATH)/include -D__PROFILING
OPENMP=-openmp
OPT= -O3 $(OPENMP) -Wall -D__DEBUG -D_USE_FLOAT

#Instructions
$(EXE): $(OBJECTS)
	$(CC) $^ -o $@ $(OPENCL_LIBS) $(LIB)

$(EXE).opti: $(OBJECTS)
	$(CC) $(OPT) $^ -o $@ $(OPENCL_LIBS) $(LIB)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c $(INCLUDES)
	$(CC) $(INCLUDE_PATH) $< -o $@ -c

$(INC_DIR)/opencl_kernels_as_string.h: $(SRC_DIR)/opencl_kernels.cl
	awk '{print "\""$$0"\\n\""}' <$< >$@


## Calls
run: $(EXE)
	cp $(EXE) $(TEST_DIR)/ ; cd $(TEST_DIR) ; make run
optirun: $(EXE)
	cp $(EXE) $(TEST_DIR)/ ; cd $(TEST_DIR) ; make optirun
test: $(EXE) $(EXE).opti
	cp $(EXE) $(EXE).opti $(TEST_DIR)/
	cd $(TEST_DIR); make test
clean:
	rm -f $(EXE) $(EXE).opti $(OBJECTS) $(INC_DIR)/opencl_kernels_as_string.h
	cd $(TEST_DIR); make clean
