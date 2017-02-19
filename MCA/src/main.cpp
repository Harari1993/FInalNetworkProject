//============================================================================
// Name        : main.cpp
// Authors     : Reut Kravchook 24178636 and Inbal Sion 204185060
// Description : This is the main of the Messenger Server Client
//============================================================================

#include <iostream>
#include <string.h>
#include "TCPMessengerClient.h"

using namespace std;

void printInstructions() {
	cout<<"============================================================"<<endl;
	cout << "Welcome to TCP messenger" << endl;
	cout << "Command List:"<<endl;
	cout << "To open connect to server type: 						 c <server ip>" << endl;
	cout << "To print the user list from the server type: 			 lu" << endl;
	cout << "To print the connected users list type: 				 lcu" << endl;
	cout << "To print all rooms type: 								 lr" << endl;
	cout << "To print all users in this room type: 					 lru <room name>" << endl;
	cout <<	"To login with the user and password type: 				 login <user> <password>" << endl;
	cout << "To register new user with given password type:          register <user> <password>" << endl;
	cout << "To open a session with the user type: 					 o <username>" << endl;
	cout << "To enter a chat room type:                              or <room name>" << endl;
	cout << "Opening session will close any previously active sessions" << endl;
	cout << "After the session is opened, to send message type type: s <message>" << endl;
	cout << "To print the current status of the client type:         l" << endl;
	cout << "To close opened session type:                           cs" << endl;
	cout << "To create new room type:                                cnr <room name>" << endl;
	cout << "To close a room type:                                   cr <room name>" << endl;
	cout << "To disconnect from server type:                         d" << endl;
	cout << "To close the app type:                                  x" << endl;
	cout<<"==========================================================="<<endl;
}

int main() {
	printInstructions();
	TCPMessengerClient* clientMessenger = new TCPMessengerClient();
	while (true) {
		string msg;
		string command;
		cin >> command;
		if(command == "c") {
			string ip;
			cin >> ip;
			clientMessenger->connect(ip);
		}
		else if(command == "lu") {
			clientMessenger->printAllUsers();
		}
		else if(command == "lru") {
			string roomName;
			cin>>roomName;
			clientMessenger->printAllUsersInRoomToServer(roomName);
		}
		else if(command == "lr") {
			clientMessenger->printAllRooms();
		}
		else if(command == "lcu") {
			clientMessenger->printConnectedUsers();
		}
		else if(command == "login") {
			string userName;
			string password;
			cin>>userName;
			cin>>password;
			clientMessenger->loginUser(userName,password);
		}
		else if(command == "register") {
			string userName;
			string password;
			cin>>userName;
			cin>>password;
			clientMessenger->registerUser(userName,password);
		}
		else if(command == "o") {
			string userName;
			cin >>userName;
			clientMessenger->open("user", userName);
		}
		else if (command == "or") {			// open a session with room
			string roomName;
			cin >> roomName;
			clientMessenger->open("room", roomName);
		}
		else if(command == "s") {
			getline(std::cin,msg);

			if (clientMessenger->state != IN_SESSION && clientMessenger->state != IN_ROOM) {
				cout<<"No opened session"<<endl;
				printInstructions();
			} else {
				clientMessenger->send(msg);
			}
		}
		else if(command == "l") {
			clientMessenger->printMyCurrentStatus();
		}
		else if(command == "cs") {
			if (clientMessenger->state == IN_SESSION)
				clientMessenger->closeActiveSession();
			else if (clientMessenger->state == IN_ROOM)
				clientMessenger->LeaveCurrentRoom();
			else
				cout<<"No Opened Sessions or Rooms."<<endl;
		}
		else if(command == "cnr") {
			string inputRoomName;
			cin>>inputRoomName;

			if (clientMessenger->state == IN_SESSION || clientMessenger->state == IN_ROOM) {
				cout << "Please close all Sessions/Rooms before creating a new one."<<endl;
			}
			else if(clientMessenger->state == NOT_CONNECTED) {
				cout << "The Client is offline." << endl;
			}
			else {
				clientMessenger->createNewRoom(inputRoomName);
			}
		}
		else if(command == "cr") {
			clientMessenger->closeRoom(clientMessenger->roomName);
		}

		else if(command == "d")	{
			clientMessenger->disconnect();
		}

		else if(command == "x") {
			clientMessenger->disconnect();
			break;
		}

		else {
			cout << "wrong input" << endl;
			printInstructions();
		}
	}

	clientMessenger->disconnect();
	delete clientMessenger;
	cout << "messenger was closed" << endl;
	return 0;
}


