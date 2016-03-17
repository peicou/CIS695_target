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

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


#include <EGL/egl.h>
#include "gUtil.h"
#include <IL/il.h>

//--------------------------------------------------------------------------------------
// Name: fslMulMatrix4x4
// Desc: 4x4 Matix Muliply
//--------------------------------------------------------------------------------------
void fslMultMatrix4x4( float *matC, float *matA, float *matB)
{
	matC[ 0] = matA[ 0] * matB[ 0] + matA[ 1] * matB[ 4] + matA[ 2] * matB[ 8] + matA[ 3] * matB[12];
	matC[ 1] = matA[ 0] * matB[ 1] + matA[ 1] * matB[ 5] + matA[ 2] * matB[ 9] + matA[ 3] * matB[13];
	matC[ 2] = matA[ 0] * matB[ 2] + matA[ 1] * matB[ 6] + matA[ 2] * matB[10] + matA[ 3] * matB[14];
	matC[ 3] = matA[ 0] * matB[ 3] + matA[ 1] * matB[ 7] + matA[ 2] * matB[11] + matA[ 3] * matB[15];
	matC[ 4] = matA[ 4] * matB[ 0] + matA[ 5] * matB[ 4] + matA[ 6] * matB[ 8] + matA[ 7] * matB[12];
	matC[ 5] = matA[ 4] * matB[ 1] + matA[ 5] * matB[ 5] + matA[ 6] * matB[ 9] + matA[ 7] * matB[13];
	matC[ 6] = matA[ 4] * matB[ 2] + matA[ 5] * matB[ 6] + matA[ 6] * matB[10] + matA[ 7] * matB[14];
	matC[ 7] = matA[ 4] * matB[ 3] + matA[ 5] * matB[ 7] + matA[ 6] * matB[11] + matA[ 7] * matB[15];
	matC[ 8] = matA[ 8] * matB[ 0] + matA[ 9] * matB[ 4] + matA[10] * matB[ 8] + matA[11] * matB[12];
	matC[ 9] = matA[ 8] * matB[ 1] + matA[ 9] * matB[ 5] + matA[10] * matB[ 9] + matA[11] * matB[13];
	matC[10] = matA[ 8] * matB[ 2] + matA[ 9] * matB[ 6] + matA[10] * matB[10] + matA[11] * matB[14];
	matC[11] = matA[ 8] * matB[ 3] + matA[ 9] * matB[ 7] + matA[10] * matB[11] + matA[11] * matB[15];
	matC[12] = matA[12] * matB[ 0] + matA[13] * matB[ 4] + matA[14] * matB[ 8] + matA[15] * matB[12];
	matC[13] = matA[12] * matB[ 1] + matA[13] * matB[ 5] + matA[14] * matB[ 9] + matA[15] * matB[13];
	matC[14] = matA[12] * matB[ 2] + matA[13] * matB[ 6] + matA[14] * matB[10] + matA[15] * matB[14];
	matC[15] = matA[12] * matB[ 3] + matA[13] * matB[ 7] + matA[14] * matB[11] + matA[15] * matB[15];
}


void fslPerspectiveMatrix4x4 ( float *m, float fov, float aspect, float zNear, float zFar)
{
	const float h = 1.0f/tan(fov*PI_OVER_360);
	float neg_depth = zNear-zFar;
	
	m[0] = h / aspect;
	m[1] = 0;
	m[2] = 0;
	m[3] = 0;
	
	m[4] = 0;
	m[5] = h;
	m[6] = 0;
	m[7] = 0;
	
	m[8] = 0;
	m[9] = 0;
	m[10] = (zFar + zNear)/neg_depth;
	m[11] = -1;
	
	m[12] = 0;
	m[13] = 0;
	m[14] = 2.0f*(zNear*zFar)/neg_depth;
	m[15] = 0;

}

void fslRotateMatrix4x4 (float *m, float angle, fslAxis axis)
{
	float radians = PI_OVER_360*2*angle;
   
 	float rotate[16] = {0};
	float store[16] = {0};

 	fslLoadIdentityMatrix4x4(rotate);

	switch (axis)
	{
		case FSL_X_AXIS:
			rotate[5] = cosf(radians);
			rotate[6] = -sinf(radians);
			rotate[9] = sinf(radians);
			rotate[10] = cosf(radians);
			fslMultMatrix4x4(store, rotate, m);
         memcpy( m, store, 16*sizeof(float) );
		break;
		case FSL_Y_AXIS:
			rotate[0] = cos(radians);
			rotate[2] = sin(radians);
			rotate[8] = -sin(radians);
			rotate[10] = cos(radians);
			fslMultMatrix4x4(store, rotate, m);
			memcpy( m, store, 16*sizeof(float) );
		break;		
		case FSL_Z_AXIS:
			rotate[0] = cos(radians);
			rotate[1] = -sin(radians);
			rotate[4] = sin(radians);
			rotate[5] = cos(radians);
			fslMultMatrix4x4(store, rotate, m);
         memcpy( m, store, 16*sizeof(float) );
		break;		
		default:
		printf("invalid axis \n");
		break;

	}
}

void fslTranslateMatrix4x4 (float *m, float transX, float transY, float transZ)
{
 	float trans[16] = {0};
 	fslLoadIdentityMatrix4x4(trans);

 	trans[12]=transX;
 	trans[13]=transY;
 	trans[14]=transZ;

 	fslMultMatrix4x4(m, trans, m);
}

void fslScaleMatrix4x4 (float *m, float scaleX, float scaleY, float scaleZ)
{
 	float scale[16] = {0};
 	fslLoadIdentityMatrix4x4(scale);

	scale[0]=scaleX;
	scale[5]=scaleY;
	scale[10]=scaleZ;

 	fslMultMatrix4x4(m, scale, m);
}

void fslLoadIdentityMatrix4x4 (float *m)
{
	m[0] = 1;
	m[1] = 0;
	m[2] = 0;
	m[3] = 0;
	
	m[4] = 0;
	m[5] = 1;
	m[6] = 0;
	m[7] = 0;
	
	m[8] = 0;
	m[9] = 0;
	m[10] = 1;
	m[11] = 0;
	
	m[12] = 0;
	m[13] = 0;
	m[14] = 0;
	m[15] = 1;
}

void fslPrintMatrix4x4(float *m){
	
	printf(" %f %f %f %f \n", m[0], m[1], m[2], m[3]);
	printf(" %f %f %f %f \n", m[4], m[5], m[6], m[7]);
	printf(" %f %f %f %f \n", m[8], m[9], m[10], m[11]);
	printf(" %f %f %f %f \n", m[12], m[13], m[14], m[15]);
	
}

/***************************************************************************************
* LoadShaders and CompileShader should be added to gUtil
***************************************************************************************/
//compiles either a fragment or vertex shader
int CompileShader(const char * FName, GLuint ShaderNum)
{
	FILE * fptr = NULL;
	fptr = fopen(FName, "rb");
	if (fptr == NULL)
	{
		return 0;
	}

	int length;
	fseek(fptr, 0, SEEK_END);
	length = ftell(fptr);
	fseek(fptr, 0 ,SEEK_SET);

	char * shaderSource = (char*)malloc(sizeof (char) * length);
	if (shaderSource == NULL)
	{
		fprintf(stderr, "Out of memory.\n");
		return 0;
	}

	size_t successful_read = fread(shaderSource, length, 1, fptr);
	if (!successful_read)
	{
		printf("failed to read shader source file\n");
	}
	glShaderSource(ShaderNum, 1, (const char**)&shaderSource, &length);
	glCompileShader(ShaderNum);

	free(shaderSource);
	fclose(fptr);

	GLint compiled = 0;
	glGetShaderiv(ShaderNum, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		// Retrieve error buffer size.
		GLint errorBufSize, errorLength;
		glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

		char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1);
		if (!infoLog)
		{
			// Retrieve error.
			glGetShaderInfoLog(ShaderNum, errorBufSize, &errorLength, infoLog);
			infoLog[errorBufSize + 1] = '\0';
			//fprintf(stderr, "%s\n", infoLog);
			printf("Compile error: %s\n",infoLog);
			free(infoLog);
		}
		return 0;
	}

	return 1;
}

void LoadShaders(const char * vShaderFName, const char * pShaderFName, GLuint & g_hShaderProgram)
{
	GLuint vertShaderNum = glCreateShader(GL_VERTEX_SHADER);
	GLuint pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);

	if (CompileShader(vShaderFName, vertShaderNum) == 0)
	{
		printf("vshader compile failed\n");
		return;
	}

	if (CompileShader(pShaderFName, pixelShaderNum) == 0)
	{
		printf("fshader compile failed\n");
		return;
	}

	g_hShaderProgram = glCreateProgram();

	glAttachShader(g_hShaderProgram , vertShaderNum);
	glAttachShader(g_hShaderProgram , pixelShaderNum);

	glLinkProgram(g_hShaderProgram );
	// Check if linking succeeded.
	GLint linked = false;
	glGetProgramiv(g_hShaderProgram , GL_LINK_STATUS, &linked);
	if (!linked)
	{
		// Retrieve error buffer size.
		GLint errorBufSize, errorLength;
		glGetShaderiv(g_hShaderProgram , GL_INFO_LOG_LENGTH, &errorBufSize);

		char * infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
		if (!infoLog)
		{
			// Retrieve error.
			glGetProgramInfoLog(g_hShaderProgram , errorBufSize, &errorLength, infoLog);
			infoLog[errorBufSize + 1] = '\0';
			fprintf(stderr, "%s", infoLog);

			free(infoLog);
		}
		printf("shader program link failed\n");
		return;
	}
	glDeleteShader(vertShaderNum);
	glDeleteShader(pixelShaderNum);
}

//vanilla egl initialization stuff
int EGLinit(EGLDisplay &eglDisplay, EGLSurface &eglSurface) {
   EGLContext		eglContext = 0;
   NativeDisplayType 	display;
   NativeWindowType 	window;

	printf("entering init\n");
	
	EGLint configAttribs[] =
	{
		EGL_RED_SIZE,       5,
		EGL_GREEN_SIZE,     6,
		EGL_BLUE_SIZE,      5,
		EGL_ALPHA_SIZE,     EGL_DONT_CARE,
		EGL_DEPTH_SIZE,     16,
		EGL_STENCIL_SIZE,   EGL_DONT_CARE,
		EGL_SAMPLE_BUFFERS, EGL_DONT_CARE,
		EGL_SAMPLES,        EGL_DONT_CARE, //2, 4, to enable FSAA, EGL_DONT_CARE to  disable
		EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
		EGL_MIN_SWAP_INTERVAL, 1,
		EGL_NONE
	};

	EGLint ctxAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLConfig configs[10];
	EGLint matchingConfigs;

	display = fbGetDisplayByIndex(0);
	
	int w, h, t, l;
	l=t=0;
	fbGetDisplayGeometry(display, &w, &h);
	window = fbCreateWindow(display, t, l, 1024, 768); //TODO fix hardcoded resolution
	
	eglDisplay = eglGetDisplay(display);
	if (!eglInitialize(eglDisplay, NULL, NULL))
	{
		printf("error in eglInitialize\n");
		return 1;
	}

	if (!eglChooseConfig(eglDisplay, configAttribs, &configs[0], 10, &matchingConfigs))
	{
		printf("error in eglChooseConfig\n");
		return 1;
	}

	if (matchingConfigs < 1)
	{
		printf("not enough matching configurations\n");
		return 1;
	}

	eglSurface = eglCreateWindowSurface(eglDisplay, configs[0], window, NULL);
	if (eglSurface == EGL_NO_SURFACE)
	{
		printf("error in eglCreateWindowSurface\n");
		return 1;
	}

	eglContext = eglCreateContext(eglDisplay, configs[0], NULL, ctxAttribs);
	if (eglContext == EGL_NO_CONTEXT)
	{
		printf("error in eglCreateContext\n");
		return 1;
	}

	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	
	return 0;
}

//vanilla egl deinitialization stuff
void EGLdeinit(EGLDisplay &eglDisplay)
{
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);
	eglReleaseThread();
}

GLuint CreateStaticCubemap()
{
	ILboolean success;
	char texName[] = "resources/cm/1.jpg";
	ILenum errorEnum;

	/* create and fill array with DevIL texture ids */
	ILuint imageIds[6];
	ilGenImages(6, imageIds); 

	/* create and fill array with GL texture ids */
	GLuint textureObject;
    glGenTextures(1, &textureObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureObject);
	/* get iterator */

	for (int i =0; i < 6 ; ++i)
	{
		texName[13] = i+49;
		ilBindImage(imageIds[i]); /* Binding of DevIL image name */
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 

		success = ilLoadImage((ILstring)texName);
		
		if (success) {
			/* Convert image to RGBA */
			ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 

			/* Create and load textures to OpenGL */
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
			glTexImage2D(face, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		}
		else {
			errorEnum = ilGetError();
			printf("errorEnum ilLoadImage: %i\n", errorEnum);
			printf("Couldn't load cubemap.\n");
		}
	}
	/* Because we have already copied image data into texture data
	we can release memory used by image. */
	ilDeleteImages(6, imageIds); 

	//Cleanup
	//delete [] imageIds;
	return textureObject;
}

void initSkybox(GLuint *sbVBO, GLuint &sbPosLoc)
{
	float cubeArray[] = {1, 1, 1,
						  -1, 1, 1,
						  -1, -1, 1,
						  1, -1, 1,
						  1, -1, -1,
						  1, 1, -1,
						  -1, 1, -1,
						  -1, -1, -1};

	GLubyte indexArray[] = {0, 1, 2, 0, 2, 3,
							 0, 3, 4, 0, 4, 5,
							 0, 5, 6, 0, 6, 1,
							 7, 6, 1, 7, 1, 2,
							 7, 4, 5, 7, 5, 6,
							 7, 2, 3, 7, 3, 4};

	glGenBuffers(2, sbVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sbVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeArray), cubeArray, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sbVBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexArray), indexArray, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, sbVBO[0]);
	glVertexAttribPointer(sbPosLoc, 3, GL_FLOAT, 0, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sbVBO[1]);
}

void renderSkybox(GLuint cubehandle, GLuint sbShaderProgram, GLuint sbVMLoc, GLuint sbPMLoc, 
				  float *matModelView, float *matProj, GLuint sbPosLoc, GLuint *sbVBO)
{
	glDisable(GL_DEPTH_TEST);   // skybox should be drawn behind anything else
	glDisable( GL_CULL_FACE );
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubehandle);
	glUseProgram(sbShaderProgram);   
	glUniformMatrix4fv( sbVMLoc, 1, 0, matModelView );
    glUniformMatrix4fv( sbPMLoc, 1, 0, matProj );
	
	glBindBuffer(GL_ARRAY_BUFFER, sbVBO[0]);
	glVertexAttribPointer(sbPosLoc, 3, GL_FLOAT, 0, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sbVBO[1]);

	glEnableVertexAttribArray(sbPosLoc);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
	glDisableVertexAttribArray(sbPosLoc);
	glEnable(GL_DEPTH_TEST);
	glEnable( GL_CULL_FACE );
}

queue *queueInit (void)
{
	queue *q;

	q = (queue *)malloc (sizeof (queue));
	if (q == NULL) return (NULL);

	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;
	q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->mut, NULL);
	q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notFull, NULL);
	q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notEmpty, NULL);
	
	return (q);
}

void queueDelete (queue *q)
{
	pthread_mutex_destroy (q->mut);
	free (q->mut);	
	pthread_cond_destroy (q->notFull);
	free (q->notFull);
	pthread_cond_destroy (q->notEmpty);
	free (q->notEmpty);
	free (q);
}

void queueAdd (queue *q, int in)
{
	q->buf[q->tail] = in;
	q->tail++;
	if (q->tail == QUEUESIZE)
		q->tail = 0;
	if (q->tail == q->head)
		q->full = 1;
	q->empty = 0;

	return;
}

void queueDel (queue *q, int *out)
{
	*out = q->buf[q->head];

	q->head++;
	if (q->head == QUEUESIZE)
		q->head = 0;
	if (q->head == q->tail)
		q->empty = 1;
	q->full = 0;

	return;
}
