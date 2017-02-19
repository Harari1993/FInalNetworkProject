/*
 * TCPMessengerServer.h
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#ifndef TCPMESSENGERSERVER_H_
#define TCPMESSENGERSERVER_H_

#include <vector>
#include "MThread.h"
#include "TCPSocket.h"
#include "MultipleTCPSocketsListener.h"
#include "LoginManager.h"
#include "TCPMessengerProtocol.h"
#include "Dispatcher.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "Room.h"

using namespace std;

class TCPMessengerServer:public MThread{

public:
	int posInVec;
	Dispatcher* myDis;
	LoginManager* myLoginManager;
	TCPSocket* serverSock;
	vector<TCPSocket*> openPeerVect;
	vector<TCPSocket*> connectedSocketsVect;
	vector<string> ipTOclientName;
	vector<string> initiatorSession;
	vector<string> wantedSession;
	vector<Room*> Rooms;


	TCPMessengerServer();
	~TCPMessengerServer() {}
	void run();
	void getListOfPeers();
	void printOpenPeerVector();

	int recieveCommandFromTCP(TCPSocket * tmpTCP);
	char * recieveMessageFromTCP(TCPSocket * tmpTCP);
	void sendCommandToTCP(int protocol,TCPSocket * tmpTCP);
	void sendMsgToTCP(string msg, TCPSocket* tmpTCP);
	void insertToOpenVector(TCPSocket* temp_soc);
	int findVector(vector<TCPSocket*> &vec, string address);
	void getListOfConnedtedUsers(TCPSocket * tmpTCP);
	string nameToIp(string name);
	string ipToName(string ip);
	void getListOfSessions();
	int findInRooms(string roomName);
	void printUsersinRoom(string name);
	string transferUsersInRoomToString(string roomName);
	void sendMsgToAllUsersInRoom(int msgType,string roomName, string userName);
	vector<string> getUserNamesFromData();
	void printAllUsers();
	void close();

};

#endif /* TCPMESSENGERSERVER_H_ */
