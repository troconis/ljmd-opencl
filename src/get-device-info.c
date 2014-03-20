/** This prints the number of devices with the type and the number of hardware threads */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "OpenCL_utils.h"

#ifdef _USE_FLOAT
#define FPTYPE float
#define ZERO  0.0f
#define HALF  0.5f
#define TWO   2.0f
#define THREE 3.0f
static const char kernelflags[] = "-D_USE_FLOAT -cl-denorms-are-zero -cl-unsafe-math-optimizations";
#else
#define FPTYPE double
#define ZERO  0.0
#define HALF  0.5
#define TWO   2.0
#define THREE 3.0
static const char kernelflags[] = "-cl-unsafe-math-optimizations";
#endif

#define Warning(...)    fprintf(stderr, __VA_ARGS__)

main(int argc, char * argv[])
{
    //Opts opts = { 0 };
    cl_int status;
    cl_platform_id *platformList;
    cl_uint numPlatforms;
    int ii;


    if ((status = clGetPlatformIDs(0, NULL, &numPlatforms)) != CL_SUCCESS) {
       Warning("Unable to query the number of platforms: %s\n", CLErrString(status));
       exit(1);
    }
    printf("Found %d platform(s).\n", numPlatforms);


    platformList = malloc(sizeof(cl_platform_id) * numPlatforms);
    if ((status = clGetPlatformIDs(numPlatforms, platformList, NULL)) != CL_SUCCESS) {
       Warning("Unable to enumerate the platforms: %s\n", CLErrString(status));
       exit(1);
    }

    for (ii = 0; ii < numPlatforms; ii++) {
       PrintPlatformShort(platformList[ii]);
    }

    free(platformList);
    exit(0);

}
