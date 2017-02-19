/*
 * Dispatcher.cpp
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#include "Dispatcher.h"
#include "TCPMessengerServer.h"
#include "TCPMessengerProtocol.h"

/*
 * The constructor recieves the TCPMessengerServer for further queries
 */
Dispatcher::Dispatcher(TCPMessengerServer* mainServer)
{
	MTL = NULL;
	this->mainServer = mainServer;
	isON = true;
}
/*
 * The main loop of the dispatcher
 */
void Dispatcher::run(){

	while (isON) {
		string buffer;
		//initialize multiple socket listener
		if (MTL != NULL)
			delete MTL;

		MTL = new MultipleTCPSocketsListener();
		MTL->addSockets(mainServer->openPeerVect);
		//Get the socket for a user
		TCPSocket* currentUser = MTL->listenToSocket(2);
		if(currentUser==NULL) {
			continue;
		}

		//we handle the current socket by ip and port
		string currentConnectedIPandPort = currentUser->destIpAndPort();
		cout<<"Incoming message from "<<currentConnectedIPandPort<<endl;

		//makes the sockets wait for incoming messages and parses commands if sent
		switch(mainServer->recieveCommandFromTCP(currentUser))
		{
		//Receieves a wanted userName by the initiator to open session with
		/*
		 * Having problems here
		 */
		case OPEN_SESSION_WITH_PEER:
		{
			string requested_username = mainServer->recieveMessageFromTCP(currentUser);
			string requested_userIP = mainServer->nameToIp(requested_username);
			//Check To see if the user is connected
			if(strcmp(requested_userIP.c_str(),"UserName was not found")!=0)
			{
				//Returns the requested peer's index from the vector of open peers
				int indexOfrequestedPeer = mainServer->findVector(mainServer->openPeerVect, requested_userIP);

				TCPSocket* tempTCPPeerToConnect = mainServer->openPeerVect.at(indexOfrequestedPeer);
				//Sends OPEN_SESSION_WITH_PEER command to the wantedUserName
				mainServer->sendCommandToTCP(OPEN_SESSION_WITH_PEER,tempTCPPeerToConnect);

				//Check the wantedUserName's state
				int temp = mainServer->recieveCommandFromTCP(tempTCPPeerToConnect);
				if(temp == IN_SESSION)
				{
					puts("The requested is IN SESSION");
					mainServer->sendCommandToTCP(SESSION_REFUSED_SESSION,currentUser);
				}
				else if(temp == IN_ROOM)
				{
					puts("The requested is IN ROOM");
					mainServer->sendCommandToTCP(SESSION_REFUSED_ROOM,currentUser);
				}
				else if (temp == LOGGED_IN)
				{
					//Sends SESSION_ESTABLISHED message to the initiator with the UDP settings
					mainServer->sendCommandToTCP(SESSION_ESTABLISHED,currentUser);
					mainServer->sendMsgToTCP(requested_username+" "+requested_userIP,currentUser);

					//Gets the initiator's userName
					string currentUserName = mainServer->ipToName(currentUser->destIpAndPort());

					//Sends SESSION_ESTABLISHED message to the wanted with the UDP settings
					mainServer->sendCommandToTCP(SESSION_ESTABLISHED,tempTCPPeerToConnect);
					mainServer->sendMsgToTCP(currentUserName+" "+currentUser->destIpAndPort(),tempTCPPeerToConnect);

					//Extracts initiator and wanted peers to the strings vector
					mainServer->initiatorSession.push_back(currentUser->destIpAndPort());
					mainServer->wantedSession.push_back(tempTCPPeerToConnect->destIpAndPort());
					puts("User is available");
				}
			}
			else
			{
				//Sends SESSION_REFUSED message to the initiator (wanted userName was not found)
				mainServer->sendCommandToTCP(SESSION_REFUSED,currentUser);
			}

			break;
		}

		case JOIN_ROOM: //Receive room name and add user to that room
		{
			string roomName = mainServer->recieveMessageFromTCP(currentUser);
			//checking if the room exists
			int roomIndex=mainServer->findInRooms(roomName);

			if(roomIndex!=-1)
			{
				//Sends a command to the client that approves its' joining
				mainServer->sendCommandToTCP(JOIN_ROOM_ARPROVED,currentUser);
				//Sends a message to the client with the room's name
				mainServer->sendMsgToTCP(roomName,currentUser);
				//Adds the new user to the room's users list(ip:port)
				mainServer->Rooms.at(roomIndex)->AddUserToRoom(currentUser->destIpAndPort());
				//Gets the user's name by the client's IP
				string userNameToSend = mainServer->ipToName(currentUser->destIpAndPort());
				//Informs all users in the room that the new user has been joined
				mainServer->sendMsgToAllUsersInRoom(JOIN_ROOM,roomName,userNameToSend);
			}
			else
			{
				//Sending a failure message in case the room doesnt exist
				mainServer->sendCommandToTCP(NO_SUCH_ROOM_NAME,currentUser);
			}

			break;
		}
		case CLOSE_SESSION_WITH_PEER:
		{
			unsigned int i;
			string closeRequestPeer = currentUser->destIpAndPort();
			int indexInOpenVect;
			for(i=0;i<mainServer->initiatorSession.size();i++)
			{
				//Returns the location of the session
				if(mainServer->initiatorSession.at(i) == closeRequestPeer)
				{
					indexInOpenVect = mainServer->findVector(mainServer->openPeerVect, mainServer->wantedSession.at(i));
					break;
				}
				if(mainServer->wantedSession.at(i)==closeRequestPeer)
				{
					indexInOpenVect = mainServer->findVector(mainServer->openPeerVect, mainServer->initiatorSession.at(i));
					break;
				}

			}
			//Erases the sessions from wantedSession+initiatorSession Vectors
			mainServer->initiatorSession.erase(mainServer->initiatorSession.begin()+i);
			mainServer->wantedSession.erase(mainServer->wantedSession.begin()+i);
			//Sends CLOSE_SESSION_WITH_PEER to the other peer
			mainServer->sendCommandToTCP(CLOSE_SESSION_WITH_PEER,mainServer->openPeerVect.at(indexInOpenVect));
			break;
		}
		case CREATE_NEW_ROOM:
		{
			string roomName = mainServer->recieveMessageFromTCP(currentUser);
			//Check if that roomName already exists
			int roomIndex = mainServer->findInRooms(roomName);

			if(roomIndex!=-1)
			{
				//There is already a room with that name. Sends ROOM_NOT_UNIQUE message to currentUser
				mainServer->sendCommandToTCP(ROOM_NOT_UNIQUE,currentUser);
			}
			else
			{
				//roomName is available, defines a owner (currentUser)
				string owner = mainServer->ipToName(currentUser->destIpAndPort());
				//Creates new room with the Ip&Port of the current user and push it to the Rooms Vector
				mainServer->Rooms.push_back(new Room(roomName,currentUser->destIpAndPort(),owner));
				//Sends CREATE_ROOM_APPROVED command to owner
				mainServer->sendCommandToTCP(CREATE_ROOM_APPROVED,currentUser);
			}
			break;
		}
		case LEAVE_ROOM:
		{
			//Gets the roomName to be closed
			string roomNametoLeave = mainServer->recieveMessageFromTCP(currentUser);
			//Returns the index of the desired roomName from Rooms vector
			int roomIndex = this->mainServer->findInRooms(roomNametoLeave);
			mainServer->Rooms.at(roomIndex)->RemoveUserFromRoom(currentUser->destIpAndPort());
			//Gets the name of the leaving user
			string tempNameFromIp= mainServer->ipToName(currentUser->destIpAndPort());
			//Sends a message to all users in the room that that user has been removed from the room
			mainServer->sendMsgToAllUsersInRoom(LEAVE_ROOM,roomNametoLeave,tempNameFromIp);
			mainServer->sendCommandToTCP(LEFT_ROOM, currentUser);
			break;
		}
		case CLOSE_ROOM_REQUEST:
		{
			string roomNametoClose = mainServer->recieveMessageFromTCP(currentUser);
			int roomIndex = this->mainServer->findInRooms(roomNametoClose);

			//Checks if the currentsUser is the owner of roomNametoClose
			string nameOfCLientRequest=mainServer->ipToName(currentUser->destIpAndPort());
			string nameOfowner=mainServer->Rooms.at(roomIndex)->hostName;
			if(nameOfCLientRequest==nameOfowner)
			{
				//Loops on all users in the room and sends ROOM_CLOSED command
				for(unsigned int i = 0; i<this->mainServer->Rooms.at(roomIndex)->usersInRoom.size();i++)
				{
					string tempIptoSendClose = this->mainServer->Rooms.at(roomIndex)->usersInRoom.at(i);
					int userIndex = mainServer->findVector(mainServer->openPeerVect, tempIptoSendClose);
					mainServer->sendCommandToTCP(ROOM_CLOSED,mainServer->openPeerVect.at(userIndex));

				}
				//Erases the room from Rooms Vector
				mainServer->Rooms.erase(mainServer->Rooms.begin()+roomIndex);
			}
			else
				mainServer->sendCommandToTCP(CLOSE_ROOM_DENIED,currentUser);
			break;
		}
		case CONNECTED_USERS: //simple print all connected users
		{
			string stringOfUsersName;
			string tempNameFromIp;
			int numberOfUsers= mainServer->openPeerVect.size();

			for(unsigned int i=0;i<mainServer->openPeerVect.size();i++)
			{
				tempNameFromIp=mainServer->ipToName(mainServer->openPeerVect.at(i)->destIpAndPort());
				stringOfUsersName.append(tempNameFromIp);
				if(i!=mainServer->openPeerVect.size()-1)
					stringOfUsersName.append(" ");
			}

			if(numberOfUsers > 0)
			{
				mainServer->sendCommandToTCP(PRINT_DATA_FROM_SERVER,currentUser);
				mainServer->sendCommandToTCP(numberOfUsers,currentUser);
				mainServer->sendMsgToTCP(stringOfUsersName,currentUser);
			}
			else
				mainServer->sendCommandToTCP(52, currentUser);
			break;
		}
		case USERS_IN_ROOM:
		{
			string roomNameFromClient = mainServer->recieveMessageFromTCP(currentUser);
			int roomIndex=mainServer->findInRooms(roomNameFromClient);

			if(roomIndex!=-1)
			{

				string stringOfUsersName;
				string tempNameFromIp;
				//Loops on all users in the room
				for(unsigned int i=0;i< mainServer->Rooms.at(roomIndex)->usersInRoom.size();i++)
				{
					//Convert Ip&Port to user name
					tempNameFromIp=mainServer->ipToName(mainServer->Rooms.at(roomIndex)->usersInRoom.at(i));
					//Adds the user name to the string by location(i)
					stringOfUsersName.append(tempNameFromIp);
					if(i!=mainServer->Rooms.at(roomIndex)->usersInRoom.size()-1)
						stringOfUsersName.append(" ");
				}

				int numOfUsersInRoom = mainServer->Rooms.at(roomIndex)->usersInRoom.size();

				mainServer->sendCommandToTCP(PRINT_DATA_FROM_SERVER,currentUser);

				//Sends the numOfUsersInRoom as numOfIter
				mainServer->sendCommandToTCP(numOfUsersInRoom,currentUser);
				mainServer->sendMsgToTCP(stringOfUsersName,currentUser);

			}
			else
			{
				//roomName was not found in Room's vector
				mainServer->sendCommandToTCP(NO_SUCH_ROOM_NAME,currentUser);
			}
			break;
		}
		case EXISTED_ROOMS:
		{

			int numOfRoom = mainServer->Rooms.size();
			string stringOfRooms;
			//Loops on all rooms in server
			for(unsigned int i=0;i<mainServer->Rooms.size();i++)
			{
				//Adds the room name to the string by location(i)
				stringOfRooms.append(mainServer->Rooms.at(i)->roomName);
				if(i!=mainServer->Rooms.size()-1)
					stringOfRooms.append(" ");
			}
			if(numOfRoom > 0)
			{
				mainServer->sendCommandToTCP(PRINT_DATA_FROM_SERVER,currentUser);

				//Sends the numOfRoom as numOfIter
				mainServer->sendCommandToTCP(numOfRoom,currentUser);
				mainServer->sendMsgToTCP(stringOfRooms,currentUser);
			}
			else
				mainServer->sendCommandToTCP(NO_ROOMS, currentUser);
			break;
		}
		case REG_USERS:
		{
			//Push all userNames to a vector from users.txt
			vector<string> tempVect = mainServer->getUserNamesFromData();
			int numOfUsers = tempVect.size();
			string users;
			for(int i = 0; i <numOfUsers;i++){
				users.append(tempVect.at(i));
				if(i!=numOfUsers-1)
					users.append(" ");
			}
			if (numOfUsers>0)
			{
				mainServer->sendCommandToTCP(PRINT_DATA_FROM_SERVER,currentUser);
				//Sends the numOfRoom as numOfIter
				mainServer->sendCommandToTCP(numOfUsers,currentUser);
				mainServer->sendMsgToTCP(users,currentUser);
			}
			else
			{
				mainServer->sendCommandToTCP(NO_USERS, currentUser);
			}

			break;
		}
		case DISCONNECT:
		{
			//Gets the user's index from openPeerVect and erase it
			int indexUser = mainServer->findVector(mainServer->openPeerVect,currentUser->destIpAndPort());
			mainServer->openPeerVect.erase(mainServer->openPeerVect.begin() + indexUser);

			break;
		}
		}
	}
}

Dispatcher::~Dispatcher() {

}
