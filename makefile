#This is the template file that serves as the file
#to be edited to compile the software for different
#platforms

# OS Name (Linux or Darwin)
OSUPPER = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
OSLOWER = $(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])

# Flags to detect 32-bit or 64-bit OS platform
OS_SIZE = $(shell uname -m | sed -e "s/i.86/32/" -e "s/x86_64/64/")
OS_ARCH = $(shell uname -m | sed -e "s/i386/i686/")

# Flags to detect either a Linux system (linux) or Mac OSX (darwin)
DARWIN = $(strip $(findstring DARWIN, $(OSUPPER)))

CC=gcc
LIB=-lm

ifeq ($(CC),icc)
      OPENMP = -openmp
  else
      OPENMP = -fopenmp
endif


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

OPENCL_PATH=/opt/AMDAPP/SDK

# OS-specific build flags
ifneq ($(DARWIN),) 
      OPENCL_LIBS= -framework OpenCL
      INCLUDE_PATH= -I$(INC_DIR) -I/Developer/NVIDIA/CUDA-5.5/include -D__PROFILING
else
  ifeq ($(OS_SIZE),32)
      CCFLAGS   := -m32
  else
      OPENCL_LIBS=-L$(OPENCL_PATH)/lib64 -lOpenCL
      INCLUDE_PATH= -I$(INC_DIR) -I/usr/include/x86_64-linux-gnu/ -I$(OPENCL_PATH)/include \
      -D__PROFILING
  endif
endif

#Compilation Flags
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
