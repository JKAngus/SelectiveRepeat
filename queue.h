/*
 * Angus Jo Kwang
 * anj218
 * 11136793
 */

#include<string.h>

typedef struct NODE{
	char item[100];
	struct NODE *next;
} NODE;

typedef struct QUEUE{
	struct NODE *head;
	struct NODE *tail;
	int size;
	int maxSize;
} QUEUE;

int QueueInit(QUEUE *queue, int maxSize);
int push(QUEUE *queue, char *item);
int pop(QUEUE *queue, char *ret);
int insertFirst(QUEUE *queue, char *item);
