#ifndef __OPENCL_UTILS__
#define __OPENCL_UTILS__

#include <stdlib.h>
#include <stdio.h>
#include "CL/cl.h"

//#define CL_DEVICE_KIND CL_DEVICE_TYPE_CPU
//#define CL_DEVICE_KIND CL_DEVICE_TYPE_ALL
//#define CL_DEVICE_KIND CL_DEVICE_TYPE_GPU

#define STRINGSIZE 2048

cl_int InitOpenCLEnvironment( char * device_type, cl_device_id * device, cl_context * context, cl_command_queue * cmdQueue );

char * source2string( char * filename );

#endif
