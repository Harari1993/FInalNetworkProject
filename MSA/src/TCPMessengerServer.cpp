/*
 * TCPMessengerServer.cpp
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#include "TCPMessengerServer.h"
#include "TCPMessengerProtocol.h"

/*
 *Basic constructor, initializes the dispatcher and creates the socket for the server
 */
TCPMessengerServer::TCPMessengerServer(){
	posInVec=0; //<----
	serverSock = new TCPSocket(MSNGR_PORT);
	myDis = new Dispatcher(this);
	myLoginManager = new LoginManager(this);
	this->start();
}
/*
 * The servers main loop
 */
void TCPMessengerServer::run(){

	//Starts the dispatcher
	myDis->start();
	myLoginManager->start();

	while (true) {
		//Waits for incoming clients connection
		TCPSocket* tmpTCP= serverSock->listenAndAccept();
		if (tmpTCP != NULL) {
			connectedSocketsVect.push_back(tmpTCP);
			//insertToOpenVector(tmpTCP);
		}
	}

}
/*
 *This forwards messages to all the users in a room
 */
void TCPMessengerServer::sendMsgToAllUsersInRoom(int msgType,string roomName, string userName)
{
	int userIndex;
	int roomIndex = this->findInRooms(roomName);
	//This loop is runs on all users in the room and informs if a user has joined or left the room
	for(unsigned int i=0;i<this->Rooms.at(roomIndex)->usersInRoom.size();i++)
	{
		string tempIPtosend = Rooms.at(roomIndex)->usersInRoom.at(i);
		userIndex=this->findVector(openPeerVect, tempIPtosend);
		this->sendCommandToTCP(ROOM_STATUS_CHANGED,this->openPeerVect.at(userIndex));
		switch(msgType)
		{
		case JOIN_ROOM:
		{
			//Informs all users about the new user who joined the room
			string tempMsg = userName;
			tempMsg.append(" Has Joined the room");
			int numOfUsers =Rooms.at(roomIndex)->countUsers();
			this->sendMsgToTCP(tempMsg,this->openPeerVect.at(userIndex));
			this->sendCommandToTCP(numOfUsers,this->openPeerVect.at(userIndex));
			string usersVectorString = this->transferUsersInRoomToString(roomName);
			//Sends the updated users list to all peers in the roomName
			this->sendMsgToTCP(usersVectorString,this->openPeerVect.at(userIndex));
			break;
		}
		case LEAVE_ROOM:
		{
			//Informs all users about the user that left
			string tempMsg = userName;
			tempMsg.append(" Has Left the room");
			int numOfUsers =Rooms.at(roomIndex)->countUsers();
			this->sendMsgToTCP(tempMsg,this->openPeerVect.at(userIndex));
			this->sendCommandToTCP(numOfUsers,this->openPeerVect.at(userIndex));
			string usersVectorString = this->transferUsersInRoomToString(roomName);
			//Sends the updated users list to all peers in the roomName
			this->sendMsgToTCP(usersVectorString,this->openPeerVect.at(userIndex));
			break;
		}
		}
	}
}

/*
 * Simple function that returns the names of the users in the room in a string
 */
string TCPMessengerServer::transferUsersInRoomToString(string roomName)
{
	string tempIp;
	int roomIndex = this->findInRooms(roomName);
	for(unsigned int i=0;i<this->Rooms.at(roomIndex)->usersInRoom.size();i++)
	{
		tempIp.append(this->Rooms.at(roomIndex)->usersInRoom.at(i));
		if(i!=this->Rooms.at(roomIndex)->usersInRoom.size()-1)
			tempIp.append(" ");
	}

	return tempIp;
}


void TCPMessengerServer::printOpenPeerVector()
{
	for(unsigned int i=0;i<this->openPeerVect.size();i++)
	{
		cout<<"PrintOpenPeerVector peer : "<<i<<" "<<openPeerVect.at(i)->destIpAndPort()<<endl;
	}
}
/*
 * prints the peers
 */
void TCPMessengerServer::getListOfPeers(){
	if(openPeerVect.size()==0)
	{
		cout<<"no peers connected"<<endl;
	}
	else
	{
		if(openPeerVect.size()!=0)
		{
			for(unsigned int i = 0;i<openPeerVect.size();i++)
			{
				if(openPeerVect.at(i)!=NULL)
				{
					string tmp = ipTOclientName.at(i);
					cout << tmp <<endl;
				}
			}
		}

	}
}
/*
 * Simply prints all the users from the file
 */
void TCPMessengerServer::printAllUsers()
{
	//GetUserNamesFromData actually returns a vector from the file
	vector<string> tempVect = this->getUserNamesFromData();
	int numOfUsers = tempVect.size();
	for(int i=0 ; i< numOfUsers ; i++)
	{
		cout<<i+1<<". "<<tempVect.at(i)<<endl;
	}
}

void TCPMessengerServer::insertToOpenVector(TCPSocket* temp_soc)
{
	openPeerVect.push_back(temp_soc);
}
/*
 * Helper function that sends a message on the tcp connection
 */
void TCPMessengerServer::sendCommandToTCP(int protocol,TCPSocket * tmpTCP)
{
	protocol = htonl(protocol);
	tmpTCP->send((char*)&protocol,4);
}

//Finds the vector's index of a certain peer by IP address in the vector of open peers
int TCPMessengerServer::findVector(vector<TCPSocket*> &vec, string address)
{
	unsigned int i;
	bool found=false;
	for(i=0;i<vec.size();i++)
	{
		string tmp = vec.at(i)->destIpAndPort();

		if(tmp==address)
		{
			found=true;
			break;
		}
	}
	if(found)
		return i;
	else
		return -1;
}
/*
 * Helper function that recieves the commands
 */
int TCPMessengerServer::recieveCommandFromTCP(TCPSocket * tmpTCP) {
	int command;
	tmpTCP->recv((char*)&command,4);
	command=htonl(command);
	return command;
}
/*
 * Helper function that recieves messages(used only if the command requires additional data)
 */
char * TCPMessengerServer::recieveMessageFromTCP(TCPSocket * tmpTCP)
{
	int msgLen;
	char* buffer =new char[1024];
	bzero(buffer,1024);
	tmpTCP->recv((char *)&msgLen,4);
	msgLen = ntohl(msgLen);
	tmpTCP->recv(buffer,msgLen);

	return buffer;
}
/*
 * Returns the ip of a certain user
 */
string TCPMessengerServer::nameToIp(string userName)
{
	for(unsigned int i=0; i<ipTOclientName.size();i++)
	{
		char* tempCharStar = strdup(ipTOclientName[i].c_str());

		string tmpUser = strtok(tempCharStar," ");
		string tmpIp = strtok(NULL, " ");

		free(tempCharStar);
		if(strcmp(tmpUser.c_str(),userName.c_str())==0)
		{
			return tmpIp;
		}
	}

	return "UserName was not found";
}
/*
 * Lists all the sessions
 */
void  TCPMessengerServer::getListOfSessions() {
	if (this->initiatorSession.size() == 0) {
		cout << "There are no open sessions" << endl;
	} else {
		for(unsigned int i = 0; i < this->initiatorSession.size(); i++) {
			cout << this->ipToName(initiatorSession.at(i)) << " || " << this->ipToName(wantedSession.at(i)) << endl;
		}
	}
}
/*
 * Helper function that sends messages
 */
void TCPMessengerServer::sendMsgToTCP(string msg, TCPSocket* tmpTCP){
	int msglen = htonl(msg.length());
	tmpTCP->send((char*)&msglen, 4);
	tmpTCP->send(msg.c_str(), msg.length());
}
/*
 * A function that looks for a room in the vector of open rooms
 */
int TCPMessengerServer::findInRooms(string roomName)
{
	bool foundroom=false;
	unsigned int roomIndex;
	for(roomIndex=0;roomIndex<this->Rooms.size();roomIndex++)
	{
		if(roomName==this->Rooms.at(roomIndex)->roomName)
		{
			foundroom=true;
			break;
		}
	}
	if(foundroom)
		return roomIndex;
	else
		return -1;
}

//Matches a given IP to a user, and returns it's name
string TCPMessengerServer::ipToName(string ip){

	for(unsigned int i=0; i<ipTOclientName.size();i++)
	{
		char* tempCharStar = strdup(ipTOclientName[i].c_str());

		string tmpUser = strtok(tempCharStar," ");
		string tmpIp = strtok(NULL, " ");

		free(tempCharStar);
		if(strcmp(tmpIp.c_str(),ip.c_str())==0)
		{
			return tmpUser;
		}
	}

	return NULL;
}
/*
 * Prints all the users in a room
 */
void TCPMessengerServer::printUsersinRoom(string roomname)
{
	int roomIndex = this->findInRooms(roomname);
	if(roomIndex!=-1)
	{
		for(unsigned int i=0;i<this->Rooms.at(roomIndex)->usersInRoom.size();i++)
		{
			string tempname = this->ipToName(Rooms.at(roomIndex)->usersInRoom.at(i));
			cout<<i+1<<"."<<tempname<<endl;
		}
	}
	else
	{
		cout<<"No such room"<<endl;
	}
}
void TCPMessengerServer::close(){

	serverSock->cclose();
	this->openPeerVect.clear();
	this->connectedSocketsVect.clear();
	this->myDis->isON = false;
	this->myLoginManager->isON = false;

	delete this->myDis->MTL;
	delete myLoginManager->MTL;
	//delete myLoginManager;
}

/*
 * This function gets all the user names from the file and returns them in a vector
 */
vector<string> TCPMessengerServer::getUserNamesFromData(){

	//Push all user names to a vector and returns it.
	ifstream myReadFile;
	myReadFile.open("users.txt");
	vector<string> userNameBuffer;
	string output;

	if (myReadFile.is_open())
	{
		while (myReadFile >> output)
		{
			userNameBuffer.push_back(output);

		}
	}

	myReadFile.close();
	return userNameBuffer;

}
