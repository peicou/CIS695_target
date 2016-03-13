/*
 *Copyright (c) 2012 Freescale Semiconductor, Inc.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without modification,
 *are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this 
 *list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this 
 *list of conditions and the following disclaimer in the documentation and/or other
 *materials provided with the distribution.
 *
 * Neither the name of the Freescale Semiconductor, Inc. nor the names of its 
 *contributors may be used to endorse or promote products derived from this software
 *without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _GUTIL_H_
#define _GUTIL_H_

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h> 
#include <malloc.h> 
#include <math.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <linux/input.h>
//#include <tslib.h> //TODO include tslib functionality

#define PI_OVER_360		0.00872664f


typedef enum fslAxis_e
{
	FSL_X_AXIS, FSL_Y_AXIS, FSL_Z_AXIS
} fslAxis;

typedef struct vector3d_int
{
	int x;
	int y;
	int z;
}vector3d_i;

typedef struct vector3d_float
{
	float x;
	float y;
	float z;
}vector3d_f;

//--------------------------------------------------------------------------------------
// Name: fslGetTickCount
// Desc: Helper function to get current time
//--------------------------------------------------------------------------------------
unsigned int fslGetTickCount();

void fslMultMatrix4x4( float *matC, float *matA, float *matB);
void fslScaleMatrix4x4 (float *m, float scaleX, float scaleY, float scaleZ);
void fslRotateMatrix4x4 (float *m, float angle, fslAxis axis);
void fslTranslateMatrix4x4 (float *m, float transX, float transY, float transZ);
void fslPerspectiveMatrix4x4 ( float *m, float fov, float aspect, float zNear, float zFar);
void fslLoadIdentityMatrix4x4 (float *m);
void fslPrintMatrix4x4(float *m);
int CompileShader(const char * FName, GLuint ShaderNum);
void LoadShaders(const char * vShaderFName, const char * pShaderFName, GLuint & g_hShaderProgram);
int EGLinit(EGLDisplay &eglDisplay, EGLSurface &eglSurface);
void EGLdeinit(EGLDisplay &eglDisplay);
GLuint CreateStaticCubemap(void);
void initSkybox(GLuint *sbVBO, GLuint &sbPosLoc);
void renderSkybox(GLuint cubehandle, GLuint sbShaderProgram, GLuint sbVMLoc, GLuint sbPMLoc, 
				  float *matModelView, float *matProj, GLuint sbPosLoc, GLuint *sbVBO);
#endif //_GUTIL_H_

