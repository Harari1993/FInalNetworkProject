//============================================================================
// Name        : main.cpp
// Authors     : Reut Kravchook 24178636 and Inbal Sion 204185060
// Description : This is the main of the Messenger Server Application
//============================================================================

#include <iostream>
#include "TCPMessengerServer.h"

using namespace std;

void printInstructions()
{
	cout<<"Server Command List:"<<endl;
	cout<<"==============================================="<<endl;
	cout<<"List of Registered Users:         lu"<<endl;
	cout<<"List of Connected Users:          lcu"<<endl;
	cout<<"List Rooms:                       lr"<<endl;
	cout<<"List of Users in Rooms:           lru <roomName>"<<endl;
	cout<<"List of Sessions:                 ls"<<endl;
	cout<<"Exit:                             x"<<endl;
	cout<<"==============================================="<<endl;
}

int main()
{
	cout<<"Welcome to our messenger Server!"<<endl;
	printInstructions();
	TCPMessengerServer* msngrServer = new TCPMessengerServer();

	bool loop = true;
	while(loop){
		string msg;
		string command;
		cin >> command;
		if(command == "lcu")
		{
			msngrServer->getListOfPeers();
		}
		else if(command == "x")
		{
			for(unsigned int i =0 ; i<msngrServer->openPeerVect.size();i++)
			{
				msngrServer->sendCommandToTCP(SERVER_DISCONNECT,msngrServer->openPeerVect.at(i));
			}
			system("sleep 1");
			loop = false;
		}
		else if(command == "lr")
		{
			if(msngrServer->Rooms.size()>0)
			{
				for(unsigned int i = 0 ; i<msngrServer->Rooms.size();i++)
					cout<<i+1<<"."<<msngrServer->Rooms.at(i)->roomName<<endl;
			}
			else
			{
				cout<<"No Opened Rooms."<<endl;
			}
		}
		else if(command == "lru")
		{
			string roomname;
			cin>>roomname;
			msngrServer->printUsersinRoom(roomname);

		}
		else if(command == "lu")
		{
			msngrServer->printAllUsers();
		}
		else if(command == "ls")
			msngrServer->getListOfSessions();
		else
		{
			cout<<"wrong input"<<endl;
			printInstructions();
		}
	}
	msngrServer->close();
	delete msngrServer;
	cout<<"messenger was closed"<<endl;
	return 0;
}
