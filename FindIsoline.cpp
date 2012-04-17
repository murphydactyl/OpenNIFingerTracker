//
//  FindContours.cpp
//  OpenNIFingerTracker
//
//  Created by Murphy Stein
//  New York University
//  MIT License

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include "FindIsoline.h"

namespace fiso {
   
#define TAU         3.14159 * 2
#define ixy(x, y)   (x) + (y) * w
   const unsigned int CASE0    =  0x00000000; 
   const unsigned int CASE1    =  0x00000001;
   const unsigned int CASE2    =  0x00000010;
   const unsigned int CASE3    =  0x00000011;
   const unsigned int CASE4    =  0x00000100;
   const unsigned int CASE5    =  0x00000101;
   const unsigned int CASE6    =  0x00000110;
   const unsigned int CASE7    =  0x00000111;
   const unsigned int CASE8    =  0x00001000;
   const unsigned int CASE9    =  0x00001001;
   const unsigned int CASE10   =  0x00001010;
   const unsigned int CASE11   =  0x00001011;
   const unsigned int CASE12   =  0x00001100;
   const unsigned int CASE13   =  0x00001101;
   const unsigned int CASE14   =  0x00001110;
   const unsigned int CASE15   =  0x00001111;
   
   static unsigned int w = 0;
   static unsigned int h = 0;
   
   static double *vx;		// tmp
   static double *vy;		// tmp
   static double *minx;
   static double *miny;
   static double *maxx;
   static double *maxy;
   
   static int      *cd;	
   static double   *cx;	
   static double   *cy;
   static double   *cu;
   static int      *cl;
   static int      *co;
   static double   *tips;
   
   static int n = 0;
   static double threshold = 128;
   static int numContours = 0;
   static int pixels;
   
   static int bZeroBorder = true;
   
   void init(int width, int height) {
      resize(width, height);
   }
   
   void resize(int width, int height) {
      if (w != width || h != height) {
         if (cd != NULL) { free(cd); }
         if (cl != NULL) { free(cl); }
         if (co != NULL) { free(co); }
         if (cx != NULL) { free(cx); }
         if (cy != NULL) { free(cy); }
         if (cu != NULL) { free(cu); }
         if (vx != NULL) { free(vx); }
         if (vy != NULL) { free(vy); }
         if (tips != NULL) { free(tips); }
         if (minx != NULL) { free(minx); }
         if (maxx != NULL) { free(maxx); }
         if (miny != NULL) { free(miny); }
         if (maxy != NULL) { free(maxy); }
         
         w = width;
         h = height;
         pixels = w * h;
         
         cd = (int*)malloc(sizeof(int[pixels]));	
         cl = (int*)malloc(sizeof(int[pixels]));	
         co = (int*)malloc(sizeof(int[pixels]));	
         cx = (double*)malloc(sizeof(double[pixels]));	
         cy = (double*)malloc(sizeof(double[pixels]));	
         cu = (double*)malloc(sizeof(double[pixels]));	
         vx = (double*)malloc(sizeof(double[pixels]));	
         vy = (double*)malloc(sizeof(double[pixels]));	
         
         tips = (double*)malloc(sizeof(double[pixels]));	
         minx = (double*)malloc(sizeof(double[pixels]));	
         maxx = (double*)malloc(sizeof(double[pixels]));	
         miny = (double*)malloc(sizeof(double[pixels]));	
         maxy = (double*)malloc(sizeof(double[pixels]));	
      }
   }
   
   void setThreshold(double t) {
      threshold = t;
   }
   
   double sample(double x, double y, double *pix) {
      
      // compute query rect
      // for each of 4 possible intersecting rects
      //		compute area of overlap
      //		multiply weight times pixel value of target times area
      //		add result to accumulator
      // return accumulator
      //
      // overlap rects are numbered 0, 1, 2, 3 going left->right & top->bottom
      
      double top[5];
      double bot[5];
      double rgt[5];
      double lft[5];
      
      top[0] = y;
      bot[0] = y + 1;
      lft[0] = x;
      rgt[0] = x + 1;
      
      top[1] = (int)top[0];
      top[2] = (int)top[0];
      top[3] = (int)top[0] + 1.0;
      top[4] = (int)top[0] + 1.0;
      
      bot[1] = (int)bot[0];
      bot[2] = (int)bot[0];
      bot[3] = (int)bot[0] + 1.0;
      bot[4] = (int)bot[0] + 1.0;
      
      lft[1] = (int)lft[0];
      lft[2] = (int)lft[0] + 1.0;
      lft[3] = (int)lft[0];
      lft[4] = (int)lft[0] + 1.0;
      
      rgt[1] = (int)rgt[0];
      rgt[2] = (int)rgt[0] + 1.0;
      rgt[3] = (int)rgt[0];
      rgt[4] = (int)rgt[0] + 1.0;
      
      double acc = 0;					// accumulator
      for (int i = 1; i < 5; i++) {		
         double t = top[0] > top[i] ? top[0] : top[i];
         double b = bot[0] > bot[i] ? bot[i] : bot[0];
         double l = lft[0] > lft[i] ? lft[0] : lft[i];
         double r = rgt[0] > rgt[i] ? rgt[i] : rgt[0];
         
         if ( t < b  && l < r ) {
            double area = (b - t) * (r - l);
            int idx = ixy(lft[i], top[i]);
            acc += area * pix[idx];
         }
      }
      return acc;
   }
   
   
   inline double t(int A, int B) {
      if (A-B == 0) return 0;
      return ((double)(A - threshold)) / ((double)(A - B));
   }
   
   int find(double *in) {
      
      if (bZeroBorder) {
         for (int i = 0; i < w; i++) {
            in[i] = 0;
            in[pixels - i - 1] = 0;
         }
         for (int i = w; i < w * h; i += w) {
            in[i] = 0;
            in[i - 1] = 0;
         }
      }
      
      int vi = 0;
      int b0, b1, b2, b3;
      /*
       for (int x = 1; x < w - 1; x++) {
       for (int y = 1; y < h - 1; y++) {
       
       b0 = in[ixy(x, y)] < threshold ? 0x00001000 : 0x00000000;
       b1 = in[ixy(x + 1, y)] < threshold ? 0x00000100 : 0x00000000;
       b2 = in[ixy(x + 1, y + 1)] < threshold ? 0x00000010 : 0x00000000;
       b3 = in[ixy(x, y + 1)] < threshold ? 0x00000001 : 0x00000000;
       cd[ixy(x, y)] = b0 | b1 | b2 | b3;
       }
       }
       */
      for (int i = w; i < pixels; i++) {
         b0 = in[i] < threshold ? 0x00001000 : 0x00000000;
         b1 = in[i + 1] < threshold ? 0x00000100 : 0x00000000;
         b2 = in[i + w + 1] < threshold ? 0x00000010 : 0x00000000;
         b3 = in[i + w] < threshold ? 0x00000001 : 0x00000000;
         cd[i] = b0 | b1 | b2 | b3;
      }
      
      int next = -1;
      int i = 0, x = 0, y = 0;
      int fromedge = -1;
      int toedge = -1;
      int code = 0;
      double avg = 0;
      int contournum = -1;
      int length = 0;
      while (i < pixels) {
         
         fromedge = toedge;			
         if (next < 0) next = ++i;
         x = next % w;
         y = next / w;
         
         code = cd[next];
         
         switch (code) {
            case CASE0:									// CASE 0
               toedge = -1;
               break;
            case CASE1:									// CASE 1
               cd[next] = 0;
               //out ("case 1");
               toedge = 2;
               break;
            case CASE2:									// CASE 2
               cd[next] = 0;
               //out ("case 2");
               toedge = 1;
               break;
            case CASE3:									// CASE 3
               cd[next] = 0;
               toedge = 1;
               //out ("case 3");
               
               break;
            case CASE4:									// CASE 4
               cd[next] = 0;
               toedge = 0;
               //out ("case 4");
               
               break;
            case CASE5:									// CASE 5, saddle
               avg = 0.25 * (double)(in[ixy(x, y)] + in[ixy(x + 1, y)] + in[ixy(x, y + 1)] + in[ixy(x + 1, y + 1)]);
               if (avg > threshold) {
                  if (fromedge == 3) {						// treat as case 1, then switch code to case 4
                     toedge = 2;								
                     cd[next] = CASE4;
                  } else {									// treat as case 4, then switch code to case 1
                     toedge = 0;								
                     cd[next] = CASE1;
                  }
               } else {
                  if (fromedge == 3) {						// treat as case 7, then switch code to case 13
                     toedge = 0;								
                     cd[next] = CASE13;
                  } else {									// treat as case 13, then switch code to case 7
                     toedge = 2;								
                     cd[next] = CASE7;
                  }
               }
               //out ("case 5");
               
               break;
            case CASE6:									// CASE 6
               cd[next] = 0;
               toedge = 0;
               //out ("case 6");
               
               break;
            case CASE7:									// CASE 7
               cd[next] = 0;
               toedge = 0;
               //out ("case 7");
               
               break;
            case CASE8:									// CASE 8
               cd[next] = 0;
               toedge = 3;
               //out ("case 8");
               
               break;
            case CASE9:									// CASE 9
               cd[next] = 0;
               toedge = 2;
               //out ("case 9");
               
               break;
            case CASE10:									// CASE 10, saddle
               avg = 0.25 * (double)(in[ixy(x, y)] + in[ixy(x + 1, y)] + in[ixy(x, y + 1)] + in[ixy(x + 1, y + 1)]);
               if (avg > threshold) {
                  if (fromedge == 0) {						// treat as case 8, then switch code to case 2
                     toedge = 3;								
                     cd[next] = CASE2;
                  } else {									// treat as case 2, then switch code to case 8
                     toedge = 1;								
                     cd[next] = CASE8;
                  }
               } else {
                  if (fromedge == 2) {						// treat as case 14, then switch code to case 11
                     toedge = 3;								
                     cd[next] = CASE11;
                  } else {									// treat as case 11, then switch code to case 14
                     toedge = 1;								
                     cd[next] = CASE14;
                  }
               }
               //out ("case 10");
               
               break;
            case CASE11:									// CASE 11
               cd[next] = 0;
               toedge = 1;
               //out ("case 11");
               
               break;
            case CASE12:									// CASE 12
               cd[next] = 0;
               toedge = 3;
               //out ("case 12");
               
               break;
            case CASE13:									// CASE 13
               cd[next] = 0;
               toedge = 2;
               //out ("case 13");
               
               break;
            case CASE14:									// CASE 14
               cd[next] = 0;
               toedge = 3;
               //out ("case 14");
               
               break;
            case CASE15:									// CASE 15
               toedge = -1;
               break;
            default:
               //out("Uh oh, unknown case");
               break;
         }
         
         if (fromedge == -1 && toedge > -1) {					// starting a new contour
            contournum++;
         }
         
         switch (toedge) {			
            case 0: 
               cx[vi] = x + t(in[ixy(x, y)], in[ixy(x + 1, y)]);
               cy[vi++] = (double)y;
               next = ixy(x, y - 1);
               cl[contournum] = ++length;			
               break;
            case 1:
               cx[vi] = (double)x + 1;
               cy[vi++] = y + t(in[ixy(x + 1, y)], in[ixy(x + 1, y + 1)]);
               next = ixy(x + 1, y);
               cl[contournum] = ++length;			
               break;
            case 2:
               cx[vi] = x + t(in[ixy(x, y + 1)], in[ixy(x + 1, y + 1)]);
               cy[vi++] = (double)(y + 1);
               next = ixy(x, y + 1);
               cl[contournum] = ++length;			
               break;
            case 3:
               cx[vi] = (double)x;
               cy[vi++] = y + t(in[ixy(x, y)], in[ixy(x, y + 1)]);
               next = ixy(x - 1, y);
               cl[contournum] = ++length;			
               break;
            default:
               next = -1;
               length = 0;
               break;
         }
         
      }
      
      numContours = contournum + 1;
      int sum = 0;
      for (i = 0; i < numContours; i++) {
         co[i] = sum;
         sum += cl[i];
      }
      
      //computeBoundingBoxes();
      
      return numContours;
   }
   
   
   int* getContourLengths(void) {
      return cl;
   }
   
   int getContourLength(int contour) {
      return cl[contour];
   }
   
   int getNumContours(void) {
      return numContours;
   }
   
   int getLastIndex(void) {
      int nc = getNumContours();
      if (nc > 0) {
         return co[nc - 1] + cl[nc - 1];
      }
      return 0;
   }
   
   void setContourX(int contour, int v, double x) {	
      int o = co[contour];
      cx[wrap(o + v, o, o + cl[contour])] = x;
   }
   
   void setContourY(int contour, int v, double y) {	
      int o = co[contour];
      cy[wrap(o + v, o, o + cl[contour])] = y;
   }
   
   
   double getContourX(int contour, int v) {	
      int o = co[contour];
      return cx[wrap(o + v, o, o + cl[contour])];
   }
   
   double getContourY(int contour, int v) {
      int o = co[contour];
      return cy[wrap(o + v, o, o + cl[contour])];
   }
   
   double getContourAngle(int contour, int v) {
      int o = co[contour];
      return cu[wrap(o + v, o, o + cl[contour])];
   }
   
   int getValidIndex(int contour, int v) {
      int o = co[contour];
      return wrap(o + v, o, o + cl[contour]);
   }
   
   double getBBMinX(int contour) {	return minx[contour];	}
   double getBBMaxX(int contour) {	return maxx[contour];	}
   double getBBMinY(int contour) {	return miny[contour];	}
   double getBBMaxY(int contour) {	return maxy[contour];	}
   
   
   double measureSegmentArea(int contour, int first, int last) {
      double area = 0;
      if (getValidIndex(contour, first) == getValidIndex(contour, last)) last = last - 1;
      double w = 0, h = 0;
      for (int i = first; i < last ; i++) {
         w = getContourX(contour, i + 1) - getContourX(contour, i);
         h = (getContourY(contour, i + 1) + getContourY(contour, i)) / 2.0;
         area += w * h;
      }
      w = getContourX(contour, first) - getContourX(contour, last);
      h = (getContourY(contour, first) + getContourY(contour, last)) / 2.0;
      area += w * h;
      return area;
   }
   
   double measureContourArea(int contour) {
      return measureSegmentArea(contour, 0, getContourLength(contour));
   }
   
   double measureMeanX(int contour) {
      double mean = 0.0;
      int l = getContourLength(contour);
      for (int i = 0; i < l; i++)
         mean += getContourX(contour, i);
      return mean / l;
   }
   
   double measureMeanY(int contour) {
      double mean = 0.0;
      int l = getContourLength(contour);
      for (int i = 0; i < l; i++)
         mean += getContourY(contour, i);
      return mean / l;
   }
   
   
   double measureSegmentLength(int contour, int first, int last) {
      if (getValidIndex(contour, first) == getValidIndex(contour, last)) last = last - 1;
      double perim = 0;
      for (int i = first; i < last ; i++) {
         perim += measureLength(contour, i);
      }
      return perim;
   }
   
   double measureSegmentPerimeter(int contour, int first, int last) {
      if (getValidIndex(contour, first) == getValidIndex(contour, last)) last = last - 1;
      double perim = 0;
      for (int i = first; i < last ; i++) {
         perim += measureLength(contour, i);
      }
      double dx = getContourX(contour, first) - getContourX(contour, last);
      double dy = getContourY(contour, first) - getContourY(contour, last);
      perim += sqrt(dx * dx + dy * dy);
      return perim;
   }
   
   double measureContourPerimeter(int contour) {
      return measureSegmentPerimeter(contour, 0, getContourLength(contour));
   }
   
   
   double measureNormalX(int contour, int i) {
      double ret = getContourY(contour, i) - getContourY(contour, i + 1);
      ret = ret / measureLength(contour, i);
      return ret;
   }
   
   double measureNormalY(int contour, int i) {
      double ret = getContourX(contour, i + 1) - getContourX(contour, i);
      ret = ret / measureLength(contour, i);
      return ret;
   }
   
   double measureSegmentNormalY(int contour, int first, int last) {
      double ret = 0;
      for (int i = first; i < last ; i++) {
         ret += measureNormalY(contour, i);
      }
      return ret;
   }
   
   double measureSegmentNormalX(int contour, int first, int last) {
      double ret = 0;
      for (int i = first; i < last ; i++) {
         ret += measureNormalX(contour, i);
      }
      return ret;
   }

   double measureCurvature(int contour, int first, int last) {
      double sum = 0;
      for (int i = first; i <= last; i++) {
         sum += measureAngle(contour, i) / measureLength(contour, i);
      }
      return sum;
   }

   double measureSegmentAngleChange(int contour, int first, int last) {
      double sum = 0;
      for (int i = first; i <= last; i++) {
         sum += measureAngle(contour, i) / measureLength(contour, i);
      }
      return sum;
   }
   
   double measureSignedSegementAngleChangedSquared(int contour, int first, int last) {
      double sum = 0;
      for (int i = first; i <= last; i++) {
         double a2 = measureAngle(contour, i);
         if (a2 < 0) 
            a2 = -a2*a2;
         else
            a2 = a2 * a2;
         sum += a2;
      }
      return sum;      
   }
   
   inline int wrap(int i, int lo, int hi) {
      int l = hi - lo;
      int d = i - lo;
      int w = 0;
      if (d < 0) w = hi - ((-d) % l);
      else w = lo + d % l;
      if (w == hi) w = lo;
      return w;		
   }
   
   double measureEndpointDistance(int contour, int first, int second) {
      double dx = getContourX(contour, first) - getContourX(contour, second);
      double dy = getContourY(contour, first) - getContourY(contour, second);
      return sqrt(dx * dx + dy * dy);
   }
   
   double measureRoundness(int k, int i, int window) {
      int lo = i - window;
      int hi = i + window;
      return measureSegmentArea(k, lo, hi) / measureSegmentPerimeter(k, lo, hi);
   }
   
   double* getRoundedCorners(int window) {
      
		for (int k = 0; k < numContours; k++) {
			int l = getContourLength(k);
			for (int i = 0; i < l; i++) {
				int lo = i - window;
				int hi = i + window;
				tips[getValidIndex(k, i)] = measureSegmentArea(k, lo, hi) / measureSegmentPerimeter(k, lo, hi);
			}
		}
		return tips;
	}
   
   // return the sum of lengths:
   //			i - 1	to		i	
   //			i		to		i + 1
   double measureLength(int contour, int i) {
      
      double aftx = 0, afty = 0, aftl = 0;
      
      int lo = co[contour];
      int n = cl[contour];
      int hi = lo + n;
      
      int v1 = wrap(lo + i+0, lo, hi);
      int v2 = wrap(lo + i+1, lo, hi);
      
      aftx = cx[v2] - cx[v1];
      afty = cy[v2] - cy[v1];
      aftl = sqrt(aftx * aftx + afty * afty);
      
      return aftl;
   }
   
   // return the relative angle change in radians
   // about the point i (assuming ccw is positive)
   double measureAngle(int contour, int i) {
      
      double befx = 0, befy = 0, aftx = 0, afty = 0, aftl = 0, befl = 0, dot;
      double rads;
      
      befx = getContourX(contour, i + 0) - getContourX(contour, i - 1);
      befy = getContourY(contour, i + 0) - getContourY(contour, i - 1);
      aftx = getContourX(contour, i + 1) - getContourX(contour, i + 0);
      afty = getContourY(contour, i + 1) - getContourY(contour, i + 0);
      
      befl = sqrt(befx * befx + befy * befy); 
      befx = befx / befl;
      befy = befy / befl;
      aftl = sqrt(aftx * aftx + afty * afty);
      aftx = aftx / aftl;
      afty = afty / aftl;		
      
      dot = befx * aftx + befy * afty;
      if (dot > 1.0) dot = 1.0;
      if (dot < 0) dot = 0;
      rads = acos(dot);
      if (isnan(rads)) {
         printf("oops: measureAngle has nan");
      }
      if (aftx * befy - afty * befx < 0) rads = rads * -1;
      return rads;
   }
   
   // return the relative angle change in radians
   // about the point i (assuming ccw is positive)
   double measureRelativeAngle(int contour, int i, int j) {
      
      double befx = 0, befy = 0, aftx = 0, afty = 0, aftl = 0, befl = 0, dot;
      double rads;
      
      befx = getContourX(contour, i + 0) - getContourX(contour, i - 1);
      befy = getContourY(contour, i + 0) - getContourY(contour, i - 1);
      aftx = getContourX(contour, j + 0) - getContourX(contour, j - 1);
      afty = getContourY(contour, j + 0) - getContourY(contour, j - 1);
      
      befl = sqrt(befx * befx + befy * befy); 
      befx = befx / befl;
      befy = befy / befl;
      aftl = sqrt(aftx * aftx + afty * afty);
      aftx = aftx / aftl;
      afty = afty / aftl;		
      
      dot = befx * aftx + befy * afty;
      if (dot > 1.0) dot = 1.0;
      if (dot < 0) dot = 0;
      rads = acos(dot);
      if (isnan(rads)) {
         printf("oops: measureRelativeAngle has nan");
      }
      if (aftx * befy - afty * befx < 0) rads = rads * -1;
      return rads;
   }
   
   
   void measureAllCurvatures() {
      
      for (int k = 0; k < numContours; k++) {
         int o = co[k];
         int l = cl[k];
         for (int i = 0; i < l; i++) {
            cu[o + i] = measureAngle(k, i) / measureLength(k, i);
         }
      }
   }
   
   void meltContour(int radius, int k) {
      int o = co[k];
      int l = cl[k];            
      for (int i = 0; i < l; i++) {
         int idx = wrap(o + i, o, o + l);
         double x = 0;
         double y = 0;
         for (int w = -radius; w <= radius; w++) {
            int vw = wrap(o + i + w, o, o + l);
            x += cx[vw];
            y += cy[vw];
         }
         vx[idx] = x / (2 * radius + 1);
         vy[idx] = y / (2 * radius + 1);
      }
      double* cxs = &cx[o];
      double* cys = &cy[o];
      double* vxs = &vx[o];
      double* vys = &vy[o];
      memcpy(cxs, vxs, l * sizeof(double));
      memcpy(cys, vys, l * sizeof(double));
   }
   
   void meltContours(int radius) {
      for (int k = 0; k < numContours; k++) {
         int o = co[k];
         int l = cl[k];            
         for (int i = 0; i < l; i++) {
            int idx = wrap(o + i, o, o + l);
            double x = 0;
            double y = 0;
            for (int w = -radius; w <= radius; w++) {
               int vw = wrap(o + i + w, o, o + l);
               x += cx[vw];
               y += cy[vw];
            }
            vx[idx] = x / (2 * radius + 1);
            vy[idx] = y / (2 * radius + 1);
         }
      }
      memcpy(cx, vx, getLastIndex() * sizeof(double));
      memcpy(cy, vy, getLastIndex() * sizeof(double));
   }
   
   int getNPoints() {
      return n + 1;
   }
   
   int getMaxContour() {
      int maxlength = 0;
      int idx = 0;
      for (int k = 0; k < numContours; k++) {
         int l = getContourLength(k);
         if (l > maxlength) {
            maxlength = l;
            idx = k;
         }
      }
      return idx;
   }
   
   void deleteContour(int k) {
      cl[k] = 0;
   }
   
   void cleanup(void) {
      free(cd);
      free(cl);
      free(co);
      free(cx);
      free(cy);
      free(cu);
      free(vx);
      free(vy);
      free(tips);
      free(minx);
      free(maxx);
      free(miny);
      free(maxy);
   }
}