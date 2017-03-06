/*
 * Angus Jo Kwang
 * 11136793
 * anj218
 */

#include"common.h"
#define STDIN 0

volatile sig_atomic_t running = 1;

void sig_handler(int signum){
	printf("\n");
	running = 0;
}

int main(int argc, char **argv){
	struct sigaction a;
	a.sa_handler = sig_handler;
	a.sa_flags = 0;
	sigemptyset(&a.sa_mask);
	sigaction(SIGINT, &a, NULL);
	
	struct sockaddr_in servaddr;
	struct sockaddr_in clntaddr;
	int sock, servport, clntport, packetsNo, timeout;
	size_t size;

	int maxSeqNo, missingSeq, windowSize, windowBegin;
	char *encodedMsg, *currentSeqChar;

	encodedMsg = malloc(sizeof(char)*100);
	currentSeqChar = malloc(sizeof(char)*3);

	int packetCount = 0;
	char **packets;
	char *recvMsg = malloc(sizeof(char)*100);
	char *type = malloc(sizeof(char));
	char *seqNo = malloc(sizeof(char)*3);
	char *msg = malloc(sizeof(char)*100);

	int statsend = 0;
	int statrecv = 0;
	int statack = 0;
	int statnak = 0;
	int stattimeout = 0;

	if(argc == 6){
		clntport = atoi(argv[1]);
		servport = atoi(argv[2]);
		maxSeqNo = atoi(argv[3]);
		windowSize = (maxSeqNo + 1) / 2;
		windowBegin = 0;
		printf("Window size: %d\n", windowSize);
		packetsNo = atoi(argv[4]);
		timeout = atoi(argv[5]);
	}
	else{
		fprintf(stderr, "Usage: testclient <client port> <server port> <max sequence number> <number of packets> <timeout in ms>\n");
		exit(1);
	}

	int i;
	FRAME **frames;
	FRAME *currentFrame;
	frames = (FRAME **)malloc(sizeof(FRAME *) * windowSize);
	for(i = 0; i< windowSize; i++){
		frames[i] = malloc(sizeof(FRAME));
		frames[i]->data = malloc(sizeof(char)*100);
	}
	FrameInit(frames, windowSize, maxSeqNo, windowBegin);

	packets = malloc(sizeof(char *)*packetsNo);
	for(i = 0; i < packetsNo; i++){
		packets[i] = malloc(sizeof(char)*10);
		sprintf(packets[i], "packet %d", i);
	}

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(servport);

	memset(&clntaddr, 0, sizeof(clntaddr));
	clntaddr.sin_family = AF_INET;
	clntaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clntaddr.sin_port = htons(clntport);

	if(bind(sock, (struct sockaddr *)&clntaddr, sizeof(clntaddr)) < 0){
		fprintf(stderr, "bind failed\n");
		exit(1);
	}

	struct timeval tv;
	fd_set readfds;

	printf("testclient running\n");

	while(running){
		if(isFrameAllAck(frames, windowSize)){
			windowBegin = (windowBegin + windowSize) % (maxSeqNo + 1);
			FrameInit(frames, windowSize, maxSeqNo, windowBegin);
			//printf("WINDOW SLIDED!!!\n");
		}	
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_SET(STDIN, &readfds);
		FD_SET(sock, &readfds);

		if(select(sock + 1, &readfds, NULL, NULL, &tv) != 0){
			if (FD_ISSET(STDIN, &readfds)){
				break;
			}
			if (FD_ISSET(sock, &readfds)){
				//recv
				//check if nak
				//resend nak
				//printf("Reply received\n");
				recvMsg[0] = '\0';
				type[0] = '\0';
				seqNo[0] = '\0';
				if(recvUDP(sock, recvMsg, clntaddr) != 0){
					statrecv++;
					decode(type, seqNo, msg, recvMsg);
					if(strcmp(type, "a") == 0){
						statack++;
						printf("ack received: %s\n",seqNo);
						if(inWindow(maxSeqNo, windowSize, windowBegin, atoi(seqNo)))
							for(i = 0; i < windowSize; i++){
								if(!frames[i]->ack){
									currentFrame = frames[i];
									printf("frame[%d] ACK\n", frames[i]->seqNo);
									FrameAck(currentFrame);
								}
								if(FrameGetSeq(frames[i]) == atoi(seqNo)){
									i = windowSize;
								}
							}
					}
					else if(strcmp(type, "n") == 0){
						statnak++;
						printf("nak received: %s\n", seqNo);
						for(i = 0; i < windowSize; i++){
							if(FrameGetSeq(frames[i]) == atoi(seqNo)){
								int j;
								for(j = 0; j < i; j++){
									if(!frames[j]->ack){
										FrameAck(frames[j]);
										printf("frame[%d] ACK\n", frames[j]->seqNo);
									}
								}
								currentFrame = frames[i];
								i = windowSize;
								//printf("frame[%d] NAK\n", frames[i]->seqNo);
							}
						}

						//printf("RESENDING NAK seqtofind: %s seq found: %d\n", seqNo, currentFrame->seqNo);

						if (sendto(sock, currentFrame->data, strlen(currentFrame->data), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
							perror("sendto failed\n");
							exit(1);
						}
						statsend++;
						printf("Nak resent to port: %i\n", htons(servaddr.sin_port));
					}
				}
				else
					break;
			}
		}
		//send new packets when frames available
		if(packetCount < packetsNo &&
				nextFrame(frames, windowSize, &currentFrame) == 1){
			sprintf(currentSeqChar, "%d", FrameGetSeq(currentFrame));
			encodedMsg[0] = '\0';
			encode("d", currentSeqChar, packets[packetCount], encodedMsg);
			FrameSet(currentFrame, 0, encodedMsg);
			if (sendto(sock, encodedMsg, strlen(encodedMsg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
				perror("sendto failed\n");
				exit(1);
			}
			statsend++;
			printf("Frame[%d] sent to port: %i\n", FrameGetSeq(currentFrame), htons(servaddr.sin_port));
			packetCount ++;	
		}
		//Check frames timeout
		for(i = 0; i < windowSize; i++){
			if(frames[i]->inUse &&
					!frames[i]->ack &&
					isFrameTimeOut(frames[i], timeout)){
				stattimeout++;
				statsend++;
				printf("Resend timeout frames[%d]\n", frames[i]->seqNo);
				if (sendto(sock, frames[i]->data, strlen(frames[i]->data), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
					perror("sendto failed\n");
					exit(1);
				}
				FrameResetTimeout(frames[i]);
			}
		}
		//check if last packet
		if(packetCount >= packetsNo && isFrameUsedAck(frames, windowSize))
			break;
	}
	printf("Shutting down testclient\n");

	printf("Statistics:\n");
	printf("Total sends: %d\n", statsend);
	printf("Total receives: %d\n", statrecv);
	printf("Total ack: %d\n", statack);
	printf("Total nak: %d\n", statnak);
	printf("Total timeouts: %d\n", stattimeout);

	for(i = 0; i < packetsNo; i++){
		free(packets[i]);
	}
	for(i = 0; i < windowSize; i++){
		free(frames[i]->data);
		free(frames[i]);
	}
	free(packets);
	free(frames);

	free(encodedMsg);
	free(currentSeqChar);
	free(recvMsg);
	free(type);
	free(seqNo);
	free(msg);
	close(sock);
	return 0;
}
