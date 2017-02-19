/*
 * MultipleTCPSocketsListener.h
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#ifndef MULTIPLETCPSOCKETSLISTENER_H_
#define MULTIPLETCPSOCKETSLISTENER_H_

#include <iostream>
#include <stdio.h>
#include <vector>
#include "TCPSocket.h"

using namespace std;


class MultipleTCPSocketsListener {
	typedef vector<TCPSocket*> tSocketsContainer;
	tSocketsContainer sockets;

public:
	void addSocket(TCPSocket* socket);
	void addSockets(vector<TCPSocket*> socketVec);
	TCPSocket* listenToSocket(int timeout=0);
};

#endif /* MULTIPLETCPSOCKETSLISTENER_H_ */
