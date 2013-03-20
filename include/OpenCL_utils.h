#ifndef __OPENCL_UTILS__
#define __OPENCL_UTILS__

#include <stdlib.h>
#include <stdio.h>
#include "CL/cl.h"
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/types.h>

/* This section contains the timing function */
double second();

//#define CL_DEVICE_KIND CL_DEVICE_TYPE_CPU
//#define CL_DEVICE_KIND CL_DEVICE_TYPE_ALL
//#define CL_DEVICE_KIND CL_DEVICE_TYPE_GPU

#define STRINGSIZE 2048

cl_int InitOpenCLEnvironment( char * device_type, cl_device_id * device, cl_context * context, cl_command_queue * cmdQueue );

char * source2string( char * filename );

const char * CLErrString(cl_int status);

void PrintPlatform(cl_platform_id platform);

// prints a short online platform summary
void PrintPlatformShort(cl_platform_id platform);

cl_int clSetMultKernelArgs( cl_kernel kernel, cl_uint first_index, cl_uint nargs, ... );

#define KArg(x) sizeof(x),&(x)

#endif
