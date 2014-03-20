/*
 *  ljmd-povtools.h
 *  
 *
 *
 */

#ifndef _LJMD_POVTOOLS_H_
#define _LJMD_POVTOOLS_H_

#ifdef _USE_FLOAT
#define FPTYPE float
#else
#define FPTYPE double
#endif

#include <string>
using namespace std;
#include <fstream>
using std::ifstream;
using std::ofstream;

#ifndef SPTSIZEINDENT
#define SPTSIZEINDENT 2
#endif

//**********************************************************************************************
//**********************************************************************************************
/** 
 This class carries configuration numbers needed to produce *.pov files.
 Some functions are integrated to easy the drawing of spheres and cylinders to povray files.
 */
//**********************************************************************************************
class povRayConfProp
//**********************************************************************************************
{
public:
   //***********************************************************************************************
   int nLightSources;
   FPTYPE **lightSource;
   FPTYPE backGroundColor[3];
   FPTYPE angView;
   FPTYPE skyCam[3];
   FPTYPE locCam[3];
   FPTYPE lookAtCam[3];
   int currIndLev;
   FPTYPE defLightSource[3];
   bool shine,inccolors;
   //**********************************************************************************************
   povRayConfProp();
   //**********************************************************************************************
   ~povRayConfProp();
   //**********************************************************************************************
   /**
    This writes the header information to the file ofil. 
    By default, it places the camera. Calling the function with placecam=false will allows
    to place the camera by hand.
    */
   bool writeHeader(ofstream &ofil,bool placecam = true);
   //**********************************************************************************************
   /** 
    This function sets the background RGB numbers.
    */
   void setBGColor(FPTYPE rgbr,FPTYPE rgbg,FPTYPE rgbb);
   //**********************************************************************************************
   /** This set the angle (see http://www.povray.org/documentation/view/3.7.0/246/)  */
   void setAngView(FPTYPE av);
   //**********************************************************************************************
   /** This set the sky vector (see http://www.povray.org/documentation/view/3.7.0/246/)  */
   void setSkyCam(FPTYPE xx,FPTYPE yy,FPTYPE zz);
   //**********************************************************************************************
   /** This set the location for the camera 
    (see http://www.povray.org/documentation/view/3.7.0/246/)  */
   void setLocCam(FPTYPE xx,FPTYPE yy,FPTYPE zz);
   //**********************************************************************************************
   /** This set the lookAt position for the camera
    (see http://www.povray.org/documentation/view/3.7.0/246/)  */
   void setLookAtCam(FPTYPE xx,FPTYPE yy,FPTYPE zz);
   //**********************************************************************************************
   /** With this function one can add light sources 
    (see http://www.povray.org/documentation/view/3.6.0/308/) */
   void addLightSource(FPTYPE xx,FPTYPE yy,FPTYPE zz);
   //**********************************************************************************************
   /** This includes the color header for povray */
   void writeIncColors(ofstream &ofil);
   //**********************************************************************************************
   void writePlaceCamera(ofstream &ofil);
   //**********************************************************************************************
   /** This writes the Ligth sources into the pov file. intens gives the intensity of the 
    light source and opts is a string that will be passed directly inside the ligth source
    options. See http://www.povray.org/documentation/view/3.6.0/308/ for more details of how 
    to add light sources. The function is intended to put ligth sources by hand.
    If the light sources have been set using addLightSource, the function writeHeader will
    write them automatically.
    */
   void writeLightSource(ofstream &ofil,int is,FPTYPE intens,const char* opts);
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
   //**********************************************************************************************
};
//**********************************************************************************************
void writeIndTabs(ofstream &ofil, int nt);
//**********************************************************************************************
void writePoVVector(ofstream &ofil,FPTYPE xx,FPTYPE yy, FPTYPE zz);
//**********************************************************************************************
void writePoVVector(ofstream &ofil,int xx, int yy, int zz);
//**********************************************************************************************
string indTabsStr(int nt);
//**********************************************************************************************
//**********************************************************************************************
/** 
 This function writes the information to draw a sphere. ofil is the pov file; nt is the number
 of tabs to indent (each tab is actually written as 3 spaces); xx,yy, and zz are the 
 coordinates of the sphere's center; rr is the radius of the sphere; and cr, cg, cb are
 the rgb colors for the sphere.
 */
bool writePOVSphere(ofstream &ofil,int nt,
                 FPTYPE xx, FPTYPE yy, FPTYPE zz, FPTYPE rr,
                 FPTYPE cr, FPTYPE cg, FPTYPE cb);
//**********************************************************************************************
bool writePOVSphere(ofstream &ofil,int nt,
                    FPTYPE xx, FPTYPE yy, FPTYPE zz, FPTYPE rr,
                    const char *str);
//**********************************************************************************************
bool writePOVSphere(ofstream &ofil,int nt,
                    FPTYPE xx, FPTYPE yy, FPTYPE zz, const char* strrad,
                    const char *strcol);
//**********************************************************************************************
bool writePOVTransparentSphere(ofstream &ofil,int nt,
                    FPTYPE xx, FPTYPE yy, FPTYPE zz, FPTYPE rr,
                    FPTYPE cr, FPTYPE cg, FPTYPE cb,FPTYPE trc);
//**********************************************************************************************
bool writePOVTriangle(ofstream &ofil,int nt);
//**********************************************************************************************
bool writePOVCylinder(ofstream &ofil,int nt, 
                      FPTYPE xa, FPTYPE ya, FPTYPE za, 
                      FPTYPE xb, FPTYPE yb, FPTYPE zb, FPTYPE rr,
                      FPTYPE cr, FPTYPE cg, FPTYPE cb);
//**********************************************************************************************
bool writePOVCylinder(ofstream &ofil,int nt,
                      FPTYPE xa, FPTYPE ya, FPTYPE za,
                      FPTYPE xb, FPTYPE yb, FPTYPE zb, FPTYPE rr,
                      const char * str);
//**********************************************************************************************
bool writePOVArrow(ofstream &ofil,int nt);
//**********************************************************************************************
//**********************************************************************************************
//**********************************************************************************************
#endif//_LJMD_POVTOOLS_H_



