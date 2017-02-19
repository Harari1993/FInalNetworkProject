/*
 * UDPManager.cpp
 *
 *  Created on: Feb 15, 2016
 *      Author: colman
 */

#include "UDPManager.h"

/*
 * Constructor, recieves the required information from the server
 */
UDPManager::UDPManager(string myUserName,string myIpandPort) {
	this->myUserName = myUserName;
	char* port = strdup(myIpandPort.c_str());
	string tempPort = strtok(port,":");
	tempPort = strtok(NULL,":");
	clientUDPSock = new UDPSocket(atoi(tempPort.c_str()));
	UDPserverConnected=true;

}
/*
 * Set the destination
 */
void UDPManager::setDestmessage(string dest) {
	char* port = strdup(dest.c_str());
	destIp = strtok(port,":");
	destPort = strtok(NULL,":");
}

/*
 * Function that sends a message too all of the rooms
 */
void UDPManager::sendToRoom(string msg) {
	//Looping on all users in the my room
	for (unsigned int i=0; i < listOfUsersInRoom.size(); i++) {
		//Gets IP:Port for each user from listOfUsersInRoom
		string tempdest = listOfUsersInRoom.at(i);
		//Initializes the destIp & destPort data members
		setDestmessage(tempdest);
		//Message to user (UDP)
		this->sendToPeer(msg);
	}
}
/*
 * Sending a message to a peer
 */
void UDPManager::sendToPeer(string msg) {
	 string finalmsg = "["+myUserName+"]"+" " + msg;
	 clientUDPSock->sendTo(finalmsg,destIp,atoi(destPort.c_str()));
}

UDPManager::~UDPManager() {
	delete clientUDPSock;
}

/*
 * The main loop of the UDp, recieves and prints incoming UDP messages
 */
void UDPManager::run(){

	char buffer[1024];

	while (UDPserverConnected) {
		  clientUDPSock->recv(buffer,sizeof(buffer));
		  //print the msg
		  cout << buffer << endl;
		  bzero((char *) &buffer, sizeof(buffer));
	}
	clientUDPSock->cclose();
}
