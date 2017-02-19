/*
 * TCPSocket.cpp
 *
 *  Created on: Feb 15, 2016
 *      Author: colman
 */

#include "TCPSocket.h"
#include <iostream>
#include <sys/types.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

/* private constructor to create a secondary server socket to
   communicate with a remote peer */
TCPSocket::TCPSocket(int connected_sock,struct sockaddr_in serverAddr,struct sockaddr_in peerAddr){
	this->serverAddr =  serverAddr;
	this->peerAddr =  peerAddr;
	socket_fd = connected_sock;
}

/* Constructor create a TCP server socket */
TCPSocket::TCPSocket(int port){
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	// clear the s_in struct
	bzero((char *) &serverAddr, sizeof(serverAddr));

	//sets the sin address
	serverAddr.sin_family = (short)AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);    /* WILDCARD */
	serverAddr.sin_port = htons((u_short)port);

	//bind the socket on the specified address
	cout << "TCP server binding..." << endl;
	if (bind(socket_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror ("Error naming channel");
	} else {
		cout << "successful binding" << endl;
	}
}

/* Constructor creates TCP client socket */
TCPSocket::TCPSocket(string peerIp, int port) {
	cout<<"opening new client socket"<<endl;

	/*
	 * int socket(int domain, int type, int protocol);
	 * creates a TCP socket
	 * AF_INET - IPv4 Internet protocols
	 * SOCK_STREAM - TCP
	 * 0 - default protocol type
	 */
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	// clear the s_in struct
	bzero((char *) &peerAddr, sizeof(peerAddr));

	//sets the sin address
	peerAddr.sin_family = (short)AF_INET;
	peerAddr.sin_addr.s_addr = inet_addr(peerIp.data());
	peerAddr.sin_port = htons((u_short)port);

	if (connect(socket_fd, (struct sockaddr *)&peerAddr, sizeof(peerAddr)) < 0) {
		perror ("Error establishing communications");
		close(socket_fd);
	}
}

/* Perform listen and accept on server socket */
TCPSocket* TCPSocket::listenAndAccept(){
	int rc = listen(socket_fd, 1);
	if (rc < 0) {
		return NULL;
	}
	size_t len = sizeof(peerAddr);
	bzero((char *) &peerAddr, sizeof(peerAddr));

	int connect_sock = accept(socket_fd, (struct sockaddr *)&peerAddr,(unsigned int *) &len);
	if (connect_sock<0)return NULL;
	return new TCPSocket(connect_sock,serverAddr,peerAddr);
}


/* Read from socket into the given buffer up to the buffer given length.
   return the number of bytes read */
int TCPSocket::recv(char* buffer, int length){
	return read(socket_fd,buffer,length);
}

/* send the given buffer to the socket */
int TCPSocket::send(const char* msg, int len){
	return write(socket_fd,msg,len);
}

/* close the socket and free all resources */
void TCPSocket::cclose(){
	cout<<"closing socket"<<endl;
	shutdown(socket_fd,SHUT_RDWR);
	close(socket_fd);
}

/* return the address of the connected peer */
string TCPSocket::fromAddr(){
	return inet_ntoa(peerAddr.sin_addr);
}

string TCPSocket::destIpAndPort(){
	string str = inet_ntoa(peerAddr.sin_addr);
	str.append(":");
	char buff[10];
	sprintf(buff,"%d",ntohs(peerAddr.sin_port));
	str.append(buff);
	return str;
}

