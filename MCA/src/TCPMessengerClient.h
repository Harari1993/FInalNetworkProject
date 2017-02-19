
#ifndef TCPMESSENGERCLIENT_H_
#define TCPMESSENGERCLIENT_H_

#include <strings.h>
#include <map>
#include <vector>
#include "MThread.h"
#include "TCPSocket.h"
#include "UDPManager.h"
#include "TCPMessengerProtocol.h"

using namespace std;

class TCPMessengerClient: public MThread {
public:
	TCPMessengerClient();
	~TCPMessengerClient();

	void run();
	bool connect(string ip);
	bool loginUser(string user,string pass);
	bool registerUser(string user,string pass);
	void sendPeerAdrrToServer(string userName, string password);//will send the peer's address
	void TCPtoServerMessage(string msg,int protocol);
	bool isConnected();
	bool disconnect();
	bool open(string typeOfSession,string name);
	bool closeActiveSession();
	bool send(string msg);
	void TCPtoServerCommandProtocol(int protocol);
	void createNewRoom(string roomName);
	void LeaveCurrentRoom();
	void printMyCurrentStatus();
	void closeRoom(string roomName);
	void printData(string data, int numOfIter);
	void printAllUsersInRoomToServer(string roomName);
	void printAllRooms();
	void printConnectedUsers();
	void printAllUsers();

	string roomName;
	int state;

private:
	TCPSocket* clientSock;
	UDPManager* UDPmanager;
	bool serverConnected;
	string inSessionWith;
	string userName;

};


#endif /* TCPMESSENGERCLIENT_H_ */
