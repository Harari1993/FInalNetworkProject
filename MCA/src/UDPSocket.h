
#ifndef UDPSOCKET_H_
#define UDPSOCKET_H_

#include <netinet/in.h>
#include <inttypes.h>
#include <strings.h>
#include <stdio.h>
#include <iostream>

using namespace std;

class UDPSocket {
public:
	UDPSocket(int port = 9999);
	virtual ~UDPSocket() {}

	int recv(char* buffer, int length);
	int sendTo(string msg, string ip, int port);
	int reply(string msg);
	void cclose();
	string fromAddr();

private:
	struct sockaddr_in  s_in;
	struct sockaddr_in from;
	unsigned int fsize;
	int socket_fd;
};

#endif /* UDPSOCKET_H_ */
