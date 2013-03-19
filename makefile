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
TEST_DIR=test
ORIGINAL_SOURCE=$(TEST_DIR)/src


test: ljmd-ori

ljmd-ori: $(ORIGINAL_SOURCE)/ljmd-c1.c
	$(CC) -o $(TEST_DIR)/$@ $< $(LIB)

clean:
	rm -v $(TEST_DIR)/ljmd-ori
