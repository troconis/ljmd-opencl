/*
 *  ljmd-trajclass.cpp
 *
 *
 *
 */

#ifndef _LJMD_TRAJCLASS_CPP_
#define _LJMD_TRAJCLASS_CPP_

#include "ljmd-trajclass.h"
#include "ljmd-memhand.h"

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

//*************************************************************************************
ljmdTrajectoryFrame::ljmdTrajectoryFrame(){
   nAtoms=0;
   x=NULL;
   atLbl=NULL;
   imsetup=false;
   for (int i=0; i<3; i++) {
      xmin[i]=1.0e+50;
      xmax[i]=-1.0e+50;
   }
   return;
}
//*************************************************************************************
ljmdTrajectoryFrame::~ljmdTrajectoryFrame()
{
   dealloc2DRealArray(x,nAtoms);
   dealloc1DStringArray(atLbl);
   return;
}
//*************************************************************************************

//*************************************************************************************
bool ljmdTrajectoryFrame::loadFrameFromFile(ifstream &ifil)
{
   if (!imsetup) {setupMe(ifil);}
   int nattmp;
   string str;
   ifil >> nattmp;
#if __DEBUG
   if (nattmp!=nAtoms) {
      cout << "Error while loading frame, the number of atoms changed from frame"
       << "\nto frame!" << endl;
      return false;
   }
#endif
   getline(ifil,str); //This reads an empty string (the pointer to the file is
                      //right after the number of atoms, and before the "\n" char
   getline(ifil,str); //This gets the line containing the energy, etc..
   for (int i=0; i<nAtoms; i++) {
      ifil >> atLbl[i];
      for (int j=0; j<3; j++) {ifil >> x[i][j];}
   }
   return true;
}
//*************************************************************************************
bool ljmdTrajectoryFrame::setupMe(ifstream &ifil)
{
   if (imsetup) {return imsetup;}
   ifil >> nAtoms;
   ifil.seekg(0);
   bool status=alloc2DRealArray(string("x"),nAtoms,3,x);
   status=(status&&(alloc1DStringArray(string("atLbl"),nAtoms,atLbl)));
   imsetup=status;
   loadFrameFromFile(ifil);
   if (imsetup) {updateMinMaxValsOfCoordinates();}
   ifil.seekg(0);
   return imsetup;
}
//*************************************************************************************
void ljmdTrajectoryFrame::displayFrame(void)
{
#if __DEBUG
   if (!imsetup) {
      cout << "First need to setup ljmdTrajectoryFrame object!" << endl;
   }
#endif
   for (int i=0; i<nAtoms; i++) {
      cout << atLbl[i];
      for (int j=0; j<3; j++) {cout << " " << x[i][j];}
      cout << endl;
   }
}
//*************************************************************************************
void ljmdTrajectoryFrame::updateMinMaxValsOfCoordinates(void)
{
#if __DEBUG
   if (!imsetup) {
      cout << "Error: Setup the ljmdTrajectory object first..." << endl;
      for (int i=0; i<3; i++) {
         xmin[i]=xmax[i]=0.0e0;
      }
      return;
   }
#endif
   for (int i=0; i<nAtoms; i++) {
      for (int j=0; j<3; j++) {
         if (xmin[j]>=x[i][j]) {xmin[j]=x[i][j];}
         if (xmax[j]<=x[i][j]) {xmax[j]=x[i][j];}
      }
   }
   return;
}
//*************************************************************************************
void ljmdTrajectoryFrame::displayMinMaxCoordinates()
{
   cout << "Minimum values of coordinates:";
   for (int i=0; i<3; i++) {cout << " " << xmin[i];}
   cout << "\nMaximum values of coordinates:";
   for (int i=0; i<3; i++) {cout << " " << xmax[i];}
   cout << endl;
   return;
}
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
#endif /* defined(_LJMD_TRAJCLASS_CPP_) */
