
#include "TCPMessengerClient.h"
#include "TCPMessengerProtocol.h"
#include "UDPSocket.h"
#include <string.h>

/* Basic constructor that initializes all the data */
TCPMessengerClient::TCPMessengerClient(){
	serverConnected = false;
	clientSock = NULL;
	UDPmanager = NULL;
	this->roomName = "none";
	this->userName = "none";
	this->state = NOT_CONNECTED;
}

TCPMessengerClient::~TCPMessengerClient() {
	if (UDPmanager != NULL) {
		delete UDPmanager;
		UDPmanager = NULL;
	}

	if (clientSock != NULL) {
		clientSock->cclose();
		delete clientSock;
		clientSock = NULL;
	}
}

/* connect to the given server ip
   (the port is defined in the protocol header file) */
bool TCPMessengerClient::connect(string ip) {
	//First we check if the user is already connected
	if (state == NOT_CONNECTED) {
		clientSock = new TCPSocket(ip,MSNGR_PORT);
		start();
		serverConnected = true;
		state = CONNECTED_STATE;
		return true;
	} else {
		puts("Connection already opened");
		return false;
	}
}

bool TCPMessengerClient::loginUser(string user,string pass) {
	if (state == CONNECTED_STATE) {
		string msg = user + " " + pass;
		this->TCPtoServerMessage(msg,USER_LOGIN_REQUEST);
		//Sets this client as the userName
		userName=user;
		return true;
	} else if (state == NOT_CONNECTED) {
		puts("You are not connected to any server");
		return false;
	} else {
		puts("You must disconnect from server first");
		return false;
	}
}

bool TCPMessengerClient::registerUser(string user,string pass) {
	if (state == CONNECTED_STATE) {
		string msg = user + " " + pass;
		this->TCPtoServerMessage(msg,CREATE_NEW_USER);
		//Sets this client as the userName
		userName=user;
		return true;
	} else if (state == NOT_CONNECTED) {
		puts("You are not connected to any server");
		return false;
	} else {
		puts("You must disconnect from server first");
		return false;
	}
}


bool TCPMessengerClient::isConnected(){
	return serverConnected;
}

/* disconnect from messenger server  */
bool TCPMessengerClient::disconnect(){

	if(state!=NOT_CONNECTED)
	{
		if(state==IN_ROOM) {
			this->LeaveCurrentRoom();
		}
		if(state==IN_SESSION) {
			this->closeActiveSession();
		}

		this->TCPtoServerCommandProtocol(DISCONNECT);
		system("sleep 1");
		serverConnected=false;
		if (UDPmanager != NULL) {
			this->UDPmanager->UDPserverConnected=false;
		}

		clientSock->cclose();
		delete clientSock;
		clientSock = NULL;
		this->state=NOT_CONNECTED;
		return true;
	}
	else
	{
		puts("You are not connected to the server");
		return true;
	}
	return false;
}

/* open session with the given peer address (ip:port) */
bool TCPMessengerClient::open(string typeOfSession,string name) {

	//First we check if you connected to the server
	if (isConnected()) {
		//Then we check if you are not already in a session
		if(state != CONNECTED_STATE) {
			if(strcmp(typeOfSession.c_str(),"user")==0) {
				if (state == IN_SESSION) {
					this->closeActiveSession();
				}
				else if (state == IN_ROOM) {
					this->LeaveCurrentRoom();
				}
				this->TCPtoServerMessage(name,OPEN_SESSION_WITH_PEER);
			} else if(strcmp(typeOfSession.c_str(),"room")==0) {
				if (state == IN_SESSION) {
					this->closeActiveSession();
				}
				else if (state == IN_ROOM) {
					this->LeaveCurrentRoom();
				}
				this->TCPtoServerMessage(name,JOIN_ROOM);
			} else {
				puts("Invalid input");
				return false;
			}
			return true;
		} else {
			puts("You are not logged in to the chat");
			return false;
		}
	} else {
		puts("You are not connected to server");
		return false;
	}
}

/*
 * Main loop of the client (for communicating with the server)
 */
void TCPMessengerClient::run()
{
	char* buffer =new char[1024];

	while(serverConnected)
	{
		int command;
		clientSock->recv((char*)&command,4);
		bzero(buffer,1024);
		command = ntohl(command);

		switch(command) {
		case SESSION_ESTABLISHED:
		{
			int msgLen;
			clientSock->recv((char*)&msgLen,4);
			msgLen = ntohl(msgLen);
			clientSock->recv(buffer, msgLen);
			cout<<"connected to "<<buffer<<endl;
			string peerNameToConnect = strtok(buffer, " ");
			string peerAddress = strtok(NULL, " ");
			//Opens UDP session with another user
			UDPmanager->setDestmessage(peerAddress);
			inSessionWith =peerNameToConnect;
			state=IN_SESSION;
			break;
		}
		case SESSION_REFUSED:
		{
			puts("Could not Open Session - The user is not found or disconnected");
			break;
		}
		case SESSION_REFUSED_ROOM:
		{
			puts("Could not Open Session - The user is in a room");
			break;
		}
		case SESSION_REFUSED_SESSION:
		{
			puts("Could not Open Session - The user is in session with another user");
			break;
		}
		case OPEN_SESSION_WITH_PEER:
		{
			TCPtoServerCommandProtocol(state);
			break;
		}
		case LOGIN_RESPONSE_ERROR:
		{
			puts("Login error");
			userName="none";
			break;
		}
		case LOGIN_RESPONSE_APPROVE:
		{
			//Server approved login, waiting for UDPManager settings
			int msgLen;
			clientSock->recv((char*)&msgLen,4);
			msgLen = ntohl(msgLen);
			clientSock->recv(buffer, msgLen);

			//Sets UDPManager
			UDPmanager = new UDPManager(userName,buffer);
			//Start listening on UDP
			UDPmanager->start();
			state = LOGGED_IN;
			cout<<"Connected to server as "<<userName<<endl;
			break;
		}
		case SEND_SERVER_USERNAME:
		{
			cout<<"Connected to server as "<<userName<<endl;

			break;

		}
		case CLOSE_SESSION_WITH_PEER:
		{
			puts("Your Session was disconnected");
			state = LOGGED_IN;
			inSessionWith="none";

			break;
		}
		case ROOM_NOT_UNIQUE:
		{
			this->roomName="none";
			puts("Room name already exists! please choose another room name");
			break;
		}
		case CREATE_ROOM_APPROVED:
		{
			this->state= IN_ROOM;
			puts("Room created");
			break;
		}
		case JOIN_ROOM_ARPROVED:
		{
			int msgLen;
			clientSock->recv((char*)&msgLen,4);
			msgLen = ntohl(msgLen);
			clientSock->recv(buffer, msgLen);
			cout<<"You have been joined to room: "<<buffer<<endl;
			this->state = IN_ROOM;
			this->roomName = buffer;
			break;
		}
		case NO_SUCH_ROOM_NAME:
		{
			puts("no room with such name");
			break;
		}
		case ROOM_STATUS_CHANGED: //update room members
		{
			int msgLen2;
			clientSock->recv((char*)&msgLen2,4);
			msgLen2 = ntohl(msgLen2);
			clientSock->recv(buffer, msgLen2);
			cout<<buffer<<endl;

			int numofUsersInString;
			clientSock->recv((char*)&numofUsersInString,4);
			numofUsersInString = ntohl(numofUsersInString);

			bzero(buffer,1024);

			int msgLen;
			clientSock->recv((char*)&msgLen,4);
			msgLen = ntohl(msgLen);
			clientSock->recv(buffer, msgLen);

			this->UDPmanager->listOfUsersInRoom.clear();

			string tempUserInRoom;
			tempUserInRoom = strtok(buffer," ");

			for(int i =0; i<numofUsersInString-1;i++){
				this->UDPmanager->listOfUsersInRoom.push_back(tempUserInRoom);
				tempUserInRoom=strtok(NULL," ");
			}

			this->UDPmanager->listOfUsersInRoom.push_back(tempUserInRoom);

			break;
		}
		case CLOSE_ROOM_DENIED:
		{
			puts("You're not allowed to close this room");
			break;
		}
		case ROOM_CLOSED:
		{
			puts("Your room was closed by its owner");
			//Cleans my saved users list in that room (UDPManager)
			this->UDPmanager->listOfUsersInRoom.clear();
			this->state=LOGGED_IN;
			this->roomName="none";
			break;
		}
		case NO_ROOMS:
		{
			cout<<"No rooms opened on server for now."<<endl;
			break;
		}
		case NO_USERS:
		{
			cout<<"No users have been registered to the server."<<endl;
			break;
		}
		case LEFT_ROOM:
		{
			puts("You have left the room");
			break;
		}
		case PRINT_DATA_FROM_SERVER:
		{
			int numOfIter;
			clientSock->recv((char*)&numOfIter,4);
			numOfIter = ntohl(numOfIter);


			int msgLen;
			clientSock->recv((char*)&msgLen,4);
			msgLen = ntohl(msgLen);
			clientSock->recv(buffer, msgLen);


			this->printData(buffer,numOfIter);
			break;
		}
		case NEW_USER_DENIED:
		{
			puts(" Failed - User name already exists");
			break;
		}
		case NEW_USER_APPROVED: {
			puts("User registered");

		}
		case SERVER_DISCONNECT:
		{
			puts("server closed");
			if(state==IN_ROOM)
			{
				this->LeaveCurrentRoom();
			}
			if(state==IN_SESSION)
			{
				this->closeActiveSession();
			}
			system("sleep 1");
			serverConnected=false;
			this->UDPmanager->UDPserverConnected=false;
			clientSock->cclose();
			this->state=NOT_CONNECTED;

			break;
		}

		}
	}
}

void TCPMessengerClient::printData(string data, int numOfIter){

	char* tempCahrFromData = strdup(data.c_str());
	int i;
	string dataString = strtok(tempCahrFromData," ");
	for(i =0; i<numOfIter-1;i++){
		cout<<i+1<<"."<<dataString<<endl;
		dataString=strtok(NULL," ");
	}
	cout<<i+1<<"."<<dataString<<endl;
	free(tempCahrFromData);
}

/*
 * Sends the request to the server to print all the rooms
 */
void TCPMessengerClient::printAllRooms(){

	this->TCPtoServerCommandProtocol(EXISTED_ROOMS);
}
/*
 * Sends the request to print all the users in a room
 */
void TCPMessengerClient::printAllUsersInRoomToServer(string roomName){
	if(this->state == NOT_CONNECTED)
		cout<<"You must be connected"<<endl;
	else
		this->TCPtoServerMessage(roomName,USERS_IN_ROOM);


}
/*
 * Disconnects the session
 */
bool TCPMessengerClient::closeActiveSession(){
	if(isConnected())
	{
		if(this->state==IN_SESSION)
		{
			this->TCPtoServerCommandProtocol(CLOSE_SESSION_WITH_PEER);
			state= LOGGED_IN;
			inSessionWith="none";
			puts("You have been disconnected from your current session");
			return true;
		}
		else
		{
			puts("You're not in a session");
		}
	}
	return false;
}

/*
 * send the given message to the connected peer (user or room)
 */
bool TCPMessengerClient::send(string msg){
	if(this->state==IN_SESSION)
	{
		//Message to user (UDP)
		UDPmanager->sendToPeer(msg);
		return true;
	}
	if(this->state==IN_ROOM)
	{
		//Message to room (UDP Loop to all users)
		UDPmanager->sendToRoom(msg);
		return true;
	}

	return false;

}
/*
 * Helper function that sends commands to the server
 */
void TCPMessengerClient::TCPtoServerCommandProtocol(int protocol)
{
	int command = htonl(protocol);
	clientSock->send((char*)&command,4);

}
/*
 * Helper function that sends messages to server
 */
void TCPMessengerClient::TCPtoServerMessage(string msg,int protocol){
	if(protocol!=DONT_SEND_COMMAND)
		this->TCPtoServerCommandProtocol(protocol);

	int msglen=htonl(msg.length());
	//Sends the command to the server
	clientSock->send((char*)&msglen,4);
	//Sends the message to the server
	clientSock->send(msg.c_str(),(msg.length()));
}

/*
 * functions that sends a request to create a new roo,
 */
void TCPMessengerClient::createNewRoom(string roomName)
{
	//Sends CREATE_NEW_ROOM command to the main server with the room name
	this->TCPtoServerMessage(roomName,CREATE_NEW_ROOM);
	this->roomName= roomName;

}
/*
 * Helper function to leave the current room
 */
void TCPMessengerClient::LeaveCurrentRoom()
{
	if(state==IN_ROOM)
	{
		this->TCPtoServerMessage(this->roomName,LEAVE_ROOM);
		this->state=LOGGED_IN;
		this->roomName="none";
	}
	else
		puts("You're not in a room");
}
/*
 * Prints the status of the client, connected,in a room, in a session.
 */
void  TCPMessengerClient::printMyCurrentStatus(){
	if(this->state==LOGGED_IN){
		cout<<"You are connected to server as: "<<this->userName<<endl;
	}
	else if(this->state==IN_ROOM){
		cout<<"You are in room "<<this->roomName<<endl;
	}
	else if(this->state==IN_SESSION){
		cout<<"You are in session with: "<<inSessionWith<<endl;
	}
	else
		cout<<"You are currently not connected to any server"<<endl;
}

/*
 * Helper function for closing a room
 */
void TCPMessengerClient::closeRoom(string roomName){
	if(state == IN_ROOM) {
		this->TCPtoServerMessage(roomName,CLOSE_ROOM_REQUEST);
	}
}

/*
 * Function that helps send the request to print all of the connected users
 */
void TCPMessengerClient::printConnectedUsers()
{
	//print all connected users ONLY when connected
	if(this->state == NOT_CONNECTED)
		cout<<"You are not connected"<<endl;
	else
		this->TCPtoServerCommandProtocol(CONNECTED_USERS);

}
/*
 * Function that helps send the request to print all the users
 */
void TCPMessengerClient::printAllUsers(){
	if(this->state == NOT_CONNECTED) //print all users ONLY when connected
		cout<<"You are not connected"<<endl;
	else
		this->TCPtoServerCommandProtocol(REG_USERS);
}

