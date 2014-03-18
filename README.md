# LJMD-OpenCL

Repository for ICTP School-Lab. Collaborative development of OpenCL version of LJMD by Axel K.  

##Install our software

###Download Sources
In order to get the source code from our repository you need to jave git installed.
Then just type this in your command line.

	$ git clone https://github.com/LJMD-OpenCL/ljmd-opencl.git

###Install
	$ cd ljmd-opencl
	$ make
You will receive a executable called ljmd-CL in the same folder

###Test
In order to test the correct execution of our software type.

	$ make test

###Command line parameters
        ljmd-cl cpu|gpu[n] [nthread] <inpfile
        n: optional number of gpus to be used
        nthread: optional number of threads to be spawned
        inpfile: simulation parameters input file
        [a restart file (defined in inpfile) must be in
         the corresponding path]
