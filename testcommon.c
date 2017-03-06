#include"common.h"
#include<stdio.h>

int main(){
	printf("\n\nThis is testcommon.c Testing program\n\n");

	char *ret;
	ret = malloc(sizeof(char)*1000);
	/*Testing encode*/
	if(encode("m", "0", "testing message0", ret)==0)
		printf("Encode SUCCESS\n");
	else
		printf("Encode FAIL\n");

	/*Testing decode*/
	char *type = malloc(sizeof(char)*51);
	char *seqNo = malloc(sizeof(char)*10);
	char *message = malloc(sizeof(char)*10);
	if(decode(type, seqNo, message, ret) == 0)
		printf("Decode SUCCESS\n");
	else
		printf("Decode FAIL\n");
	printf("Encode value: m 0 testing message0\n");
	printf("Decode value: %s %s %s\n", type, seqNo, message);
	free(type);
	free(seqNo);
	free(message);
}
