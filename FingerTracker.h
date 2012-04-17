////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//  FingerTracker.h
//  Contour-based fingertracking
//
//  Uses depth image and skeleton from OpenNI
//  Finds contours using marching squares
//  Assumes fingers are points of "high" curvature on the contour
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

#ifndef OpenNIUserTest_FingerTracker_h
#define OpenNIUserTest_FingerTracker_h

#include <iostream>
#include <math.h>
#include <XnCppWrapper.h>
#include "FindIsoline.h"

namespace FingerTracker {
   
   const int XN_SKEL_FINGER               = 25;
   
   const int XN_SKEL_LEFT_FINGER          = 26;
   const int XN_SKEL_LEFT_FINGER_THUMB    = 27;
   const int XN_SKEL_LEFT_FINGER_INDEX    = 28;
   const int XN_SKEL_LEFT_FINGER_MIDDLE   = 29; 
   const int XN_SKEL_LEFT_FINGER_RING     = 30; 
   const int XN_SKEL_LEFT_FINGER_PINKY    = 31; 
   
   const int XN_SKEL_RIGHT_FINGER         = 32;
   const int XN_SKEL_RIGHT_FINGER_THUMB   = 33;
   const int XN_SKEL_RIGHT_FINGER_INDEX   = 34;
   const int XN_SKEL_RIGHT_FINGER_MIDDLE  = 35; 
   const int XN_SKEL_RIGHT_FINGER_RING    = 36; 
   const int XN_SKEL_RIGHT_FINGER_PINKY   = 37;
   
   typedef struct finger_struct {
      XnSkeletonJointPosition screen;
      XnSkeletonJointPosition world;
      float nx;
      float ny;
      int idx;
      int contour;
      int skeletonID;
      int handID;
      bool keep;
      double curvature;
      bool isoutlier;
   } finger;
   
   typedef struct hand_struct {
      XnSkeletonJointPosition world;
      XnSkeletonJointPosition screen;
      finger *fingers[10];
      int numfound;
      int frameid;
      int skeletonID;
   } hand;
      
   void init(int, int, int);
   void update(const XnDepthPixel*);
   int findFingers(double*, finger*);
   void prepSignal(const XnDepthPixel*, float);
   int getNumFingers(void);
   finger getFinger(int i);
   finger* getFingers();
   void setUserGenerator(xn::UserGenerator ug);
   void setDepthGenerator(xn::DepthGenerator dg);
   double dist2(XnPoint3D a, XnPoint3D b);
   double calcangle(float ox, float oy, float px, float py);
   double measureSignedAngleChange(XnPoint3D a, XnPoint3D b, XnPoint3D c);
   void identifyFingers(hand h);
   hand getlefthand(void);
   hand getrighthand(void);
   bool isPeninsula(finger f);
   bool isTipLike(int k, int i);
   bool isTipLike(int k, int i, double &store);
   double sampleDepth(double cx, double cy, double* img);
   void printFinger(int i);
   void printFingers(void);
   void saveFingers(void);
   void cleanup(void);

}

#endif
