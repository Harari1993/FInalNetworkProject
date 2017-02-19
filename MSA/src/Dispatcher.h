/*
 * Dispatcher.h
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <string>
#include <stdio.h>
#include "MThread.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocol.h"

class TCPMessengerServer;

class Dispatcher : public MThread {
public:
	TCPMessengerServer* mainServer;
	MultipleTCPSocketsListener* MTL;
	bool isON;
	void run();
	Dispatcher(TCPMessengerServer* tcpMS);
	virtual ~Dispatcher();
};

#endif /* DISPATCHER_H_ */
