/*
 * LoginManager.h
 *
 *  Created on: Feb 17, 2016
 *      Author: colman
 */

#ifndef LOGINMANAGER_H_
#define LOGINMANAGER_H_

#include <string>
#include <stdio.h>
#include "MThread.h"

using namespace std;

class TCPMessengerServer;
class MultipleTCPSocketsListener;

class LoginManager: public MThread {
public:
	LoginManager(TCPMessengerServer* mainServer);
	virtual ~LoginManager();
	void run();

	bool isON;
	MultipleTCPSocketsListener* MTL;

private:
	int Register(string userNamePlusPassword);
	int Login(string userName, string password);

	TCPMessengerServer* mainServer;
};

#endif /* LOGINMANAGER_H_ */
