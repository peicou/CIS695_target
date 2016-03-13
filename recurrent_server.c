/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <math.h>
#include <vector>
#include <map>
#include <sys/time.h>
#include "obj3d.h"

#define MAXDATASIZE 100 // max number of bytes we can get at once 

#define BACKLOG 5     // how many pending connections queue will hold
#define QUEUESIZE 10
#define LOOP 20

void *TCPServer (void *args);
void *ThreeDeeApp (void *args);

const int width  = 1024; //your display x res
const int height = 768;  //your display y res

int frames = 0;

//needed for assImp. info stream
struct aiLogStream stream;

//native EGL calls
EGLDisplay		eglDisplay;
EGLSurface		eglSurface;

// Global Variables, shader handle and program handle
GLuint       g_hPShaderProgram   = 0;
GLuint       g_hSBShaderProgram   = 0;
GLuint       g_hTXShaderProgram   = 0;

GLuint       viewMatrixLoc		= 0;
GLuint       projMatrixLoc      = 0;

GLuint sbVMLoc, sbPMLoc, sbPosLoc, sbTxHandle;
GLuint sbVBO[2];

float matProj[16] = {0};
float matModelView[16] = {0};
float matSkyBox[16] = {0};

Obj3d * assets;
typedef struct {
	int buf[QUEUESIZE];
	long head, tail;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, int in);
void queueDel (queue *q, int *out);

int preRender()
{

	// load and compiler vertex/fragment shaders.
	LoadShaders("resources/shaders/vs_phong.vert", "resources/shaders/fs_phong.frag", g_hPShaderProgram);
	LoadShaders("resources/shaders/vs_skybox.vert", "resources/shaders/fs_skybox.frag", g_hSBShaderProgram);
	LoadShaders("resources/shaders/vs_texture.vert", "resources/shaders/fs_texture.frag", g_hTXShaderProgram );

	//init assImp stream
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);

	//write stream to log 
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&stream);
	
	ilInit(); //if you use textures, do this.

	if (g_hSBShaderProgram  != 0)	
	{
		sbPMLoc= glGetUniformLocation(g_hSBShaderProgram, "projMatrix");
		sbVMLoc= glGetUniformLocation(g_hSBShaderProgram, "viewMatrix");
		sbPosLoc = glGetAttribLocation(g_hSBShaderProgram, "positionSB");
		initSkybox(sbVBO, sbPosLoc);
	} else {
		printf("skybox shader program not compiled/linked\n");
		return 1;
	}
	if (g_hPShaderProgram  != 0)
	{
		projMatrixLoc= glGetUniformLocation(g_hPShaderProgram, "projMatrix");
		viewMatrixLoc= glGetUniformLocation(g_hPShaderProgram, "viewMatrix");
	} else {
		printf("phong shader program not compiled/linked\n");
		return 1;
	}
	if (g_hTXShaderProgram  != 0)
	{
		projMatrixLoc= glGetUniformLocation(g_hTXShaderProgram, "projMatrix");
		viewMatrixLoc= glGetUniformLocation(g_hTXShaderProgram, "viewMatrix");
	} else {
		printf("Texture shader program not compiled/linked\n");
		return 1;
	}

	// Build a perspective projection matrix. 
	fslPerspectiveMatrix4x4 ( matProj, 70.f, 1.3333f, 0.1f, 200.f);
	fslLoadIdentityMatrix4x4 (matModelView);
	fslTranslateMatrix4x4 (matModelView, 0, -5.0f, -15.0f); //(0, -2, -10)
	fslLoadIdentityMatrix4x4 (matSkyBox);
	fslRotateMatrix4x4(matSkyBox, 180, FSL_Z_AXIS);

	return 0;

}

// sets matrices, renders objects
void Render(Obj3d *assets, float Xrot, float Yrot, float Zrot, float zoomtr)
{
	// Clear background.
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	//rotate skyBox with object so it looks like the camera is rotating
	fslRotateMatrix4x4 (matSkyBox, -Yrot, FSL_Y_AXIS);
	renderSkybox(sbTxHandle, g_hSBShaderProgram, sbVMLoc, sbPMLoc,
			matSkyBox, matProj, sbPosLoc, sbVBO);
	fslRotateMatrix4x4 (matModelView, Yrot, FSL_Y_AXIS);
	assets->draw(matModelView, matProj, viewMatrixLoc, projMatrixLoc);
   
	glFlush();
	
}

void RenderCleanup(Obj3d *assets)
{
	
	// free assImp scene resources
	aiReleaseImport(assets->getScene());

	// detach assImp log
	aiDetachAllLogStreams();
}



// Cleanup the shaders.
void DestroyShaders()
{
	glDeleteProgram(g_hPShaderProgram );
	glDeleteProgram(g_hSBShaderProgram );
	glDeleteProgram(g_hTXShaderProgram );
	glUseProgram(0);
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *TCPServer (void *param)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int port;
    int numbytes = 0;
	char buf[MAXDATASIZE];
	char cPort[256];
	queue *fifo;

	fifo = (queue *)param;

	memset(&hints, 0, sizeof hints);
    memset(&buf, 0, sizeof buf);
    
    queueDel(fifo, &port);
    sprintf(cPort, "%d", port);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, cPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections on port %i...\n", port);
 	sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
    }
    inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
    printf("server: got connection from %s\n", s);
	close(sockfd);
    while(1) { 
        if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
		    perror("recv");
		    exit(1);
		}
		if(numbytes == 0) {			
			close(new_fd);
    		printf("connection from %s terminated\n", s);
			exit(0);
		}

		pthread_mutex_lock(fifo->mut);
		while(fifo->full){
			printf("server: queue full \n");
			pthread_cond_wait (fifo->notFull, fifo->mut);
		}
		queueAdd(fifo, atoi(buf));
		printf("server: %s added to queue\n",buf);
		pthread_mutex_unlock(fifo->mut);
		pthread_cond_signal(fifo->notEmpty);
		usleep(1000);
    }

	return (NULL);
}

void *ThreeDeeApp (void *param)
{
	queue *fifo;
	int msg;
	fifo = (queue *)param;
	
	int frameCount = 0;
	unsigned int start = fslGetTickCount();
	float Xrotation, Yrotation, Zrotation, zoom = 0;
	assets = new Obj3d(true);
	
	EGLinit(eglDisplay, eglSurface);

	if (1==preRender())
	{
		EGLdeinit(eglDisplay);
		exit(1);
	}

	assets->start(g_hTXShaderProgram, "resources/models/tumbler/BatMobil.obj", *assets);

    if(!assets->getScene())
	{
		printf("scene could not be loaded\n");	
		exit(1);
	}

	printf("scene loaded\n");
	assets->setCubeHandle(CreateStaticCubemap());
	sbTxHandle = assets->getCubeHandle();
	
	for (;;)
	{
		if(!fifo->empty)
		{
			pthread_mutex_lock(fifo->mut);
			queueDel(fifo, &msg);
			pthread_mutex_unlock(fifo->mut);
			pthread_cond_signal(fifo->notFull);
			printf("3D app: received %i\n", msg);
		}
		Render(assets, Xrotation, 1, Zrotation, zoom);
		++ frameCount;
		eglSwapBuffers(eglDisplay, eglSurface);
	}

	unsigned int end = fslGetTickCount();
	float fps = frameCount / ((end - start) / 1000.0f);
	printf("%d frames in %d ticks -> %.3f fps\n", frameCount, end - start, fps);
	RenderCleanup(assets);
	
	// cleanup
	DestroyShaders();
	EGLdeinit(eglDisplay);

	return (NULL);
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

int main(int argc, char *argv[])
{
    int port = 0;
	
	if (argc !=2)
	{
		printf("Usage: ./rs <port number>\n");
		return 1;
	}

	port = atoi(argv[1]);

	if (port<2000 || port>65535)
	{
		printf("port must be an integer between 2000 and 65535\n");
		printf("invalid value %i", port);
		return 1;
	}

	queue *fifo;
	pthread_t srv, app;

	fifo = queueInit();
	if (fifo==NULL)
	{
		printf("queueInit failed\n");
		exit (1);
	}
	queueAdd(fifo, port);

	pthread_create(&srv, NULL, TCPServer, fifo);
	pthread_create(&app, NULL, ThreeDeeApp, fifo);
	pthread_join(srv, NULL);
	pthread_join(app, NULL);

	queueDelete(fifo);
	
	return 0;
}
