#include "OpenCL_utils.h"
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/types.h>

#define Warning(...)    fprintf(stderr, __VA_ARGS__)

/*
 * CLErrString --
 *
 *      Utility function that converts an OpenCL status into a human
 *      readable string.
 *
 * Results:
 *      const char * pointer to a static string.
 */




double second()
 
/* Returns elepsed seconds past from the last call to timer rest */
{

    struct timeval tmp;
    double sec;
    gettimeofday( &tmp, (struct timezone *)0 );
    sec = tmp.tv_sec + ((double)tmp.tv_usec)/1000000.0;
    return sec;
}



const char * CLErrString(cl_int status) {
   static struct { cl_int code; const char *msg; } error_table[] = {
      { CL_SUCCESS, "success" },
      { CL_DEVICE_NOT_FOUND, "device not found", },
      { CL_DEVICE_NOT_AVAILABLE, "device not available", },
      { CL_COMPILER_NOT_AVAILABLE, "compiler not available", },
      { CL_MEM_OBJECT_ALLOCATION_FAILURE, "mem object allocation failure", },
      { CL_OUT_OF_RESOURCES, "out of resources", },
      { CL_OUT_OF_HOST_MEMORY, "out of host memory", },
      { CL_PROFILING_INFO_NOT_AVAILABLE, "profiling not available", },
      { CL_MEM_COPY_OVERLAP, "memcopy overlaps", },
      { CL_IMAGE_FORMAT_MISMATCH, "image format mismatch", },
      { CL_IMAGE_FORMAT_NOT_SUPPORTED, "image format not supported", },
      { CL_BUILD_PROGRAM_FAILURE, "build program failed", },
      { CL_MAP_FAILURE, "map failed", },
      { CL_INVALID_VALUE, "invalid value", },
      { CL_INVALID_DEVICE_TYPE, "invalid device type", },
      { 0, NULL },
   };
   static char unknown[25];
   int ii;

   for (ii = 0; error_table[ii].msg != NULL; ii++) {
      if (error_table[ii].code == status) {
         return error_table[ii].msg;
      }
   }

   snprintf(unknown, sizeof unknown, "unknown error %d", status);
   return unknown;
}

void PrintDeviceShort(cl_device_id device);

void PrintPlatform(cl_platform_id platform) {

   static struct { cl_platform_info param; const char *name; } props[] = {
      { CL_PLATFORM_PROFILE, "profile" },
      { CL_PLATFORM_VERSION, "version" },
      { CL_PLATFORM_NAME, "name" },
      { CL_PLATFORM_VENDOR, "vendor" },
      { CL_PLATFORM_EXTENSIONS, "extensions" },
      { 0, NULL },
   };
   cl_device_id *deviceList;
   cl_uint numDevices;
   cl_int status;
   char buf[65536];
   size_t size;
   int ii;

   for (ii = 0; props[ii].name != NULL; ii++) {
      status = clGetPlatformInfo(platform, props[ii].param, sizeof buf, buf, &size);
      if ( status != CL_SUCCESS ) {
	fprintf( stderr, "platform[%p]: Unable to get %s: %s\n", platform, props[ii].name, CLErrString( status ) );
         continue;
      }
      if ( size > sizeof buf ) {
	fprintf( stderr, "platform[%p]: Huge %s (%ld bytes)!  Truncating to %ld\n", platform, props[ii].name, size, sizeof buf );
      }
      fprintf( stdout, "platform[%p]: %s: %s\n", platform, props[ii].name, buf );
   }
  
}

// prints a short one line platform summary
void PrintPlatformShort(cl_platform_id platform) {

   static struct { cl_platform_info param; const char *name;  char value[255];} props[] = {
      { CL_PLATFORM_PROFILE, "profile","" },
      { CL_PLATFORM_VERSION, "version","" },
      { CL_PLATFORM_NAME, "name","" },
      { CL_PLATFORM_VENDOR, "vendor","" },
      { 0, NULL },
   };

   cl_int status;
   size_t size;
   int ii;
   cl_device_id *deviceList;
   cl_uint numDevices;

   for (ii = 0; props[ii].name != NULL; ii++) {
      status = clGetPlatformInfo(platform, props[ii].param, sizeof(props[ii].value), props[ii].value, &size);
      //fprintf(stdout, "size of buf: %d \n", sizeof(props[ii].value));
      if ( status != CL_SUCCESS ) {
		fprintf( stderr, "platform[%p]: Unable to get %s: %s\n", platform, props[ii].name, CLErrString( status ) );
			 continue;
      }
      if ( size > sizeof  props[ii].value) {
		fprintf( stderr, "platform[%p]: Huge %s (%ld bytes)!  Truncating to %ld\n",
				platform, props[ii].name, size, sizeof  props[ii].value );
		  }

   }
   fprintf( stdout, "%s (%s)\n",  props[3].value, props[2].value);

   if ((status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL,
                                0, NULL, &numDevices)) != CL_SUCCESS) {
      Warning("platform[%p]: Unable to query the number of devices: %s\n",
              platform, CLErrString(status));
      return;
   }
   printf("    Found %d device(s).\n",numDevices);



   deviceList = malloc(numDevices * sizeof(cl_device_id));
   if ((status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL,
                                numDevices, deviceList, NULL)) != CL_SUCCESS) {
      Warning("platform[%p]: Unable to enumerate the devices: %s\n",
              platform, CLErrString(status));
      free(deviceList);
      return;
   }

   for (ii = 0; ii < numDevices; ii++) {
     PrintDeviceShort(deviceList[ii]);
   }

   free(deviceList);

}


typedef struct {
	char name[255];
	cl_device_type type;
	cl_bool available;
	size_t max_workgroup_items;
	cl_uint max_compute_units;

} BasicDeviceInfo;



void PrintDeviceShort(cl_device_id device) {

   cl_int status;
   size_t size;

   cl_device_id *deviceList;
   BasicDeviceInfo device_info;

   status = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_info.name), device_info.name, &size);
   if (status != CL_SUCCESS) {Warning("Unable to get device name (%s)", CLErrString(status));}
   fprintf(stdout, "        name: %s\n", device_info.name);

   status = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(device_info.max_compute_units), &device_info.max_compute_units, &size);
   if (status != CL_SUCCESS) {Warning("Unable to get max_workgroup_items (%s)", CLErrString(status));}
   fprintf(stdout, "        max_compute_units: %d\n", device_info.max_compute_units);

   status = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(device_info.max_workgroup_items), &device_info.max_workgroup_items, &size);
   if (status != CL_SUCCESS) {Warning("Unable to get max_workgroup_items (%s)", CLErrString(status));}
   fprintf(stdout, "        max_workgroup_items: %ld\n", device_info.max_workgroup_items);

   //TODO get device type etc ...

}





cl_int InitOpenCLEnvironment( char * device_type, cl_device_id * device, cl_context * context, cl_command_queue * cmdQueue ) {

  cl_int status;
  cl_uint numPlatforms, numDevices;
  cl_device_type device_kind;
  cl_platform_id * tmp_platforms, * platform;

  /* Initialize the Platform. Program consider a single platform. The first platform is taken whether numPlatforms > 1 */
  if ( ( status = clGetPlatformIDs( 0, NULL, &numPlatforms ) ) != CL_SUCCESS ) {
    fprintf( stderr, "Unable to query the number of platforms: %s\n", CLErrString(status) );
    exit( 1 );
  }



#ifdef __DEBUG
  fprintf( stdout, "Found %d platform(s).\n", numPlatforms );
#endif

  tmp_platforms = (cl_platform_id *) malloc( sizeof(cl_platform_id) * numPlatforms );
  if ( ( status = clGetPlatformIDs( numPlatforms, tmp_platforms, NULL ) ) != CL_SUCCESS ) {
    fprintf( stderr, "Unable to enumerate the platforms: %s\n", CLErrString(status));
    exit( 1 );
  }
  
  if( !strcmp( device_type, "gpu" ) ) {
    fprintf( stderr, "\n GPU RANGE" );
    platform = &tmp_platforms[0];
    device_kind = CL_DEVICE_TYPE_GPU;
  }
  else { 
    platform = &tmp_platforms[1];
    device_kind = CL_DEVICE_TYPE_CPU;
  }

#ifdef __DEBUG
  PrintPlatform( (* platform) );
#endif 

  /* Initialize the Devices */
  if ((status = clGetDeviceIDs( (* platform ), device_kind, 0, NULL, &numDevices ) ) != CL_SUCCESS) {
    fprintf( stderr, "platform[%p]: Unable to query the number of devices: %s\n", (* platform), CLErrString( status ) );
    exit( 1 );
   }

#ifdef __DEBUG
  fprintf( stdout, "platform[%p]: Found a device.\n", (* platform) );
#endif

   if ((status = clGetDeviceIDs( (* platform), device_kind, 1, device, NULL)) != CL_SUCCESS) {
     fprintf ( stderr, "platform[%p]: Unable to enumerate the devices: %s\n", (* platform), CLErrString( status ) );
     exit( 1 );
   }

   (* context) = clCreateContext( NULL, 1, device, NULL, NULL, &status );
  
   if ( status != CL_SUCCESS ) {
     fprintf ( stderr, "platform[%p]: Unable to init OpenCL context: %s\n", (* platform), CLErrString( status ) );
     exit( 1 );     
   }

   (* cmdQueue) = clCreateCommandQueue( (* context), (* device), 0, &status );
  
   if ( status != CL_SUCCESS ) {
     fprintf ( stderr, "platform[%p]: Unable to init OpenCL command queue: %s\n", (* platform), CLErrString( status ) );
     exit( 1 );
   }

   return CL_SUCCESS;

}


char * source2string( char * filename ){

  char line_buffer[STRINGSIZE];
  char * string_buffer = NULL, * tmp = NULL;

  size_t buffer_size = 0;
  FILE * fp_file;




  fp_file = fopen( filename, "r" );
  if( !fp_file ){
    fprintf( stderr, "Unable to open the source file %s. Program will end.\n", filename );
    exit(1);
  }

  while( fgets( line_buffer, sizeof( line_buffer ), fp_file ) != NULL ){

    tmp = realloc( string_buffer, buffer_size + strlen( line_buffer ) + 1 );
    if( !tmp ) {
      fprintf( stderr, "Unable to allocate buffer to store OpenCL kernel source. Program will end.\n" );
      exit(1);
    }
    
    string_buffer = tmp;
    string_buffer[ buffer_size ] = 0;
    strcat( string_buffer, line_buffer );
    buffer_size += strlen( line_buffer ) + 1;
  
  }
  
#ifdef __DEBUG
  fprintf( stdout, "\n\nPrinting content of the %s file: \n\n %s", filename, string_buffer );
#endif

  return string_buffer;



}


/* commodity function for a row of clSetKernelArg calls
   arguments: kernel,
              first_index: index to start from,
              nargs: number of clSetKernelArg calls (with linearly increasing index,
              then follow narg arguments to be set in the form of KArg(variablename)
*/
cl_int clSetMultKernelArgs( cl_kernel kernel, cl_uint first_index, cl_uint nargs, ... ) {
    va_list ap;
    cl_uint ind,e_ind;
    cl_int status;
    size_t s;
    void *p;
    
    va_start(ap, nargs);
    status = CL_SUCCESS;
    e_ind = first_index+nargs;
    for(ind=first_index; ind<e_ind; ind++)
    {
        s = va_arg(ap, size_t);
        p = va_arg(ap, void *);
        status |= clSetKernelArg(kernel, ind, s, p);
    }
    va_end(ap);
    
    return status;
}


