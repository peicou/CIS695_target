/*
** Author - Francisco Sandoval
 * Interfacing the UE4 TCPActorComponent with an embedded target
 * CIS695
*/

#include "obj3d.h"

void *ThreeDeeApp (void *args);

const int width  = 1024; //your display x res
const int height = 768;  //your display y res

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

float matProj[16] = {0};
float matModelView[16] = {0};

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
	PerspectiveMatrix4x4 ( matProj, 70.f, 1.3333f, 0.1f, 200.f);
	LoadIdentityMatrix4x4 (matModelView);
	TranslateMatrix4x4 (matModelView, 0, 0, -20.0f); //(0, -2, -10)
	RotateMatrix4x4 (matModelView, -90, X_AXIS);
	RotateMatrix4x4 (matModelView, -90, Y_AXIS);
	return 0;

}

void RenderCleanup(Obj3d *assets)
{
	// free assImp scene resources
	aiReleaseImport(assets->getScene());
	// detach assImp log
	aiDetachAllLogStreams();
}


// sets matrices, renders objects
void Render(Obj3d *assets, float Xrot, float Yrot, float Zrot, float zoomtr)
{
	// Clear background.
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	RotateMatrix4x4 (matModelView, Yrot, Y_AXIS);
	assets->draw(matModelView, matProj, viewMatrixLoc, projMatrixLoc);
   
	glFlush();
	
}

// Cleanup the shaders.
void DestroyShaders()
{
	glDeleteProgram(g_hPShaderProgram );
	glDeleteProgram(g_hTXShaderProgram );
	glUseProgram(0);
}

void *ThreeDeeApp (void *param)
{
	queue *fifo;
	int msg, buff = 0;
	fifo = (queue *)param;
	
	float Yrotation = 0;
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
			//printf("3D app: received %i\n", msg);
		}
		if (msg != buff)
		{
			Yrotation = msg - buff;
			buff = msg;
			Render(assets, 0, Yrotation*3, 0, 0);
		} else 
		{
			Render(assets, 0, 0, 0, 0);	
		}
		Yrotation = 0;
		//msg=0;

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
