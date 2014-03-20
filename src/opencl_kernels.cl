
#ifdef _USE_FLOAT
#define FPTYPE float
#define ZERO    0.0f
#define HALF    0.5f
#define ONE     1.0f
#define SIX     6.0f
#define TWELVE 12.0f
#else
#pragma OPENCL EXTENSION cl_khr_fp64: enable
#define FPTYPE double
#define ZERO    0.0
#define HALF    0.5
#define ONE     1.0
#define SIX     6.0
#define TWELVE 12.0
#endif

__kernel void opencl_azzero(  __global FPTYPE * a, __global FPTYPE * b, __global FPTYPE * c, const int natoms ) {
	 
  int nths = get_global_size( 0 );
  int id_th = get_global_id( 0 );
  int loc_id = id_th;
    
  while( loc_id < natoms ) {

     a[ loc_id ] = ZERO;
     b[ loc_id ] = ZERO;
     c[ loc_id ] = ZERO;	

     loc_id += nths;
  }

} 	 
/** Calculate kinetic energy */
__kernel void opencl_ekin(  __global FPTYPE * vx, __global FPTYPE * vy, __global FPTYPE * vz, const int natoms, __global FPTYPE * ekin ) {

  int nths = get_global_size( 0 );
  int id_th = get_global_id( 0 );
  int loc_id = id_th;

  ekin[id_th] = ZERO;
    
  while( loc_id < natoms ) {

    ekin[id_th] += vx[loc_id] * vx[loc_id] + vy[loc_id] * vy[loc_id] + vz[loc_id] * vz[loc_id];

    loc_id += nths;
  }
  //    sys->ekin *= 0.5*mvsq2e*sys->mass;
  //    sys->temp  = 2.0*sys->ekin/(3.0*sys->natoms-3.0)/kboltz;
}

/// Boundary conditions
inline FPTYPE pbc(FPTYPE x, const FPTYPE boxby2, const FPTYPE box)
{
    while (x >  boxby2) x -= box;
    while (x < -boxby2) x += box;
    return x;
}

/** Calculate the opencl force */
__kernel void opencl_force( __global FPTYPE * fx, __global FPTYPE * fy, __global FPTYPE * fz, __global FPTYPE * rx, __global FPTYPE * ry, __global FPTYPE * rz,  __global FPTYPE * vx, __global FPTYPE * vy, __global FPTYPE * vz, const FPTYPE ksi, const FPTYPE mass, const int natoms, __global FPTYPE * epot, const FPTYPE c12, const FPTYPE c6, const FPTYPE rcsq, const FPTYPE boxby2, const FPTYPE box, const int atom1, const int natoms1 ){
  int nths = get_global_size( 0 );
  int id_th = get_global_id( 0 );
  int loc_id;

  /** zero energy and forces */
  epot[id_th] = ZERO;

  loc_id = id_th;
  while( loc_id < natoms1 ){

    fx[ loc_id ] = ZERO;
    fy[ loc_id ] = ZERO;
    fz[ loc_id ] = ZERO;
    loc_id += nths;
  }
  
  loc_id = id_th;
  while( loc_id < natoms1  ) {

    int j,k;
    FPTYPE rx1, ry1, rz1;
    k = loc_id+atom1;
    rx1 = rx[k];
    ry1 = ry[k];
    rz1 = rz[k];
    
    for( j = 0; j < natoms; ++j ) {

      FPTYPE loc_rx, loc_ry, loc_rz, rsq;
      
      /** particles have no interactions with themselves */
      if ( k == j) continue;
      
      /** get distance between particle i and j */
      loc_rx = pbc(rx1 - rx[j], boxby2, box);
      loc_ry = pbc(ry1 - ry[j], boxby2, box);
      loc_rz = pbc(rz1 - rz[j], boxby2, box);
      rsq = loc_rx * loc_rx + loc_ry * loc_ry + loc_rz * loc_rz;
      
      /** compute force and energy if within cutoff */
      if (rsq < rcsq) {
  	FPTYPE r6, rinv, ffac;
	
  	rinv = ONE / rsq;
  	r6 = rinv * rinv * rinv;
        
  	ffac = ( TWELVE * c12 * r6 - SIX * c6 ) * r6 * rinv;
  	epot[id_th] += HALF * r6 * ( c12 * r6 - c6 );
	
/*  	fx[loc_id] += loc_rx * ffac;
  	fy[loc_id] += loc_ry * ffac;
  	fz[loc_id] += loc_rz * ffac;*/
	/*calculate forces incorporating thermostat*/
  	fx[loc_id] += loc_rx * ffac  -  ksi*mass*vx[loc_id]; 
  	fy[loc_id] += loc_ry * ffac  -  ksi*mass*vy[loc_id];
  	fz[loc_id] += loc_rz * ffac  -  ksi*mass*vz[loc_id];

      }
    }

    loc_id += nths;
  }

}

/** opencl verlet fisrt step*/
__kernel void opencl_verlet_first( __global FPTYPE * fx, __global FPTYPE * fy, __global FPTYPE * fz, __global FPTYPE * rx, __global FPTYPE * ry, __global FPTYPE * rz, __global FPTYPE * vx, __global FPTYPE * vy, __global FPTYPE * vz, const FPTYPE lambda, const int natoms, const FPTYPE dt, const FPTYPE dtmf) {

  int nths = get_global_size( 0 );
  int id_th = get_global_id( 0 );
  int loc_id = id_th;

  /** first part: propagate velocities by half and positions by full step */
  while( loc_id < natoms ){
  
    vx[loc_id] += dtmf * fx[loc_id];
    vy[loc_id] += dtmf * fy[loc_id];
    vz[loc_id] += dtmf * fz[loc_id];
    vx[loc_id] *= lambda;
    vy[loc_id] *= lambda;
    vz[loc_id] *= lambda;
    rx[loc_id] += dt*vx[loc_id];
    ry[loc_id] += dt*vy[loc_id];
    rz[loc_id] += dt*vz[loc_id];
  
    loc_id += nths;
  }
}


/** opencl verlet second step*/
__kernel void opencl_verlet_second( __global FPTYPE * fx, __global FPTYPE * fy, __global FPTYPE * fz, __global FPTYPE * vx, __global FPTYPE * vy, __global FPTYPE * vz, const int natoms, const FPTYPE dt, const FPTYPE dtmf) {

  int nths = get_global_size( 0 );
  int id_th = get_global_id( 0 );
  int loc_id = id_th;

  /** second part: propagate velocities by another half step */
  while( loc_id < natoms ){

    vx[loc_id] += dtmf * fx[loc_id];
    vy[loc_id] += dtmf * fy[loc_id];
    vz[loc_id] += dtmf * fz[loc_id];
    
    loc_id += nths;
  }
}


