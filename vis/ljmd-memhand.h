/*
 *  ljmd-memhand.h
 *  
 *   Auxiliar functions to allocate/deallocate 1D, and 2D arrays
 *
 */

#ifndef _LJMD_MEMHANDLE_H_
#define _LJMD_MEMHANDLE_H_

#ifdef _USE_FLOAT
#define FPTYPE float
#else
#define FPTYPE double
#endif

#include <iostream>

#include <string>
using std::string;

//**************************************************************************************************
//**************************************************************************************************
bool alloc1DRealArray(string ptrname,const int n,FPTYPE* &thptr);
//**************************************************************************************************
bool alloc1DRealArray(string ptrname,const int n,FPTYPE* &thptr,const FPTYPE inval);
//**************************************************************************************************
bool dealloc1DRealArray(FPTYPE* &tp);
//**************************************************************************************************
bool alloc1DIntArray(string ptrname,const int n,int* &thptr,const int inval);
//**************************************************************************************************
bool alloc1DIntArray(string ptrname,const int n,int* &thptr);
//**************************************************************************************************
bool dealloc1DIntArray(int* &tp);
//**************************************************************************************************
bool alloc1DStringArray(string ptrname,const int n, string* &thptr);
//**************************************************************************************************
bool dealloc1DStringArray(string* &tp);
//**************************************************************************************************
bool alloc1DBoolArray(string ptrname,const int n,bool* &thptr);
//**************************************************************************************************
bool alloc1DBoolArray(string ptrname,const int n,bool* &thptr,const bool inval);
//**************************************************************************************************
bool dealloc1DBoolArray(bool* &tp);
//**************************************************************************************************
bool alloc2DRealArray(string ptrname,const int rows,const int cols,FPTYPE** &thptr);
//**************************************************************************************************
bool alloc2DRealArray(string ptrname,const int rows,const int cols,FPTYPE** &thptr,const FPTYPE inval);
//**************************************************************************************************
bool dealloc2DRealArray(FPTYPE** & tp,const int nc);
//**************************************************************************************************
bool alloc3DRealArray(string ptrname,const int idx1,const int idx2,const int idx3,FPTYPE*** &thptr);
//**************************************************************************************************
bool dealloc3DRealArray(FPTYPE*** & tp,const int idx1,const int idx2);
//**************************************************************************************************
bool alloc2DIntArray(string ptrname,const int rows,const int cols,int** &thptr,const int val);
//**************************************************************************************************
bool alloc2DIntArray(string ptrname,const int rows,const int cols,int** &thptr);
//**************************************************************************************************
bool dealloc2DIntArray(int** & tp,const int nc);
//**************************************************************************************************
#endif//_LJMD_MEMHANDLE_H_



