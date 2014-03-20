/*
 *  ljmd-trajclass.h
 *
 *
 *
 */

#ifndef _LJMD_TRAJCLASS_H_
#define _LJMD_TRAJCLASS_H_

#ifdef _USE_FLOAT
#define FPTYPE float
#else
#define FPTYPE double
#endif

#include <fstream>
using std::ifstream;
#include <string>
using std::string;


///    In the class define the Trajectory Frame 
class ljmdTrajectoryFrame{
public:
   /// Numbers of atoms
   int nAtoms;
   /// Positions
   FPTYPE **x; 
   /// Atoms label
   string *atLbl;   
   /// Minimum and Maximum Positions of atoms
   FPTYPE xmin[3],xmax[3]; 
   //*****************************************************************
   ljmdTrajectoryFrame();
   ~ljmdTrajectoryFrame();
   //*****************************************************************
   bool loadFrameFromFile(ifstream &ifil);
   //*****************************************************************
   void displayFrame(void);
   //*****************************************************************
   /// Display Minimum and Maximum Coordinates
   void displayMinMaxCoordinates(void);
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
private:
   //*****************************************************************
   /** This function reads information from ifil (a file) and allocates 
   memory */
   bool setupMe(ifstream &ifil); //This function reads information from
                   //ifil (a file) and allocates memory
   //*****************************************************************
   bool imsetup;
   //*****************************************************************
   /// Update Min and Max Valus Of Coordinates
   void updateMinMaxValsOfCoordinates(void);
   //*****************************************************************
   //*****************************************************************
   //*****************************************************************
};

#endif /* defined(_LJMD_TRAJCLASS_H_) */
