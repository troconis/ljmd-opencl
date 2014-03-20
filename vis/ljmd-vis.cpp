/*
 *  ljmd-vis.cpp
 *
 *
 *
 */

/** 
 
 */

#ifndef _LJMD_VIS_CPP_
#define _LJMD_VIS_CPP_

#ifdef _USE_FLOAT
#define FPTYPE float
#else
#define FPTYPE double
#endif

#define NUMOFZEROSFORIMAGENAME 5
#define DEFAULTIMAGETYPE "jpg"
#define DEFAULTATOMRADIUS 1.5e0
#define DEFAULTRGBRED 0.3e0
#define DEFAULTRGBGREEN 0.3e0
#define DEFAULTRGBBLUE 0.3e0
#define CAMERADISTFACTOR 2.0e0


#include <iostream>
using std::cout;
using std::endl;
using std::ios;
#include <sstream>
#include <fstream>
using std::ifstream;
using std::ofstream;
#include <cstdlib>
using std::exit;
#include <math.h>
#include <string>
using namespace std;
#include <iomanip>
using std::setprecision;
using std::scientific;
#include <cstdlib>
#include <stdlib.h> 


#include "ljmd-trajclass.h"
#include "ljmd-povtools.h"

string getStringFromInt(const int number,const int width);
/* This function returns a string with a fixed field width
 from an integer, the field is filled with zeros */

string getImageName(const string basename,const string extension,const int nf);
/* This function is self-descriptive */

void setupPovConfig(ljmdTrajectoryFrame &fm,povRayConfProp &pvp);
/* Setup the povray configuration object, using the data contained in the frame */

void writePovFile(const string povname,povRayConfProp &pvc,ljmdTrajectoryFrame &fr);
/* Self descriptive */

void putAtomsInPovFile(ofstream &pof,ljmdTrajectoryFrame &tf);
/* Function used by writePovFile to transform the coordinates of each atom
 into a pov sphere */

void displayHelp(char **argv);
/* This function displays the help for the main program */

int main (int argc, char ** argv){
   
   if (argc<2) {
      displayHelp(argv);
      exit(1);
   }
   
   ljmdTrajectoryFrame frame;
   ifstream infile;
   string povfname="tmppovfile.pov";
   infile.open(argv[1],ios::in);
   if (!infile.is_open()) {
      cout << "Error while opening the file \"" << argv[1] << "\"..." << endl;
      displayHelp(argv);
      exit(1);
   }
   int nFrame=0;
   
   frame.loadFrameFromFile(infile); //needed to setup the frame object
   infile.seekg(0); //This returns the file pointer to the begining
   //frame.displayMinMaxCoordinates();
   
   
   povRayConfProp povconf; //This object has all the configuration properties
                           //for creating a pov file, for instance
                           //the camera position, ligth sources, etc.
   
   setupPovConfig(frame,povconf);
   
   string basename="image",extname=DEFAULTIMAGETYPE;
   string cmdline;
   
   while (!infile.eof()) {
      frame.loadFrameFromFile(infile);
      nFrame++;
      cout << "Working on frame " << nFrame << "...\r";
      cout.flush();
      writePovFile(povfname,povconf,frame);
      cmdline="povray +FJ +O"+getImageName(basename,extname,nFrame)+" -D "+povfname;
#ifndef __DEBUG
      cmdline+=" >/dev/null 2>&1";
#endif
      system(cmdline.c_str());
   }
   cout << endl;
   
   //frame.displayMinMaxCoordinates();
   //frame.displayFrame();
   //cmdline="rm "+povfname;
   //system(cmdline.c_str());
   infile.close();
   
   /*
   //Activate for automatically calling mencoder from inside this program.
   //To activate, replace "/*" by "//*"
   cmdline="mencoder mf://*.jpg -mf w=600:h=400:fps=6:type=jpg -ovc lavc ";
   cmdline+="-lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o output.avi  >/dev/null 2>&1";
   system(cmdline.c_str());
   // */
   return 0;
}



string getStringFromInt(const int number,const int width)
{
   std::ostringstream numstr;
   numstr.str("");
   numstr.fill('0');
   numstr << std::setw(width) << number;
   return numstr.str();
}

string getImageName(const string basename,const string extension,const int nf)
{
   return (basename+getStringFromInt(nf,NUMOFZEROSFORIMAGENAME)+"."+extension);
}

void setupPovConfig(ljmdTrajectoryFrame &fm,povRayConfProp &pvp)
{
   FPTYPE rr1=0.0e0,rr2=0.0e0;
   for (int i=0; i<3; i++) {
      rr1+=fm.xmin[i]*fm.xmin[i];
      rr2+=fm.xmax[i]*fm.xmax[i];
   }
   rr1=sqrt(rr1);
   rr2=sqrt(rr2);
   FPTYPE rrmax=rr1;
   if (rr2>rr1) {rrmax=rr2;}
   pvp.setLocCam(rrmax*CAMERADISTFACTOR,0.0e0,0.0e0); //this puts the camera at a distance rrmax
   pvp.setLookAtCam(0.5*(fm.xmin[0]+fm.xmax[0]),\
                    0.5*(fm.xmin[1]+fm.xmax[1]),\
                    0.5*(fm.xmin[2]+fm.xmax[2])); //This makes the camera to look at the center
                                                  //of mass of the set of atoms
   pvp.setBGColor(0.7,0.7,0.7); //This sets the background color
   /* For adding light sources, use pvp.addLightSource(x,y,z); */
   pvp.addLightSource(1.5e0*rrmax,0.2e0,0.0e0);
   return;
}

void writePovFile(const string povname,povRayConfProp &pvc,ljmdTrajectoryFrame &fr)
{
   ofstream pof;
   pof.open(povname.c_str(),ios::out);
   pvc.writeHeader(pof);
   pof << "global_settings { ambient_light White }" << endl;
   putAtomsInPovFile(pof,fr);
   pof.close();
   return;
}

void putAtomsInPovFile(ofstream &pof,ljmdTrajectoryFrame &tf)
{
   int numtabs=0; //this is just to tab the output
   FPTYPE rgbr,rgbg,rgbb; //these are to set the color of the atoms
   rgbr=DEFAULTRGBRED;
   rgbg=DEFAULTRGBGREEN;
   rgbb=DEFAULTRGBBLUE;
   FPTYPE atomRad=DEFAULTATOMRADIUS;
   for (int i=0; i<tf.nAtoms; i++) {
      writePOVSphere(pof,numtabs,tf.x[i][0], tf.x[i][1], tf.x[i][2], atomRad,rgbr,rgbg,rgbb);
   }
   return;
}

void displayHelp(char **argv)
{
   cout << "Usage:\n\n   " << argv[0] << " trajectoryfilename\n" << endl;
   return;
}




#endif /* defined(_LJMD_VIS_CPP_) */