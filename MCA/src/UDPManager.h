
#ifndef UDPMANAGER_H_
#define UDPMANAGER_H_

#include "UDPSocket.h"
#include "MThread.h"
#include <string.h>
#include <vector>
#include <stdlib.h>

class UDPManager: public MThread {
public:
	UDPManager(string myUserName, string myIpandPort);
	virtual ~UDPManager();

	void run();
	void sendToPeer(string msg);
	void setDestmessage(string dest);
	void sendToRoom(string msg);
	void PrintUsersInRoom();

	vector<string> listOfUsersInRoom;
	bool UDPserverConnected;

private:
	UDPSocket* clientUDPSock;
	string myUserName;
	string destIp;
	string destPort;
	string roomName;
};

#endif /* UDPMANAGER_H_ */
