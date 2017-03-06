/*
 * Angus Jo Kwang
 * anj218
 * 11136793
 */

#include<pthread.h>
#include"queue.h"
#include"common.h"

volatile sig_atomic_t running = 1;

void sig_handler(int signum){
	printf("\n");
	running = 0;
}

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


main(int argc, char **argv){
	pthread_t threadID;
	struct sigaction a;
	a.sa_handler = sig_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGINT, &a, NULL);

	int servSock;
	unsigned short port, clntport;
	int toPort;
	struct sockaddr_in clntaddr;

	int maxSeqNo, windowSize, windowBegin, currentSeqNo, lastSeqNo, block;
	char *recvMsg = malloc(sizeof(char)*100);
	char *type = malloc(sizeof(char));
	char *seqNo = malloc(sizeof(char)*3);
	char *missingSeq = malloc(sizeof(char)*3);
	char *msg = malloc(sizeof(char)*100);
	char *replyMsg = malloc(sizeof(char)*100);
	struct FRAME **frames;
	struct FRAME *currentFrame;

	int statsend = 0;
	int statrecv = 0;
	int statack = 0;
	int statnak = 0;

	if(argc != 4){
		fprintf(stderr, "Usage: testserver <server port> <client port> <max sequence number>\n");
		exit(1);
	}

	port = atoi(argv[1]);
	clntport = atoi(argv[2]);
	maxSeqNo = atoi(argv[3]);
	windowSize = (maxSeqNo + 1) / 2;
	windowBegin = 0;
	printf("Window size: %d\n", windowSize);
	frames = (FRAME **)malloc(sizeof(FRAME *) * windowSize);
	int i;
	for(i = 0; i < windowSize; i++){
		frames[i] = malloc(sizeof(FRAME));
		frames[i]->data = malloc(sizeof(char)*100);
	}
	FrameInit(frames, windowSize, maxSeqNo, windowBegin);
	currentFrame = frames[0];

	servSock = CreateUDPSocket(port);

	printf("testserver running\n");

	currentSeqNo = -1;
	lastSeqNo = -1;
	block = 0;
	int framecount = 0;
	int frameNo;

	while(running){
		if(isFrameAllAck(frames, windowSize)){
			windowBegin = (windowBegin + windowSize) % (maxSeqNo + 1);
			FrameInit(frames, windowSize, maxSeqNo, windowBegin);
			framecount = 0;
			//printf("WINDOW SLIDED!!!\n");
		}
		frameNo = -1;
		recvMsg[0] = '\0';
		type[0] = '\0';
		seqNo[0] = '\0';
		msg[0] = '\0';
		toPort = recvUDP(servSock, recvMsg, clntaddr);
		if(toPort == 0)
			break;
		statrecv++;
		clntaddr.sin_family = AF_INET;
		clntaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		clntaddr.sin_port = htons(clntport);
		decode(type, seqNo, msg, recvMsg);
		currentSeqNo = atoi(seqNo);
		if(inWindow(maxSeqNo, windowSize, windowBegin, currentSeqNo)){
			for(i = 0; i < windowSize; i++){
				if(FrameGetSeq(frames[i]) == currentSeqNo){
					frameNo = i;
					FrameSet(frames[i], 1, msg);
				}
			}
			if(frameNo == -1){
				fprintf(stderr, "frame not found in window\n");
				fprintf(stderr, "Seq Recved: %s", seqNo);
				exit(1);
			}
			
			//printf("Message received - frameNo: %d seqNo: %d\n", frameNo, currentSeqNo);
			//printf("type: %s seqNo: %d msg: %s\n", type, currentSeqNo, msg);

			if(frameNo - framecount <= 0){
				if(frameNo < framecount || !block){
					//send ack
					encode("a", seqNo, "", replyMsg);
					statsend++;
					statack++;
					if(sendto(servSock, replyMsg, strlen(replyMsg), 0, (struct sockaddr *)&clntaddr, sizeof(clntaddr)) < 0){
						fprintf(stderr, "server send ack failed: %s\n", strerror(errno));
						exit(1);
					}
					//else
						//printf("Ack[%s] sent to client: %d\n", seqNo, ntohs(clntaddr.sin_port));
					if(frameNo == framecount){
						printf("TO NETWORK LAYER: %s\n", frames[framecount]->data);
						framecount++;
						lastSeqNo = currentSeqNo;
					}
				}
				else if(block && frameNo == framecount){
					printf("TO NETWORK LAYER: %s\n", frames[framecount]->data);
					lastSeqNo = frames[framecount]->seqNo;
					framecount++;
					while(framecount < windowSize && frames[framecount]->inUse){
						printf("TO NETWORK LAYER: %s\n", frames[framecount]->data);
						lastSeqNo = frames[framecount]->seqNo;
						framecount++;
					}
					if(framecount == windowSize){
						//send ack
						statsend++;
						statack++;
						sprintf(seqNo, "%d", lastSeqNo);
						encode("a", seqNo, "", replyMsg);
						if(sendto(servSock, replyMsg, strlen(replyMsg), 0, (struct sockaddr *)&clntaddr, sizeof(clntaddr)) < 0){
							fprintf(stderr, "server send ack failed: %s\n", strerror(errno));
							exit(1);
						}
						//else
							//printf("Ack[%s] sent to client: %d\n", seqNo, ntohs(clntaddr.sin_port));
						block = 0;
					}else{
						//send nak
						statsend++;
						statnak++;
						if(lastSeqNo < maxSeqNo)
							sprintf(missingSeq, "%d", lastSeqNo+1);
						else
							sprintf(missingSeq, "0");
						encode("n", missingSeq, "", replyMsg);
						if(sendto(servSock, replyMsg, strlen(replyMsg), 0, (struct sockaddr *)&clntaddr, sizeof(clntaddr)) < 0){
						fprintf(stderr, "server send ack failed: %s\n", strerror(errno));
							exit(1);
						}
						//else
							//printf("Nck[%s] sent to client: %d\n",missingSeq, ntohs(clntaddr.sin_port));
					}
					
				}
			}
			else if(!block){
				block = 1;
				if(frameNo > framecount){
					//send nak
					statnak++;
					statsend++;
					if(lastSeqNo < maxSeqNo)
						sprintf(missingSeq, "%d", lastSeqNo+1);
					else
						sprintf(missingSeq, "0");
					encode("n", missingSeq, "", replyMsg);
					if(sendto(servSock, replyMsg, strlen(replyMsg), 0, (struct sockaddr *)&clntaddr, sizeof(clntaddr)) < 0){
						fprintf(stderr, "server send ack failed: %s\n", strerror(errno));
						exit(1);
					}
					//else
						//printf("Nck[%s] sent to client: %d\n", missingSeq, ntohs(clntaddr.sin_port));
				}
			}
		}
		else{
			encode("a", seqNo, "", replyMsg);
			//send ack
			statsend++;
			statack++;
			if(sendto(servSock, replyMsg, strlen(replyMsg), 0, (struct sockaddr *)&clntaddr, sizeof(clntaddr)) < 0){
				fprintf(stderr, "server send ack failed: %s\n", strerror(errno));
				exit(1);
			}
			//else
				//printf("Ack sent to client: %d\n", ntohs(clntaddr.sin_port));
		}
	}

	printf("Shutting down testserver\n");

	printf("Statistics:\n");
	printf("Total receives: %d\n", statrecv);
	printf("Total sends: %d\n", statsend);
	printf("Total ack sent: %d\n", statack);
	printf("Total nak sent: %d\n", statnak);

	for(i = 0; i < windowSize; i++){
		free(frames[i]->data);
		free(frames[i]);
	}
	free(frames);
	free(recvMsg);
	free(type);
	free(seqNo);
	free(msg);
	free(replyMsg);
	close(servSock);
	exit(0);
}
