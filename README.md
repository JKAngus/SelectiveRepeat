# SelectiveRepeat

This is a selective repeat server created by Angus Jo Kwang for University of Saskatchewan course CMPT 434

A UDP-emulator is used to simulate an unstable connection.  A server and client communicates through the emulator and uses selective repeat to ensure every packet is sent.

See Design.txt for detailed description.

To compile:
make

To run UDP-emulator:
Usage: UDP-emulator <p> <d> <q> <port A> <port B> <port C>

  p - drop probability
  d - delay value
  q - maximum queue size
  port A - the port that the emulator listens
  port B - the port that the emulator receives from and sends to port C
  port C - the port that the emulator receives from and sends to port B


To run testserver:
Usage: testserver <server port> <client port> <max sequence number>
  server port - the port that this server listens
  client port - the port that this server replies to
  max sequence number - should be same with client

To run testclient:
Usage: testclient <client port> <server port> <max sequence number> <number of packets> <timeout in ms>
  client port - the port that this client listens
  server port - the port that this client sends to
  max sequence number - should be same with server
  number of packets - number of packets it will generate and send to server
  timeout in ms - timeout to re-sent frame in milli-seconds
