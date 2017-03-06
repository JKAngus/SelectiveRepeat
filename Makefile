#Angus Jo Kwang
#anj218
#11136793

cc = gcc -c -g

All: Libraries UDP-emulator testclient testserver Tests

#########LIBRARIES#############

Libraries:	libqueue.a libcommon.a

libqueue.a:	queue.o
	ar rvs libqueue.a queue.o

queue.o:	queue.c
	$(cc) queue.c

libcommon.a:	common.o
	ar rvs libcommon.a common.o

common.o:	common.c
	$(cc) common.c

#########UDP EMULATOR##########

testclient:	testclient.o libcommon.a
	gcc -Wall -pedantic -g testclient.o -L./ -lcommon -o testclient

testclient.o:	testclient.c
	$(cc) testclient.c

testserver:	testserver.o libcommon.a libqueue.a
	gcc -Wall -pedantic -g testserver.o -L./ -lcommon -lqueue -o testserver

testserver.o:	testserver.c
	$(cc) testserver.c

UDP-emulator:	UDP-emulator.o libqueue.a
	gcc -pthread -Wall -pedantic -g UDP-emulator.o -L./ -lqueue -o UDP-emulator

UDP-emulator.o:	UDP-emulator.c
	$(cc) UDP-emulator.c
	
#########TESTS#############

Tests:	testqueue testcommon

testqueue:	testqueue.o libqueue.a
	gcc -Wall -pedantic -g testqueue.o -L./ -lqueue -o testqueue

testqueue.o:	testqueue.c
	$(cc) testqueue.c

testcommon:	testcommon.o libcommon.a
	gcc -Wall -pedantic -g testcommon.o -L./ -lcommon -o testcommon

testcommon.o:	testcommon.c
	$(cc) testcommon.c

##########Clean##########

clean:
	rm -rf *.o *.a *~ UDP-emulator testqueue testcommon testclient testserver
