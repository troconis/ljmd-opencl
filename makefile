#This is the template file that serves as the file
#to be edited to compile the software for different
#platforms

CC=gcc
#CFLAGS= -I/opt/amd/AMD-APP-SDK-v2.8-RC-lnx64/include
#CFLAGS= -I/opt/intel/opencl-1.2-3.0.56860/include
#CFLAGS=-I/usr/include/x86_64-linux-gnu/ -I/opt/cuda/5.0/include/
#OPENMP=-openmp 
#NUM=4
#OPT=-Wall -O0 $(OPENMP)
#OPT= -O3 $(OPENMP) -Wall -D__DEBUG -D_USE_FLOAT
#OPT= -O3 $(OPENMP) -Wall #-D_USE_FLOAT
#OPT= -O3 $(OPENMP) -Wall
#OPENCL_LIBS=-L/opt/amd/AMD-APP-SDK-v2.8-RC-lnx64/lib/x86_64 -lOpenCL
#OPENCL_LIBS=-L/opt/intel/opencl-1.2-3.0.56860/lib64 -lOpenCL
#OPENCL_LIBS=-L/opt/cuda/5.0/lib -lOpenCL
LIB=-lm


#Directories
SRC_DIR=src
OBJ_DIR=obj
INC_DIR=include

TEST_DIR=test
ORI_SRC_DIC=$(TEST_DIR)/src
.PHONY : clean test



EXE=ljmd_CL
CODE_FILES	= ljmd-cl.c OpenCL_utils.c
HEADER_FILES	= OpenCL_utils.h OpenCL_data.h opencl_kernels_as_string.h

OBJECTS	=$(patsubst %,$(OBJ_DIR)/%,$(CODE_FILES:.c=.o))
INCLUDES=$(patsubst %,$(INC_DIR)/%,$(HEADER_FILES))

#Compilation Flags
CFLAGS= -I./include -I/usr/include/x86_64-linux-gnu/ -I/opt/cuda/5.0/include/
OPENMP=-openmp
OPT= -O3 $(OPENMP) -Wall -D__DEBUG -D_USE_FLOAT
OPENCL_LIBS=-L/opt/cuda/5.0/lib -lOpenCL

$(EXE): $(OBJECTS)
	$(CC) $^ -o $@ $(OPENCL_LIBS) $(LIB)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c $(INCLUDES)
	$(CC) $(CFLAGS) $< -o $@ -c

run: $(EXE)
	cp $(EXE) $(TEST_DIR)/ ; cd $(TEST_DIR) ; make run

$(INC_DIR)/opencl_kernels_as_string.h: $(SRC_DIR)/opencl_kernels.cl
	awk '{print "\""$$0"\\n\""}' <$< >$@

test:
	cd $(TEST_DIR); make
clean:
	rm -f $(EXE) $(OBJECTS)
	cd $(TEST_DIR); make clean


#inputs and Benchmarks
# required input and data files.
INPUTS= argon_108.inp argon_2916.inp argon_78732.inp \
	argon_108.rest argon_2916.rest argon_78732.rest
