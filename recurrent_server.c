/*
** Author - Francisco Sandoval
 * Interfacing the UE4 TCPActorComponent with an embedded target
 * CIS695
*/

#include "obj3d.h"

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
GLuint       g_hTXShaderProgram   = 0;

GLuint       viewMatrixLoc		= 0;
GLuint       projMatrixLoc      = 0;

GLuint sbVMLoc, sbPMLoc, sbPosLoc, sbTxHandle;
GLuint sbVBO[2];

float matProj[16] = {0};
float matModelView[16] = {0};
float matSkyBox[16] = {0};

Obj3d * assets;

int preRender()
{

	// load and compiler vertex/fragment shaders.
	LoadShaders("resources/shaders/vs_phong.vert", "resources/shaders/fs_phong.frag", g_hPShaderProgram);
	LoadShaders("resources/shaders/vs_texture.vert", "resources/shaders/fs_texture.frag", g_hTXShaderProgram );

	//init assImp stream
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);

	//write stream to log 
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_FILE,"assimp_log.txt");
	aiAttachLogStream(&stream);
	
	ilInit(); //if you use textures, do this.

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
	glDeleteProgram(g_hTXShaderProgram );
	glUseProgram(0);
}

void sigchld_handler(int s)
{
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

	RenderCleanup(assets);
	
	// cleanup
	DestroyShaders();
	EGLdeinit(eglDisplay);

	return (NULL);
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
