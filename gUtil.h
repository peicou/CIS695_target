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
#include <pthread.h>
#include <linux/input.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PI_OVER_360		0.00872664f

#define BACKLOG 5     // how many pending connections queue will hold
#define QUEUESIZE 100

 #define MAXDATASIZE 100 // max number of bytes we can get at once 

typedef enum Axis_e
{
	X_AXIS, Y_AXIS, Z_AXIS
} Axis;

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

typedef struct {
	int buf[QUEUESIZE];
	long head, tail;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

void *TCPServer (void *param);

void MultMatrix4x4( float *matC, float *matA, float *matB);
void ScaleMatrix4x4 (float *m, float scaleX, float scaleY, float scaleZ);
void RotateMatrix4x4 (float *m, float angle, Axis axis);
void TranslateMatrix4x4 (float *m, float transX, float transY, float transZ);
void PerspectiveMatrix4x4 ( float *m, float fov, float aspect, float zNear, float zFar);
void LoadIdentityMatrix4x4 (float *m);
void PrintMatrix4x4(float *m);

int CompileShader(const char * FName, GLuint ShaderNum);
void LoadShaders(const char * vShaderFName, const char * pShaderFName, GLuint & g_hShaderProgram);
int EGLinit(EGLDisplay &eglDisplay, EGLSurface &eglSurface);
void EGLdeinit(EGLDisplay &eglDisplay);
GLuint CreateStaticCubemap(void);
void initSkybox(GLuint *sbVBO, GLuint &sbPosLoc);
void renderSkybox(GLuint cubehandle, GLuint sbShaderProgram, GLuint sbVMLoc, GLuint sbPMLoc, 
				  float *matModelView, float *matProj, GLuint sbPosLoc, GLuint *sbVBO);

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

void *get_in_addr(struct sockaddr *sa);
void sigchld_handler(int s);

#endif //_GUTIL_H_

