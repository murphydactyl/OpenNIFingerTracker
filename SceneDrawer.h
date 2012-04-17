/****************************************************************************
 *                                                                           *
 *  OpenNI 1.x Alpha                                                         *
 *  Copyright (C) 2011 PrimeSense Ltd.                                       *
 *                                                                           *
 *  This file is part of OpenNI.                                             *
 *                                                                           *
 *  OpenNI is free software: you can redistribute it and/or modify           *
 *  it under the terms of the GNU Lesser General Public License as published *
 *  by the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  OpenNI is distributed in the hope that it will be useful,                *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the             *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU Lesser General Public License *
 *  along with OpenNI. If not, see <http://www.gnu.org/licenses/>.           *
 *                                                                           *
 ****************************************************************************/
#ifndef XNV_POINT_DRAWER_H_
#define XNV_POINT_DRAWER_H_

#include <XnCppWrapper.h>

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd);

void XN_CALLBACK_TYPE MyCalibrationInProgress(xn::SkeletonCapability& capability, XnUserID id, XnCalibrationStatus calibrationError, void* pCookie);
void XN_CALLBACK_TYPE MyPoseInProgress(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID id, XnPoseDetectionStatus poseError, void* pCookie);

unsigned int getClosestPowerOfTwo(unsigned int n);
GLuint initTexture(void** buf, int& width, int& height);
void DrawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY);
void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY);
void glPrintString(void *font, char *str);
void DrawLimb(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
const XnChar* GetCalibrationErrorString(XnCalibrationStatus error);
const XnChar* GetPoseErrorString(XnPoseDetectionStatus error);
void drawVideo(const xn::ImageMetaData& imd);
void drawContours(XnDepthPixel *depth);
void drawFingers(void);
void drawHandsAndContours(XnDepthPixel *depth);
unsigned long long GetTimeSinceBootInMilliseconds(void);
double tic(void);
double toc(void);
double avg(XnDepthPixel* depth);

#endif
