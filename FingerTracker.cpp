//
//  FingerTracker.cpp
//  OpenNIUserTest
//
//  Created by research on 11/4/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include "FingerTracker.h"

namespace FingerTracker {
   
#define MAXFINGERS 100
#define TAU 6.28318531
   
   finger *fingers;
   finger *fingers2;
   finger *oldfingers;
   
   int w;
   int h;
   int numfingers;
   int oldnumfingers;
   
   double *tips;
   double FINGER_RADIUS = 15.0;				// perimeter of a fingertip
   double ROUNDNESS_THRESHOLD = 1.0;			// minimum allowable value for
   double MIN_CONTOUR_LENGTH = 250;
   double* img;
   bool  trackingHands = false;
   XnUserID aUserIDs[20];
	 XnUInt16 nUsers = 20;
   
   int SCREEN_SIZE;
   int WIDTH;
   int HEIGHT;
   
   hand lefthand;
   hand righthand;
   
   xn::UserGenerator userGenerator;
   xn::DepthGenerator depthGenerator;   
   
   void init(int width, int height, int screen_size) {
      WIDTH = width;
      HEIGHT = height;
      SCREEN_SIZE = screen_size;
      lefthand.skeletonID = XN_SKEL_LEFT_HAND;
      righthand.skeletonID = XN_SKEL_RIGHT_HAND;
      
      if (fingers == NULL) {
         fingers = (finger*)malloc(sizeof(finger[MAXFINGERS]));
         fingers2 = (finger*)malloc(sizeof(finger[MAXFINGERS]));
         oldfingers = (finger*)malloc(sizeof(finger[MAXFINGERS]));
      }
      
   }
   
   void setUserGenerator(xn::UserGenerator ug) {
      userGenerator = ug;
   }
   
   void setDepthGenerator(xn::DepthGenerator dg) {
      depthGenerator = dg;
   }
   
   void prepSignal(const XnDepthPixel* depth, float radius) {
      
      trackingHands = false;
      if (img == NULL) {
         img = (double*)malloc(sizeof(double[SCREEN_SIZE]));
         fiso::init(WIDTH, HEIGHT);
      }
      fiso::setThreshold(0.5);
      
      nUsers = 1;
      userGenerator.GetUsers(aUserIDs, nUsers);
      
      if (nUsers >= 1 && userGenerator.GetSkeletonCap().IsTracking(aUserIDs[0])) {
         
         trackingHands = true;
         
         XnUserID myUserID = aUserIDs[0];
         
         userGenerator.GetSkeletonCap().GetSkeletonJointPosition(myUserID, XN_SKEL_RIGHT_HAND, righthand.world);
         userGenerator.GetSkeletonCap().GetSkeletonJointPosition(myUserID, XN_SKEL_LEFT_HAND, lefthand.world);
         
         depthGenerator.ConvertRealWorldToProjective(1, &righthand.world.position, &righthand.screen.position);
         depthGenerator.ConvertRealWorldToProjective(1, &lefthand.world.position, &lefthand.screen.position);
         
         for (int i = 0; i < SCREEN_SIZE; i++) {
            float z = depth[i];
            float x = i % WIDTH;
            float y = i / WIDTH;
            float ldx = (x - lefthand.screen.position.X);
            float ldy = (y - lefthand.screen.position.Y);
            float ldz = (z - lefthand.screen.position.Z);
            float ldd = ldx * ldx + ldy * ldy;
            float rdx = (x - righthand.screen.position.X);
            float rdy = (y - righthand.screen.position.Y);
            float rdz = (z - righthand.screen.position.Z);
            float rdd = rdx * rdx + rdy * rdy;
            float z2 = z * z / 500000;
            
            bool nearLeft = (ldz * ldz < radius && ldd < radius / z2);
            bool nearRight = (rdz * rdz < radius && rdd < radius / z2);
            if (nearLeft || nearRight) {
               img[i] = z;
            } else {
               img[i] = 0.0;
            }
            
            
         }
         
      } else {
         for (int i = 0; i < SCREEN_SIZE; i++) {
            float z = depth[i];
            if (z > 0 && z < 1500) {
               img[i] = z;
            } else {
               img[i] = 0.0;
            }
         }
      }
      
   }
   
   void update(const XnDepthPixel* depth) {
      
      if (!trackingHands) {
         prepSignal(depth, 100 * 100);
         numfingers = findFingers(img, fingers);
      } else {
         
         
         //prepSignal(depth, 150 * 150);
         //int numfingers2 = findFingers(img, fingers2);
         
         prepSignal(depth, 120 * 120);
         int numfingers1 = findFingers(img, fingers);
         
         /*
         for (int i = 0; i < numfingers1; i++) {
            float mindist = 1000;
            for (int j = 0; j < numfingers2; j++) {
               float dist = dist2(fingers[i].screen.position, fingers2[j].screen.position);
               if (dist < mindist) {
                  mindist = dist;
               }
            }
            if (mindist > 25) {
               fingers[i].keep = false;
            }
         }
         
         int goodfingers = 0;
         for (int i = 0; i < numfingers1; i++) {
            if (fingers[i].keep) {
               fingers[goodfingers] = fingers[i];
               goodfingers++;
            }
         }
         numfingers = goodfingers;
         */
         numfingers = numfingers1;
         
         
         lefthand.numfound = 0;
         righthand.numfound = 0;
         
         for (int i = 0; i < numfingers; i++) {
            float ld = dist2(lefthand.world.position, fingers[i].world.position);
            float rd = dist2(righthand.world.position, fingers[i].world.position);
            if (ld <= rd) {
               fingers[i].handID = XN_SKEL_LEFT_HAND;
               fingers[i].skeletonID = XN_SKEL_LEFT_FINGER_INDEX;
               lefthand.fingers[lefthand.numfound] = &fingers[i];
               lefthand.numfound++;
            } else {
               fingers[i].handID = XN_SKEL_RIGHT_HAND;
               fingers[i].skeletonID = XN_SKEL_RIGHT_FINGER_INDEX;
               righthand.fingers[righthand.numfound] = &fingers[i];
               righthand.numfound++;
            }
         }
         
         identifyFingers(lefthand);
         identifyFingers(righthand);
         
         saveFingers();
      }
      
   }
   
   
   void identifyFingers(hand h) {
      
      int oldnumfound = 0;
      for (int i = 0; i < oldnumfingers; i++) {
         if (oldfingers[i].handID == h.skeletonID) {
            oldnumfound++;
         }
      }
      
      // new finger added, was it spurious?
      if (oldnumfound < h.numfound) {
         //printf("Added finger\n");
         float maxdist = 0;
         int outliernew = -1;
         for (int i = 0; i < h.numfound; i++) {
            for (int j = 0; j < oldnumfingers; j++) {
               if (oldfingers[j].handID == h.skeletonID) {
                  float dist = dist2(h.fingers[i]->screen.position, oldfingers[j].screen.position);
                  if (dist > maxdist) {
                     maxdist = dist;
                     outliernew = i;
                  }
               }
            }
         }
         if (outliernew > -1) {
            h.fingers[outliernew]->isoutlier = true;
            h.fingers[outliernew]->skeletonID = XN_SKEL_FINGER;
            //printf("Dropping finger, hand now has %i fingers\n", h.numfound);
            int goodfingers = 0;
            for (int i = 0; i < h.numfound; i++) {
               if (!h.fingers[i]->isoutlier) {
                  h.fingers[goodfingers] = h.fingers[i];
                  goodfingers++;
               }
            }
            h.numfound = goodfingers;
         }
      }
      
      
      
      if (h.numfound == 3) {
         int minc = 1000;  int minidx = -1;
         for (int i = 0; i < 3; i++) {
            if (h.fingers[i]->curvature < minc) {
               minc = h.fingers[i]->curvature;
               minidx = i;
            }
         }
         if (minidx >= 0 && minidx < 3) {
            finger* tmp = h.fingers[2];
            h.fingers[2] = h.fingers[minidx];
            h.fingers[minidx] = tmp;
            h.fingers[2]->isoutlier = true;
            h.numfound = 2;
            h.fingers[2]->screen.position.X = 100;
            h.fingers[2]->screen.position.Y = 100;
            printf("Picking highest two curvatures\n");
         }
      }
      
      
      double angle;
      if (h.numfound == 2) {
         angle = measureSignedAngleChange(h.fingers[0]->screen.position, h.fingers[1]->screen.position, h.screen.position);
         if (h.skeletonID == XN_SKEL_LEFT_HAND) {
            if (angle >= 0) {
               h.fingers[0]->skeletonID = XN_SKEL_LEFT_FINGER_THUMB;
               h.fingers[1]->skeletonID = XN_SKEL_LEFT_FINGER_INDEX;
            } else {
               h.fingers[0]->skeletonID = XN_SKEL_LEFT_FINGER_INDEX;
               h.fingers[1]->skeletonID = XN_SKEL_LEFT_FINGER_THUMB;
            }
         } else {
            if (angle < 0) {
               h.fingers[0]->skeletonID = XN_SKEL_RIGHT_FINGER_THUMB;
               h.fingers[1]->skeletonID = XN_SKEL_RIGHT_FINGER_INDEX;
            } else {
               h.fingers[0]->skeletonID = XN_SKEL_RIGHT_FINGER_INDEX;
               h.fingers[1]->skeletonID = XN_SKEL_RIGHT_FINGER_THUMB;
            }            
         }
      }
      if (h.numfound > 2 && h.numfound <= 5) {
         if (h.skeletonID == XN_SKEL_LEFT_HAND) {            
            for (int i = 0; i < h.numfound; i++) {
               if (i == 0) h.fingers[i]->skeletonID = XN_SKEL_LEFT_FINGER_THUMB;
               if (i == 1) h.fingers[i]->skeletonID = XN_SKEL_LEFT_FINGER_INDEX;
               if (i == 2) h.fingers[i]->skeletonID = XN_SKEL_LEFT_FINGER_MIDDLE;
               if (i == 3) h.fingers[i]->skeletonID = XN_SKEL_LEFT_FINGER_RING;
               if (i == 4) h.fingers[i]->skeletonID = XN_SKEL_LEFT_FINGER_PINKY;
            }
         } else {
            for (int i = 0; i < h.numfound; i++) {
               if (i == 0) h.fingers[i]->skeletonID = XN_SKEL_RIGHT_FINGER_THUMB;
               if (i == 1) h.fingers[i]->skeletonID = XN_SKEL_RIGHT_FINGER_INDEX;
               if (i == 2) h.fingers[i]->skeletonID = XN_SKEL_RIGHT_FINGER_MIDDLE;
               if (i == 3) h.fingers[i]->skeletonID = XN_SKEL_RIGHT_FINGER_RING;
               if (i == 4) h.fingers[i]->skeletonID = XN_SKEL_RIGHT_FINGER_PINKY;
            }
         }
      }
      
      //printf("identifyFingers: numfound is %i\n", h.numfound);
   }
   
   
   int findFingers(double* depth, finger* fingers) {
      
      fiso::find(img);
      int numcontours = fiso::getNumContours();
      
      for (int k = 0; k < numcontours; k++) {
         int length = fiso::getContourLength(k);
         if (length >= MIN_CONTOUR_LENGTH) {
            int reps = length / 25;
            for (int i = 0; i < reps; i++) {
               fiso::meltContour(5,k);
            }
         } else {
            //printf("Contour too small (%i segments), dropping...\n", length);
         }
      }      
      
      for (int i = 0; i < MAXFINGERS; i++) {
         fingers[i].screen.fConfidence = 0.0;
         fingers[i].world.fConfidence = 0.0;
         fingers[i].keep = false;
         fingers[i].curvature = 0.0;
         fingers[i].handID = 0;
         fingers[i].skeletonID = -1;
         fingers[i].contour = -1;
         fingers[i].idx = -1;
      }
      
      int numfingers = 0;
      for (int k = 0; k < numcontours; k++) {
         int spanoffset = 0;
         int l = fiso::getContourLength(k);
         int window = (int)FINGER_RADIUS;
         
         if (l < MIN_CONTOUR_LENGTH) 
            continue;
         
         //printf("Contour %i is %i segments long with FINGER_RADIUS = %2.f\n", k, l, FINGER_RADIUS);
         for (int i = 0; i < l; i++) {
            if (!isTipLike(k, i)) {
               spanoffset = i + 1;
               break;
            }
         }
         int span = 0;
         for (int i = spanoffset; i < l + spanoffset; i++) {
            if (isTipLike(k, i)) {
               span++;
            } else {
               if (span > 0) {
                  int tip = (i - 1) - span/2;
                  int lo = tip - window;
                  int hi = tip + window;
                  double cx = 0;
                  double cy = 0;
                  for (int j = lo; j <= hi; j++) {
                     cx += fiso::getContourX(k,j);
                     cy += fiso::getContourY(k,j);
                  }
                  cx = (int)(cx / (2 * window + 1));
                  cy = (int)(cy / (2 * window + 1));
                  fingers[numfingers].screen.position.X = cx;
                  fingers[numfingers].screen.position.Y = cy;
                  fingers[numfingers].screen.position.Z = sampleDepth(cx, cy, img);
                  
                  double nx = fiso::measureSegmentNormalX(k, tip - 2 * window, tip + 2 * window);
                  double ny = fiso::measureSegmentNormalY(k,  tip - 2 * window, tip + 2 * window);
                  double nl = sqrt(nx * nx + ny * ny);
                  if (nl == 0) {
                     fingers[numfingers].nx = 0;
                     fingers[numfingers].ny = 0;
                  } else {
                     fingers[numfingers].nx = (float)(nx / nl);
                     fingers[numfingers].ny = (float)(ny / nl);
                  }
                  fingers[numfingers].idx = tip;
                  fingers[numfingers].contour = k;
                  fingers[numfingers].screen.fConfidence = 0.5;
                  fingers[numfingers].world.fConfidence = 0.5;
                  fingers[numfingers].keep = true;
                  fingers[numfingers].isoutlier = false;
                  isTipLike(k, tip, fingers[numfingers].curvature);
                  
                  depthGenerator.ConvertProjectiveToRealWorld(1, &(fingers[numfingers].screen.position), &(fingers[numfingers].world.position));
                  //printf("Finger %i: x:%3.1f\ty:%3.1f\tz:%3.1f\n",i,fingers[numfingers].world.position.X,fingers[numfingers].world.position.Y,fingers[numfingers].world.position.Z);
                  numfingers++;
               }
               span = 0;
            }
         }
      }
      return numfingers;
   }
   
   bool isTipLike(int k, int i) {
      double r = 0;
      return isTipLike(k, i, r);
   }
   
   bool isTipLike(int k, int i, double &store) {
      int r = 15;
      store = fiso::measureCurvature(k, i - r, i + r);      
      return store > 3;
   }
   
   bool isPeninsula(finger f) {
      int radius = 25;
      int score = 0;
      int inc = 300;
      int ox = f.screen.position.X;
      int oy = f.screen.position.Y;
      for (int i = 0; i < inc; i++) {
         double angle = (i / inc) * TAU;
         int x = (int)(ox + radius * cos(angle));
         int y = (int)(oy + radius * sin(angle));
         int idx = x + y * 640;
         if (idx >= 0 && idx < SCREEN_SIZE) {
            double sample = img[idx];
            bool test = sample == 0;
            if (test) score++;
         }
      }
      return ((score / inc) > 0.5);
   }
   
   double sampleDepth(double cx, double cy, double* img) {
      int x = cx;
      int y = cy;
      int idx = WIDTH * y + x;
      double val = img[idx];
      if (val != 0) return val;
      
      double hits = 0;
      double acc = 0;
      for (int dx = -15; dx <= 15; dx++) {
         for (int dy = -15; dy <= 15; dy++) {
            int idx = WIDTH * (y + dy) + (x + dx);
            val = img[idx];
            if (val != 0) {
               acc += val;
               hits++;
            }
         }
      }
      if (hits != 0) return acc / hits;
      
      printf("sampleDepth: got background (0) value\n");
      return 0;
   }
   
   int getNumFingers(void) {
      return numfingers;
   }
   
   finger getFinger(int i) {
      return fingers[i];
   }
   
   finger* getFingers() {
      return fingers;
   }
   
   
   double dist2(XnPoint3D a, XnPoint3D b) {
      return ((a.X - b.X) * (a.X - b.X) + (a.Y - b.Y) * (a.Y - b.Y) + (a.Z - b.Z) * (a.Z - b.Z));
   }
   
   double calcangle(float ox, float oy, float px, float py) {
      float y = py - oy;
      float x = px - ox;
      float rads = atan2f(y, x);
      
      if (rads < 0) rads = rads + TAU;
      return rads;
   }
   
   
   // measure 2D angle change clockwise around a circle
   // From point A to point B with center c
   double measureSignedAngleChange(XnPoint3D a, XnPoint3D b, XnPoint3D c) {
      
      double fromx = 0, fromy = 0, tox = 0, toy = 0, tol = 0, froml = 0, dot;
      double rads;
      fromx = a.X - c.X;
      fromy = a.Y - c.Y;
      tox = b.X - c.X;
      toy = b.Y - c.Y;
      
      froml = sqrt(fromx * fromx + fromy * fromy); 
      fromx = fromx / froml;
      fromy = fromy / froml;
      tol = sqrt(tox * tox + toy * toy);
      tox = tox / tol;
      toy = toy / tol;		
      
      dot = fromx * tox + fromy * toy;
      if (dot > 1.0) dot = 1.0;
      if (dot < 0) dot = 0;
      rads = acos(dot);
      if (isnan(rads)) {
         printf("oops: measureRelativeAngle has nan");
      }
      if (tox * fromy - toy * fromx < 0) rads = rads * -1;
      return rads;
   }
   
   hand getlefthand(void) {
      return lefthand;
   }
   
   hand getrighthand(void) {
      return righthand;
   }
   
   void printFinger(int i) {
      finger f = getFinger(i);
      printf("Finger %i: curvature is %3.5f\n", i, f.curvature);
   }
   
   void printFingers() {
      for (int i = 0; i < numfingers; i++) {
         printFinger(i);
      }
      
      /*
       printf("****** OLD FINGERS *******\n");
       for (int i = 0; i < oldnumfingers; i++) {
       if (oldfingers[i].handID == XN_SKEL_LEFT_HAND) {
       printf("Old Finger %i is on left hand\n",i);       
       } else if (oldfingers[i].handID == XN_SKEL_RIGHT_HAND) {
       printf("Old Finger %i is on right hand\n",i);                   
       } else {
       printf("Old Finger %i is not on a hand\n",i);                
       }
       }
       printf("**************************\n");
       */
   }
   
   void saveFingers(void) {
      for (int i = 0; i < numfingers; i++) {
         oldfingers[i] = fingers[i];
      }
      //memcpy(oldfingers, fingers, sizeof(fingers[MAXFINGERS]));
      oldnumfingers = numfingers;
   }
   
   void cleanup(void) {
      free(fingers);
      free(fingers2);
      free(oldfingers);
   }
   
}