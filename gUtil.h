/* Author: Francisco Sandoval */

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

#define PI_OVER_360		0.00872664f //self descriptive. saves compute time :)

#define BACKLOG 5     // how many pending connections queue will hold
#define QUEUESIZE 100

 #define MAXDATASIZE 100 // max number of bytes we can get at once 

/*
 * Axis - enumeration for x, y, z axis
 */
typedef enum Axis_e
{
	X_AXIS, Y_AXIS, Z_AXIS
} Axis;

/*
 * vector3d_i - structure that holds a 3D int vector
 * x, y, z - values of int vector
 */
typedef struct vector3d_int
{
	int x;
	int y;
	int z;
}vector3d_i;

/*
 * vector3d_f - structure that holds a 3D float vector
 * x, y, z - values of float vector
 */
typedef struct vector3d_float
{
	float x;
	float y;
	float z;
}vector3d_f;

/* 
 * queue - FIFO Structure to keep messages
 * buf - array to store messages
 * head, tail - pointers to first and last message
 * full, empty - flags to check queue status
 * mut - mutex to lock resource
 * notFull, notEmpty - pthread conditional flags
 */
typedef struct {
	int buf[QUEUESIZE];
	long head, tail;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;


/*
 * TCPServer - waits for connections, launched in POSIX thread
 * param - pointer to shared queue resource
 */
void *TCPServer (void *param);

/*
 * MultMatrix4x4 - multiply a 4x4 matrix. C = A * B
 * matC - stores the multiplication result
 * matA, matB - operands
 */
void MultMatrix4x4( float *matC, float *matA, float *matB);

/*
 * ScaleMatrix4x4 - scale a 4x4 matrix in 3 axis.
 * m - matrix to be scaled
 * scaleX, scaleY, scaleZ - factor to be scaled by.
 */
void ScaleMatrix4x4 (float *m, float scaleX, float scaleY, float scaleZ);

/*
 * RotateMatrix4x4 - Rotate a matrix by "angle" grades in a single axis
 * m - matrix to be rotated
 * angle - rotation angle
 * axis - axis where the rotation will take part
 */
void RotateMatrix4x4 (float *m, float angle, Axis axis);

/*
 * TranslateMatrix4x4 - translate a 4x4 matrix in 3 axis
 * m - matrix to be translated
 * transX, transY, transZ - translation units in different angles
 */
void TranslateMatrix4x4 (float *m, float transX, float transY, float transZ);

/*
 * PerspectiveMatrix4x4 - calculates a perspective projection matrix
 * m - stores the calculated matrix.
 * fov - field of view in degrees.
 * aspect - aspect ratio of projection matrix to be calculated
 * zNear - near z value
 * zFar - far z value
 */
void PerspectiveMatrix4x4 ( float *m, float fov, float aspect, float zNear, float zFar);

/*
 * LoadIdentityMatrix4x4 - fills a 4x4 matrix with 0
 * m - matrix to be cleared
 */
void LoadIdentityMatrix4x4 (float *m);

/*
 * PrintMatrix4x4 - prints matrix values. good for debugging.
 * m - matrix to print.
 */
void PrintMatrix4x4(float *m);

/*
 * CompileShader - compiles a vertex or fragment shader.
 * FName - char array containing the shader code
 * ShaderNum - handler to the created shader
 */
int CompileShader(const char * FName, GLuint ShaderNum);

/*
 * LoadShaders - links a fragment and a vertex shader, creates a shader program
 * vShaderFName, pShaderFName - shaders to be linked
 * g_hShaderProgram - shader program ready to be used
 */
void LoadShaders(const char * vShaderFName, const char * pShaderFName, GLuint & g_hShaderProgram);

/*
 * EGLinit - prepares an EGL context so OGLES2 can draw
 * eglDisplay - egl display structure
 * eglSurface - eglSurface structure
 */
int EGLinit(EGLDisplay &eglDisplay, EGLSurface &eglSurface);

/*
 * EGLdeinit - deallocates resources used by the EGL context
 * eglDisplay - the display context to be terminated
 */
void EGLdeinit(EGLDisplay &eglDisplay);

/*
 * CreateSaticCubemap - creates a cubemap made from static pictures -hardcoded-
 */
GLuint CreateStaticCubemap(void);

/* initSkybox - prepares resources to draw a skybox
 * sbVBO - vertex buffer object that will have the skybox vertices
 * sbPosLoc - Location in shader program for the vertices positions
 */
void initSkybox(GLuint *sbVBO, GLuint &sbPosLoc);

/*
 * renderSkybox - draws the skybox. to be called after initSkybox returns successfully
 * cubehandle - handle to the cubemap to be used for the skybox
 * sbShaderProgram - linked skybox shader program
 * sbVMLoc, sbPMLoc - locations of Vertex and Projection matrices in shader program
 * madModelView, matProj - modelView and Projection matrices
 * sbPosLoc - location of vertices positions in shader program
 * sbVBO - vertex buffer object for skybox
 */
void renderSkybox(GLuint cubehandle, GLuint sbShaderProgram, GLuint sbVMLoc, GLuint sbPMLoc, 
				  float *matModelView, float *matProj, GLuint sbPosLoc, GLuint *sbVBO);

/*
 * queueInit - returns a pointer to a queue structure
 */
queue *queueInit (void);

/*
 * queueDelete - deallocates and deletes the queue object
 * q - pointer to queue to finish
 */
void queueDelete (queue *q);

/*
 * queueAdd - adds a message to the queue
 * q - pointer to queue where the message will be added
 * in - message to be added
 */
void queueAdd (queue *q, int in);

/*
 * queueDel - deletes a message from the queue
 * q - queue to read and delete the message from
 * out - pointer to the deleted message
 */
void queueDel (queue *q, int *out);

/*
 * get_in_addr - 
 * sa - A sockaddr, this is an IPv4 or 6 address structure
 */
void *get_in_addr(struct sockaddr *sa);

/*
 * sigchld_handler - dead child reaper f(o.0)f  \(0.0)/
 * s - signal
 */
void sigchld_handler(int s);

#endif //_GUTIL_H_

