/** \mainpage Simple lennard-jones potential MD code with velocity verlet.
 
  OpenCL parallel baseline version.
  optimization 1: apply serial improvements except newtons 3rd law\n
  units: Length=Angstrom, Mass=amu; Energy=kcal

  \date Mar / 20  / 2014
  \author Ajasja Ljubetic
  \author Johannes Voss
  \author Moses Sokunbi
  \author Ivan Girotto
  \author Oscar Najera
  \author Rodrigo Neumann


  Install our software

  Download Sources
  In order to get the source code from our repository you need to jave git installed.
  Then just type this in your command line.

	$ git clone https://github.com/LJMD-OpenCL/ljmd-opencl.git

  Install
	$ cd ljmd-opencl
	$ make
  You will receive a executable called ljmd-CL in the same folder

  Test
  In order to test the correct execution of our software type.

	$ make test

  Command line parameters
        ljmd-cl cpu|gpu[n] [nthread] <inpfile n: optional number of gpus to be used
        nthread: optional number of threads to be spawned inpfile: simulation parameters\n 
        input file   [a restart file (defined in inpfile) must be in the corresponding path]
 */

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

/** generic file- or pathname buffer length */
#define BLEN 200

/** a few physical constants */
const FPTYPE kboltz=0.0019872067;     /* boltzman constant in kcal/mol/K */
const FPTYPE mvsq2e=2390.05736153349; /* m*v^2 in kcal/mol */

/** structure to hold the complete information
    about the MD system */
struct _mdsys {
    int natoms,nfi,nsteps;
    FPTYPE dt, mass, epsilon, sigma, box, rcut;
    FPTYPE ekin, epot, temp;
    FPTYPE *rx, *ry, *rz;
    FPTYPE *vx, *vy, *vz;
    FPTYPE *fx, *fy, *fz;
};
typedef struct _mdsys mdsys_t;

/** structure to hold the complete information
    about the MD system on a OpenCL device*/
struct _cl_mdsys {
    int natoms,nfi,nsteps;
    FPTYPE dt, mass, epsilon, sigma, box, rcut;
    FPTYPE ekin, epot, temp;
    cl_mem rx, ry, rz;
    cl_mem vx, vy, vz;
    cl_mem fx, fy, fz;
};
typedef struct _cl_mdsys cl_mdsys_t;

/** helper function: read a line and then return
   the first string with whitespace stripped off */
static int get_me_a_line(FILE *fp, char *buf)
{
    char tmp[BLEN], *ptr;

    /** read a line and cut of comments and blanks */
    if (fgets(tmp,BLEN,fp)) {
        int i;

        ptr=strchr(tmp,'#');
        if (ptr) *ptr= '\0';
        i=strlen(tmp); --i;
        while(isspace(tmp[i])) {
            tmp[i]='\0';
            --i;
        }
        ptr=tmp;
        while(isspace(*ptr)) {++ptr;}
        i=strlen(ptr);
        strcpy(buf,tmp);
        return 0;
    } else {
        perror("problem reading input");
        return -1;
    }
    return 0;
}

void PrintUsageAndExit() {
    fprintf( stderr, "\nError. Run the program as follow: ");
    fprintf( stderr, "\n./ljmd-cl.x device [thread-number] < input ");
    fprintf( stderr, "\ndevice = cpu | gpu \n\n" );
    exit(1);
}

/** append data to output. */
static void output(mdsys_t *sys, FILE *erg, FILE *traj)
{
    int i;

    printf("% 8d % 20.8f % 20.8f % 20.8f % 20.8f\n", sys->nfi, sys->temp, sys->ekin, sys->epot, sys->ekin+sys->epot);
    fprintf(erg,"% 8d % 20.8f % 20.8f % 20.8f % 20.8f\n", sys->nfi, sys->temp, sys->ekin, sys->epot, sys->ekin+sys->epot);
    fprintf(traj,"%d\n nfi=%d etot=%20.8f\n", sys->natoms, sys->nfi, sys->ekin+sys->epot);
    for (i=0; i<sys->natoms; ++i) {
      fprintf(traj, "Ar  %20.8f %20.8f %20.8f\n", sys->rx[i], sys->ry[i], sys->rz[i]);
    }
}





/** main */
int main(int argc, char **argv)
{
  /** OpenCL variables */
  cl_device_id *devices=NULL;
  cl_device_type device_type; /*to test if we are on cpu or gpu*/
  cl_context *contexts=NULL;
  cl_command_queue *cmdQueues=NULL;

  /** The event variables are created only when needed */
#ifdef _UNBLOCK
  cl_event *event;
#endif

  cl_event *force_event;

  FPTYPE * buffers[6];
  cl_mdsys_t *cl_sys;
  cl_int status;
  cl_uint ndevices;

  int nprint, i, nthreads = 0;
  char restfile[BLEN], trajfile[BLEN], ergfile[BLEN], line[BLEN];
  FILE *fp,*traj,*erg;
  mdsys_t sys;
  cl_uint u, nforce, *firstatoms, *natoms;


/** Start profiling */

#ifdef __PROFILING

  double t1, t2;

  t1 = second();

#endif

  /** handling the command line arguments */
  switch (argc) {
      case 2: /** only the cpu/gpu argument was passed, setting default nthreads */
	      if( !strcmp( argv[1], "cpu" ) ) nthreads = 16;
	      else nthreads = 1024;
	      break;
      case 3: /** both the device type (cpu/gpu) and the number of threads were passed */
	      nthreads = strtol(argv[2],NULL,10);
	      if( nthreads<0 ) {
		      fprintf( stderr, "\n. The number of threads must be more than 1.\n");
		      PrintUsageAndExit();
	      }
	      break;
      default:
	      PrintUsageAndExit();
	      break;
  }

  /** Initialize the OpenCL environment */
  if( InitOpenCLEnvironment( argv[1], &devices, &contexts, &cmdQueues, &ndevices ) != CL_SUCCESS ){
    fprintf( stderr, "Program Error! OpenCL Environment was not initialized correctly.\n" );
    return 4;
  }

  /** The event initialization is performed only when needed */
  if(!(cl_sys = (cl_mdsys_t *) malloc(sizeof(cl_mdsys_t)*ndevices))) {
    fprintf( stderr, "Cannot allocate memory of cl_sys copies.\n");
    return 5;
  }

#ifdef _UNBLOCK
  /** initialize the cl_event handler variables */
  event = (cl_event *) alloca(sizeof(cl_event)*(ndevices+2));
  for( i = 0; i < 3; ++i) {
	  event[i] = clCreateUserEvent( contexts[0], NULL );
	  clSetUserEventStatus( event[i], CL_COMPLETE );
  }
  for( i = 3; i < ndevices+2; ++i) {
	  event[i] = clCreateUserEvent( contexts[i-2], NULL );
	  clSetUserEventStatus( event[i], CL_COMPLETE );
  }
#endif

  /** read input file */
  if(get_me_a_line(stdin,line)) return 1;
  sys.natoms=atoi(line);
  if(get_me_a_line(stdin,line)) return 1;
  sys.mass=atof(line);
  if(get_me_a_line(stdin,line)) return 1;
  sys.epsilon=atof(line);
  if(get_me_a_line(stdin,line)) return 1;
  sys.sigma=atof(line);
  if(get_me_a_line(stdin,line)) return 1;
  sys.rcut=atof(line);
  if(get_me_a_line(stdin,line)) return 1;
  sys.box=atof(line);
  if(get_me_a_line(stdin,restfile)) return 1;
  if(get_me_a_line(stdin,trajfile)) return 1;
  if(get_me_a_line(stdin,ergfile)) return 1;
  if(get_me_a_line(stdin,line)) return 1;
  sys.nsteps=atoi(line);
  if(get_me_a_line(stdin,line)) return 1;
  sys.dt=atof(line);
  if(get_me_a_line(stdin,line)) return 1;
  nprint=atoi(line);


  /** allocate memory */
  for(u = 0; u < ndevices; u++) {
    cl_sys[u].natoms = sys.natoms;
    cl_sys[u].rx = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].ry = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].rz = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].vx = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].vy = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].vz = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].fx = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].fy = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
    cl_sys[u].fz = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR, cl_sys[u].natoms * sizeof(FPTYPE), NULL, &status );
  }

  /// positions and velocities
  buffers[0] = (FPTYPE *) malloc( 2 * cl_sys[0].natoms * sizeof(FPTYPE) );
  buffers[1] = (FPTYPE *) malloc( 2 * cl_sys[0].natoms * sizeof(FPTYPE) );
  buffers[2] = (FPTYPE *) malloc( 2 * cl_sys[0].natoms * sizeof(FPTYPE) );
  //forces
  buffers[3] = (FPTYPE *) malloc( cl_sys[0].natoms * sizeof(FPTYPE) );
  buffers[4] = (FPTYPE *) malloc( cl_sys[0].natoms * sizeof(FPTYPE) );
  buffers[5] = (FPTYPE *) malloc( cl_sys[0].natoms * sizeof(FPTYPE) );

  /** read restart */
  fp = fopen( restfile, "r" );
  if( fp ) {
    for( i = 0; i < 2 * cl_sys[0].natoms; ++i ){
#ifdef _USE_FLOAT
      fscanf( fp, "%f%f%f", buffers[0] + i, buffers[1] + i, buffers[2] + i);
#else
      fscanf( fp, "%lf%lf%lf", buffers[0] + i, buffers[1] + i, buffers[2] + i);
#endif
    }

    for( u = 0; u < ndevices; u++ ) {
      status = clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].rx, CL_TRUE, 0, cl_sys[u].natoms * sizeof(FPTYPE), buffers[0], 0, NULL, NULL );
      status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].ry, CL_TRUE, 0, cl_sys[u].natoms * sizeof(FPTYPE), buffers[1], 0, NULL, NULL );
      status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].rz, CL_TRUE, 0, cl_sys[u].natoms * sizeof(FPTYPE), buffers[2], 0, NULL, NULL );

      status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].vx, CL_TRUE, 0, cl_sys[u].natoms * sizeof(FPTYPE), buffers[0] + cl_sys[u].natoms, 0, NULL, NULL );
      status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].vy, CL_TRUE, 0, cl_sys[u].natoms * sizeof(FPTYPE), buffers[1] + cl_sys[u].natoms, 0, NULL, NULL );
      status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].vz, CL_TRUE, 0, cl_sys[u].natoms * sizeof(FPTYPE), buffers[2] + cl_sys[u].natoms, 0, NULL, NULL );
    }

    fclose(fp);

  } else {
    perror("cannot read restart file");
    return 3;
  }

  /** initialize forces and energies.*/
  sys.nfi=0;

  size_t globalWorkSize[1];
  globalWorkSize[0] = nthreads;

  const char * sourcecode =
  #include <opencl_kernels_as_string.h>
  ;

  cl_program *program = (cl_program *) alloca(sizeof(cl_program)*ndevices);
  cl_kernel *kernel_force = (cl_kernel *) alloca(sizeof(cl_kernel)*ndevices);
  cl_kernel *kernel_ekin = (cl_kernel *) alloca(sizeof(cl_kernel)*ndevices);
  cl_kernel *kernel_verlet_first = (cl_kernel *) alloca(sizeof(cl_kernel)*ndevices);
  cl_kernel *kernel_verlet_second = (cl_kernel *) alloca(sizeof(cl_kernel)*ndevices);
  cl_kernel *kernel_azzero = (cl_kernel *) alloca(sizeof(cl_kernel)*ndevices);

  for(u = 0; u < ndevices; u++) {
    program[u] = clCreateProgramWithSource( contexts[u], 1, (const char **) &sourcecode, NULL, &status );

    status |= clBuildProgram( program[u], 0, NULL, kernelflags, NULL, NULL );

#ifdef __DEBUG
  size_t log_size;
  char log [200000];
  clGetProgramBuildInfo( program[u], devices[u], CL_PROGRAM_BUILD_LOG, sizeof(log), log, &log_size );
  fprintf( stderr, "\nLog: \n\n %s", log );
#endif

    kernel_force[u] = clCreateKernel( program[u], "opencl_force", &status );
    kernel_ekin[u] = clCreateKernel( program[u], "opencl_ekin", &status );
    kernel_verlet_first[u] = clCreateKernel( program[u], "opencl_verlet_first", &status );
    kernel_verlet_second[u] = clCreateKernel( program[u], "opencl_verlet_second", &status );
    kernel_azzero[u] = clCreateKernel( program[u], "opencl_azzero", &status );

  }

  FPTYPE ** tmp_epot;
  cl_mem *epot_buffer = (cl_mem *) alloca(sizeof(epot_buffer)*ndevices);
  tmp_epot = (FPTYPE **) malloc( sizeof(void *) * ndevices );
  for( u = 0; u < ndevices; u++)
    tmp_epot[u] = (FPTYPE *) malloc( nthreads * sizeof(FPTYPE) );
  for(u = 0; u < ndevices; u++)
    epot_buffer[u] = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, nthreads * sizeof(FPTYPE), NULL, &status );


  /** precompute some constants */
  FPTYPE c12 = 4.0 * sys.epsilon * pow( sys.sigma, 12.0);
  FPTYPE c6  = 4.0 * sys.epsilon * pow( sys.sigma, 6.0);
  FPTYPE rcsq = sys.rcut * sys.rcut;
  FPTYPE boxby2 = HALF * sys.box;
  FPTYPE dtmf = HALF * sys.dt / mvsq2e / sys.mass;
  sys.epot = ZERO;
  sys.ekin = ZERO;

  FPTYPE ** tmp_ekin;
  cl_mem *ekin_buffer = (cl_mem *) alloca(sizeof(cl_mem)*ndevices);
  tmp_ekin = (FPTYPE **) malloc( sizeof(void *) * ndevices );
  for( u = 0; u < ndevices; u++)
    tmp_ekin[u] = (FPTYPE *) malloc( nthreads * sizeof(FPTYPE) );

  ///determine how many force vectors to calculate per gpu
  nforce = sys.natoms / ndevices;

  firstatoms = (cl_uint *) alloca(sizeof(cl_uint) * ndevices);
  natoms = (cl_uint *) alloca(sizeof(cl_uint) * ndevices);

  for( u = 0; u < ndevices-1 ; u++) {
    firstatoms[u] = u * nforce;
    natoms[u] = nforce;
  }
  ///last gpu gets a few more atoms if it doesn't match
  firstatoms[ndevices-1] = (ndevices-1)*nforce;
  natoms[ndevices-1] = sys.natoms - firstatoms[ndevices-1];

  for( u = 0; u < ndevices; u++) {
  /** Azzero force buffer */
    status = clSetMultKernelArgs( kernel_azzero[u], 0, 4, KArg(cl_sys[u].fx), KArg(cl_sys[u].fy), KArg(cl_sys[u].fz), KArg(cl_sys[u].natoms));


    status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_azzero[u], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );

    status |= clSetMultKernelArgs( kernel_force[u], 0, 15,
	  KArg(cl_sys[u].fx),
	  KArg(cl_sys[u].fy),
	  KArg(cl_sys[u].fz),
	  KArg(cl_sys[u].rx),
	  KArg(cl_sys[u].ry),
	  KArg(cl_sys[u].rz),
	  KArg(cl_sys[u].natoms),
	  KArg(epot_buffer[u]),
	  KArg(c12),
	  KArg(c6),
	  KArg(rcsq),
	  KArg(boxby2),
	  KArg(sys.box),
	  KArg(firstatoms[u]),
	  KArg(natoms[u]));


    status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_force[u], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );

    status |= clEnqueueReadBuffer( cmdQueues[u], epot_buffer[u], CL_TRUE, 0, nthreads * sizeof(FPTYPE), tmp_epot[u], 0, NULL, NULL );
  }

  for( u = 0; u < ndevices; u++ )
    for( i = 0; i < nthreads; i++) sys.epot += tmp_epot[u][i];

  for( u = 0; u < ndevices; u++ ) {
    ekin_buffer[u] = clCreateBuffer( contexts[u], CL_MEM_READ_WRITE, nthreads * sizeof(FPTYPE), NULL, &status );

    status |= clSetMultKernelArgs( kernel_ekin[u], 0, 5, KArg(cl_sys[u].vx), KArg(cl_sys[u].vy), KArg(cl_sys[u].vz),
      KArg(cl_sys[u].natoms), KArg(ekin_buffer[u]));

    status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_ekin[u], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );
  }

  status |= clEnqueueReadBuffer( cmdQueues[0], ekin_buffer[0], CL_TRUE, 0, nthreads * sizeof(FPTYPE), tmp_ekin[0], 0, NULL, NULL );

  for( i = 0; i < nthreads; i++) sys.ekin += tmp_ekin[0][i];
  sys.ekin *= HALF * mvsq2e * sys.mass;
  sys.temp  = TWO * sys.ekin / ( THREE * sys.natoms - THREE ) / kboltz;

  erg=fopen(ergfile,"w");
  traj=fopen(trajfile,"w");

  printf("Starting simulation with %d atoms for %d steps.\n",sys.natoms, sys.nsteps);
  printf("     NFI            TEMP            EKIN                 EPOT              ETOT\n");

  /** download data on host */
  status = clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].rx, CL_TRUE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[0], 0, NULL, NULL );
  status |= clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].ry, CL_TRUE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[1], 0, NULL, NULL );
  status |= clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].rz, CL_TRUE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[2], 0, NULL, NULL );

  sys.rx = buffers[0];
  sys.ry = buffers[1];
  sys.rz = buffers[2];

  output(&sys, erg, traj);

  if( ndevices > 1 ) {
    force_event = (cl_event *) alloca(sizeof(cl_event)*ndevices);
    for( u = 0; u < ndevices; u++ ) {
	  force_event[u] = clCreateUserEvent( contexts[u], NULL );
	  clSetUserEventStatus( force_event[u], CL_COMPLETE );
    }


    /// download force fragments and distribute them among gpus
    for( u = 0 ; u < ndevices; u++ ) {
        status |= clEnqueueReadBuffer( cmdQueues[u], cl_sys[u].fx, CL_FALSE, 0, natoms[u] * sizeof(FPTYPE), buffers[3] + firstatoms[u], 0, NULL, NULL );
        status |= clEnqueueReadBuffer( cmdQueues[u], cl_sys[u].fy, CL_FALSE, 0, natoms[u] * sizeof(FPTYPE), buffers[4] + firstatoms[u], 0, NULL, NULL );
        status |= clEnqueueReadBuffer( cmdQueues[u], cl_sys[u].fz, CL_FALSE, 0, natoms[u] * sizeof(FPTYPE), buffers[5] + firstatoms[u], 0, force_event+u, NULL );
    }

    clWaitForEvents(ndevices, force_event);

    for( u = 0 ; u < ndevices; u++ ) {
        status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].fx, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[3], 0, NULL, NULL );
        status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].fy, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[4], 0, NULL, NULL );
        status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].fz, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[5], 0, force_event+u, NULL );
    }

    clWaitForEvents(ndevices, force_event);
  }

  /**************************************************/
  /** main MD loop */
  for(sys.nfi=1; sys.nfi <= sys.nsteps; ++sys.nfi) {

    /** propagate system and recompute energies */
    /** 2) verlet_first   */
    for( u = 0; u < ndevices; u++ ) {
      status |= clSetMultKernelArgs( kernel_verlet_first[u], 0, 12,
        KArg(cl_sys[u].fx),
        KArg(cl_sys[u].fy),
        KArg(cl_sys[u].fz),
        KArg(cl_sys[u].rx),
        KArg(cl_sys[u].ry),
        KArg(cl_sys[u].rz),
        KArg(cl_sys[u].vx),
        KArg(cl_sys[u].vy),
        KArg(cl_sys[u].vz),
        KArg(cl_sys[u].natoms),
        KArg(sys.dt),
        KArg(dtmf));
      CheckSuccess(status, 2);

    /** When the data transfer is non blocking, this kernel has to wait the completion of part 8 (event[2]) */
#ifdef _UNBLOCK
      status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_verlet_first[u], 1, NULL, globalWorkSize, NULL, 1, &event[1], NULL );
#else
      status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_verlet_first[u], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );
#endif
    }

    /** 6) download position@device to position@host */
    if ((sys.nfi % nprint) == nprint-1) {

    /** In non blocking mode (CL_FALSE) this data transfer raises events[i] */
#ifdef _UNBLOCK
	status  = clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].rx, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[0], 0, NULL, NULL );
	status |= clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].ry, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[1], 0, NULL, NULL );
	status |= clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].rz, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[2], 0, NULL, &event[0] );
#else
	status  = clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].rx, CL_TRUE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[0], 0, NULL, NULL );
	status |= clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].ry, CL_TRUE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[1], 0, NULL, NULL );
	status |= clEnqueueReadBuffer( cmdQueues[0], cl_sys[0].rz, CL_TRUE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[2], 0, NULL, NULL );
#endif
	CheckSuccess(status, 6);
    }

    /** 3) force */
    for( u = 0; u < ndevices; u++) {
      status |= clSetMultKernelArgs( kernel_force[u], 0, 15,
        KArg(cl_sys[u].fx),
        KArg(cl_sys[u].fy),
        KArg(cl_sys[u].fz),
        KArg(cl_sys[u].rx),
        KArg(cl_sys[u].ry),
        KArg(cl_sys[u].rz),
        KArg(cl_sys[u].natoms),
        KArg(epot_buffer[u]),
        KArg(c12),
        KArg(c6),
        KArg(rcsq),
        KArg(boxby2),
        KArg(sys.box),
        KArg(firstatoms[u]),
        KArg(natoms[u]));

      CheckSuccess(status, 3);
      status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_force[u], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );
    }
    /// download force fragments and distribute them among gpus
    if( ndevices > 1 ) {
      for( u = 0 ; u < ndevices; u++ ) {
        status |= clEnqueueReadBuffer( cmdQueues[u], cl_sys[u].fx, CL_FALSE, 0, natoms[u] * sizeof(FPTYPE), buffers[3] + firstatoms[u], 0, NULL, NULL );
        status |= clEnqueueReadBuffer( cmdQueues[u], cl_sys[u].fy, CL_FALSE, 0, natoms[u] * sizeof(FPTYPE), buffers[4] + firstatoms[u], 0, NULL, NULL );
        status |= clEnqueueReadBuffer( cmdQueues[u], cl_sys[u].fz, CL_FALSE, 0, natoms[u] * sizeof(FPTYPE), buffers[5] + firstatoms[u], 0, force_event+u, NULL );
      }

      clWaitForEvents(ndevices, force_event);

      for( u = 0 ; u < ndevices; u++ ) {
        status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].fx, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[3], 0, NULL, NULL );
        status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].fy, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[4], 0, NULL, NULL );
        status |= clEnqueueWriteBuffer( cmdQueues[u], cl_sys[u].fz, CL_FALSE, 0, cl_sys[0].natoms * sizeof(FPTYPE), buffers[5], 0, force_event+u, NULL );
      }

      clWaitForEvents(ndevices, force_event);
    }


    /** 7) download E_pot[i]@device and perform reduction to E_pot@host */
    if ((sys.nfi % nprint) == nprint-1) {

    /** In non blocking mode (CL_FALSE) this data transfer kernel raises an event[1] */
    for( u = 0; u < ndevices; u++) {
#ifdef _UNBLOCK
	    status |= clEnqueueReadBuffer( cmdQueues[u], epot_buffer[u], CL_FALSE, 0, nthreads * sizeof(FPTYPE), tmp_epot[u], 0, NULL, &event[u+2] );
#else
	    status |= clEnqueueReadBuffer( cmdQueues[u], epot_buffer[u], CL_TRUE, 0, nthreads * sizeof(FPTYPE), tmp_epot[u], 0, NULL, NULL );
#endif
	    CheckSuccess(status, 7);
	  }
    }

    /** 4) verlet_second */
    for( u = 0; u < ndevices; u++) {
      status |= clSetMultKernelArgs( kernel_verlet_second[u], 0, 9,
        KArg(cl_sys[u].fx),
        KArg(cl_sys[u].fy),
        KArg(cl_sys[u].fz),
        KArg(cl_sys[u].vx),
        KArg(cl_sys[u].vy),
        KArg(cl_sys[u].vz),
        KArg(cl_sys[u].natoms),
        KArg(sys.dt),
        KArg(dtmf));

      CheckSuccess(status, 4);
      status = clEnqueueNDRangeKernel( cmdQueues[u], kernel_verlet_second[u], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );
    }

    if ((sys.nfi % nprint) == nprint-1) {

	/** 5) ekin */
	status |= clSetMultKernelArgs( kernel_ekin[0], 0, 5, KArg(cl_sys[0].vx), KArg(cl_sys[0].vy), KArg(cl_sys[0].vz),
			KArg(cl_sys[0].natoms), KArg(ekin_buffer[0]));
	CheckSuccess(status, 5);
	status = clEnqueueNDRangeKernel( cmdQueues[0], kernel_ekin[0], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL );


	/** 8) download E_kin[i]@device and perform reduction to E_kin@host */
	/** In non blocking mode (CL_FALSE) this data transfer kernel raises an event[2] */
#ifdef _UNBLOCK
	status |= clEnqueueReadBuffer( cmdQueues[0], ekin_buffer[0], CL_FALSE, 0, nthreads * sizeof(FPTYPE), tmp_ekin[0], 0, NULL, &event[2] );
#else
	status |= clEnqueueReadBuffer( cmdQueues[0], ekin_buffer[0], CL_TRUE, 0, nthreads * sizeof(FPTYPE), tmp_ekin[0], 0, NULL, NULL );
#endif
	CheckSuccess(status, 8);
    }

    /** 1) write output every nprint steps */
    if ((sys.nfi % nprint) == 0) {

    /** Calling a synchronization function (only when in non blocking mode) that will wait until all the
     * events[i], related to the data transfers, to be completed */
#ifdef _UNBLOCK
        clWaitForEvents(ndevices+2, event);
#endif
	sys.rx = buffers[0];
	sys.ry = buffers[1];
	sys.rz = buffers[2];

	/** initialize the sys.epot@host and sys.ekin@host variables to ZERO */
	sys.epot = ZERO;
	sys.ekin = ZERO;

	/** reduction on the tmp_Exxx[i] buffers downloaded from the device
	 * during parts 7 and 8 of the previous MD loop iteration */
	for( u = 0; u < ndevices; u++)
	    for( i = 0; i < nthreads; i++)
		  sys.epot += tmp_epot[u][i];
	for( i = 0; i < nthreads; i++)
		sys.ekin += tmp_ekin[0][i];

	/** multiplying the kinetic energy by prefactors */
	sys.ekin *= HALF * mvsq2e * sys.mass;
	sys.temp  = TWO * sys.ekin / ( THREE * sys.natoms - THREE ) / kboltz;

	/** writing output files (positions, energies and temperature) */
	output(&sys, erg, traj);
    }

  }
  /**************************************************/

/* End profiling */

#ifdef __PROFILING

t2 = second();

fprintf( stdout, "\n\nTime of execution = %.3g (seconds)\n", (t2 - t1) );

#endif






  /** clean up: close files, free memory */
  printf("Simulation Done.\n");
  fclose(erg);
  fclose(traj);

  free(buffers[0]);
  free(buffers[1]);
  free(buffers[2]);
  free(buffers[3]);

  free(cl_sys);
  free(cmdQueues);
  free(contexts);
  free(devices);

  return 0;
}
