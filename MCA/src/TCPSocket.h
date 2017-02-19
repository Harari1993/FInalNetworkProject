
#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

class TCPSocket {
public:
	TCPSocket(int port);
	TCPSocket(string peerIp, int port);
	virtual ~TCPSocket() {}

	TCPSocket* listenAndAccept();
	int recv(char* buffer, int length);
	int send(const char* msg, int len);
	void cclose();
	string fromAddr();
	string destIpAndPort();
	int getSocketFid() { return socket_fd; }

private:
	TCPSocket(int connected_sock,struct sockaddr_in serverAddr,struct sockaddr_in peerAddr);

	struct sockaddr_in serverAddr;
	struct sockaddr_in peerAddr;
	int socket_fd;
};

#endif /* TCPSOCKET_H_ */
