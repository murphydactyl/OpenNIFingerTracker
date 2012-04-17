////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//  FindIsoline.h
//  Fast implementation of marching squares
//
//
//  AUTHOR:   Murphy Stein
//            New York University
//  CREATED:  Jan 2012
//
//  LICENSE:  BSD
//
//  Copyright (c) 2012 New York University.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms are permitted
//  provided that the above copyright notice and this paragraph are
//  duplicated in all such forms and that any documentation,
//  advertising materials, and other materials related to such
//  distribution and use acknowledge that the software was developed
//  by New York Univserity.  The name of the
//  University may not be used to endorse or promote products derived
//  from this software without specific prior written permission.
//  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef OpenNIUserTest_fincontours_h
#define OpenNIUserTest_fincontours_h
namespace fiso {
   void init(int width, int height);
   void setThreshold(double t);
   int find(double *in);
   
   double sample(double x, double y, double *pix);
   double t(int A, int B);
   int* getContourLengths(void);
   int getContourLength(int contour);
   int getNumContours(void);
   int getLastIndex(void);
   void setContourX(int contour, int v, double x);
   void setContourY(int contour, int v, double y);
   double getContourX(int contour, int v);
   double getContourY(int contour, int v);
   double getContourAngle(int contour, int v);
   int getValidIndex(int contour, int v);
   double getBBMinX(int contour);
   double getBBMaxX(int contour);
   double getBBMinY(int contour);
   double getBBMaxY(int contour);
   
   /* measurement functions */
   double measureSegmentArea(int contour, int first, int last);
   double measureContourArea(int contour);
   double measureMeanX(int contour);
   double measureMeanY(int contour);
   double measureSegmentLength(int contour, int first, int last);
   double measureSegmentPerimeter(int contour, int first, int last);
   double measureContourPerimeter(int contour);
   double measureNormalX(int contour, int i);
   double measureNormalY(int contour, int i);
   double measureCurvature(int contour, int first, int last);
   double measureSegmentNormalY(int contour, int first, int last);
   double measureSegmentNormalX(int contour, int first, int last);
   double measureSegmentAngleChange(int contour, int first, int last);
   double measureSignedSegementAngleChangedSquared(int contour, int first, int last);
   double measureEndpointDistance(int contour, int first, int second);
   double measureLength(int contour, int i);
   double measureRoundness(int k, int i, int window);    
   double measureAngle(int contour, int i);
   double measureRelativeAngle(int contour, int i, int j);
   void measureAllCurvatures();
   double* getRoundedCorners(int window);
   int wrap(int i, int lo, int hi);
   void meltContours(int radius);
   void meltContour(int radius, int k);
   int ixy(int x, int y);
   int getNPoints();
   int getMaxContour();
   void deleteContour(int k);
   void resize(int width, int height);
   void cleanup(void);
}
#endif
