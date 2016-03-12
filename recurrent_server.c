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

#define MAXDATASIZE 100 // max number of bytes we can get at once 

#define BACKLOG 5     // how many pending connections queue will hold
#define QUEUESIZE 10
#define LOOP 20

void *TCPServer (void *args);
void *ThreeDeeApp (void *args);

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
		usleep(500000);
    }

	return (NULL);
}

void *ThreeDeeApp (void *param)
{
	queue *fifo;
	int msg;
	fifo = (queue *)param;
	
	while (1)
	{
		pthread_mutex_lock(fifo->mut);
		while(fifo->empty)
		{
			printf("3D app: no messages\n");
			pthread_cond_wait(fifo->notEmpty, fifo->mut);
		}
		queueDel(fifo, &msg);
		pthread_mutex_unlock(fifo->mut);
		pthread_cond_signal(fifo->notFull);
		printf("3D app: received %i\n", msg);
		usleep(500000);
	}

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
