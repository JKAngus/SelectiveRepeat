/*
 * Angus Jo Kwang
 * anj218
 * 11136793
 */
#include"queue.h"
#include<stdio.h>
#include<stdlib.h>

int main(){
	printf("\n\nThis is testqueue.c Testing program\n\n");

	char *ret;
	ret = malloc(sizeof(char)*100);

	QUEUE *myQ = (QUEUE *)malloc(sizeof(QUEUE));
	NODE *itr;

	printf("Testing Queue init\n");
	if(QueueInit(myQ, 10) == 1)
		printf("Queue init SUCCESS\n");
	else
		printf("Queue init FAILED\n");

	printf("\nTesting Queue push\n");

	printf("Push item1\n");
	char *item1 = malloc(100);
	strcpy(item1, "item1");
	if(push(myQ, item1) == 1)
		printf("Queue push SUCCESS\n");
	else
		printf("Queue push FAILED\n");
	
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	printf("\n\nPush item2\n");
	char *item2 = malloc(100);
	strcpy(item2, "item2");
	if(push(myQ, item2) == 1)
		printf("Queue push SUCCESS\n");
	else
		printf("Queue push FAILED\n");
	
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	printf("\n\nPush item3\n");
	char *item3 = malloc(100);
	strcpy(item3, "item3");
	if(push(myQ, item3) == 1)
		printf("Queue push SUCCESS\n");
	else
		printf("Queue push FAILED\n");
	
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	printf("\n\nTesting queue pop\n");

	printf("1st POP\n");
	if(pop(myQ, ret) == 1)
		printf("Queue pop SUCCESS\n");
	else
		printf("Queue pop FAILED\n");

	printf("Item popped: %s\n", ret);
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	printf("\n\n2nd POP\n");
	ret[0] = '\0';
	if(pop(myQ, ret) == 1)
		printf("Queue pop SUCCESS\n");
	else
		printf("Queue pop FAILED\n");

	printf("Item popped: %s\n", ret);
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	printf("\n\nPush item4\n");
	char *item4 = malloc(100);
	strcpy(item4, "item4");
	if(push(myQ, item4) == 1)
		printf("Queue push SUCCESS\n");
	else
		printf("Queue push FAILED\n");
	
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}


	printf("\n\n3rd POP\n");
	ret[0] = '\0';
	if(pop(myQ, ret) == 1)
		printf("Queue pop SUCCESS\n");
	else
		printf("Queue pop FAILED\n");

	printf("Item popped: %s\n", ret);
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	printf("\n\n4th POP\n");
	ret[0] = '\0';
	if(pop(myQ, ret) == 1)
		printf("Queue pop SUCCESS\n");
	else
		printf("Queue pop FAILED\n");

	printf("Item popped: %s\n", ret);
	printf("Item in queue:\n");
	itr = myQ->head;
	while(itr!=NULL){
		printf("%s ", itr->item);
		itr = itr->next;
	}

	free(item1);
	free(item2);
	free(item3);
	free(item4);
	free(ret);
	free(myQ);
	printf("\n\nTesting Complete\n");
}

