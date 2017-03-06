/*
 * Angus Jo Kwang
 * anj218
 * 11136793
 */

#include<errno.h>

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/syscall.h>
#include"queue.h"

volatile sig_atomic_t running = 1;

void sig_handler(int signum){
	printf("\n");
	running = 0;
}

void *mainp(void *arg);

struct Argp{
	int sock;
	struct sockaddr_in destAddr;
	struct QUEUE *queue;
	int delay;
	char *item;
} Argp;

int CreateUDPSocket(unsigned short port){
	int sock;
	struct sockaddr_in servAddr;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "socket create failed\n");
		exit(1);
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);

	if(bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
		fprintf(stderr, "socket bind failed\n");
		exit(1);
	}

	return sock;
}

int recvUDPclient(int sock, char *msg){
	struct sockaddr_in remaddr;
	socklen_t addrlen = sizeof(remaddr);
	int recvlen;
	unsigned char buf[100];

	//printf("Waiting on port\n");
	recvlen = recvfrom(sock, buf, 100, 0, (struct sockaddr *)&remaddr, &addrlen);
	if (recvlen > 0) {
		buf[recvlen] = '\0';
		printf("received message: %s\n", buf);
		strcat(msg, buf);
	}
	return ntohs(remaddr.sin_port);
}

main(int argc, char **argv){
	pthread_t threadID;
	struct sigaction a;
	a.sa_handler = sig_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGINT, &a, NULL);

	int emuSock, sockB, sockC;
	struct sockaddr_in addrB, addrC;
	unsigned short portA, portB, portC;
	float p, d;
	int q, di;
	int toPort;
	char *newMsg = malloc(sizeof(char)*100);

	srand(time(NULL));
	if(argc != 7){
		fprintf(stderr, "Usage: UDP-emulator <p> <d> <q> <port A> <port B> <port C>\n");
		fprintf(stderr, "p - drop probability\n");
		fprintf(stderr, "d - delay value\n");
		fprintf(stderr, "q - maximum queue size\n");
		exit(1);
	}

	p = atof(argv[1]);
	d = atof(argv[2]);
	di = (int)(d * 1000000);
	q = atoi(argv[3]);
	portA = atoi(argv[4]);
	portB = atoi(argv[5]);
	portC = atoi(argv[6]);

	struct QUEUE *queueB = (QUEUE *)malloc(sizeof(QUEUE));
	struct QUEUE *queueC = (QUEUE *)malloc(sizeof(QUEUE));
	if(QueueInit(queueB, q) != 1){
		fprintf(stderr, "Queue B init failed\n");
		exit(1);
	}
	if(QueueInit(queueC, q) != 1){
		fprintf(stderr, "Queue C init failed\n");
		exit(1);
	}

	emuSock = CreateUDPSocket(portA);
	sockB = CreateUDPSocket(0);
	sockC = CreateUDPSocket(0);

	memset(&addrB, 0, sizeof(addrB));
	addrB.sin_family = AF_INET;
	addrB.sin_addr.s_addr = htonl(INADDR_ANY);
	addrB.sin_port = htons(portB);

	memset(&addrC, 0, sizeof(addrC));
	addrC.sin_family = AF_INET;
	addrC.sin_addr.s_addr = htonl(INADDR_ANY);
	addrC.sin_port = htons(portC);

	printf("UDP emulator running\n");

	while(running){
		usleep(10);
		newMsg[0] = '\0';
		toPort = recvUDPclient(emuSock, newMsg);
		if(rand() % 100 < p * 100){
			printf("msg dropped from port %d \n", toPort);
		}
		else{
			printf("msg to queue from port %d\n", toPort);
			struct Argp *argpTemp = (struct Argp *)malloc(sizeof(struct Argp));
			argpTemp->delay = di;
			argpTemp->item = malloc(sizeof(char)*100);
			strcpy(argpTemp->item, newMsg);
			if(toPort == portB){
				argpTemp->sock = sockB;
				argpTemp->destAddr = addrC;
				argpTemp->queue = queueB;
				if(pthread_create(&threadID, NULL, mainp, (void*)argpTemp) != 0){
					fprintf(stderr, "server pthread_create failed\n");
					exit(1);
				}
			}else if (toPort == portC){
				argpTemp->sock = sockC;
				argpTemp->destAddr = addrB;
				argpTemp->queue = queueC;
				if(pthread_create(&threadID, NULL, mainp, (void*)argpTemp) != 0){
					fprintf(stderr, "server pthread_create failed\n");
					exit(1);
				}
			}else{
				printf("Unidentified source port\n");
			}
		}
	}

	printf("Shutting down emulator\n");

	free(newMsg);
	free(queueB);
	free(queueC);
	close(emuSock);
	close(sockB);
	close(sockC);
	exit(0);
}

void *mainp(void *arg){
	int sock, delay;
	QUEUE *queue;
	char *ret;
	struct sockaddr_in destAddr;
	char *item = malloc(sizeof(char)*100);

	pid_t tid = syscall(__NR_gettid);

	pthread_detach(pthread_self());

	sock = ((struct Argp *)arg)->sock;
	destAddr = ((struct Argp *)arg)->destAddr;
	queue = ((struct Argp *)arg)->queue;
	delay = ((struct Argp *)arg)->delay;
	strcpy(item, ((struct Argp *)arg)->item);
	ret = malloc(sizeof(char)*100);
	free(((struct Argp *)arg)->item);
	free(arg);

	//printf("thread %d running\n", tid);

	if(push(queue, item) <= 0){
		printf("thread %d: queue full, message dropped\n", tid);
		return NULL;
	}

	usleep(delay);

	if(pop(queue, ret) == 1){
		//printf("thread %d: queue popped\n", tid);
		if(sendto(sock, item, strlen(item), 0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0){
			fprintf(stderr, "thread sentto failed: %s\n", strerror(errno));
			exit(1);
		}
		printf("thread %d: send to client port: %i\n", tid, ntohs(destAddr.sin_port));
		printf("thread %d: message: %s\n", tid, item);
	}
	else{
		fprintf(stderr, "pop failed");
		//exit(1);
	}
	free(item);
	free(ret);
	return NULL;
}
