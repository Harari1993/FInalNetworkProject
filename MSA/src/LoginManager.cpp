/*
 * LoginManager.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: colman
 */

#include "LoginManager.h"
#include <iostream>
#include <fstream>
#include "TCPMessengerServer.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocol.h"

LoginManager::LoginManager(TCPMessengerServer* mainServer) {
	MTL = NULL;
	this->mainServer = mainServer;
	isON = true;
}

void LoginManager::run() {
	while (isON) {
		string buffer;
		//initialize multiple socket listener
		if (MTL != NULL)
			delete MTL;

		MTL = new MultipleTCPSocketsListener();
		MTL->addSockets(mainServer->connectedSocketsVect);
		//Get the socket for a user
		TCPSocket* currentUser = MTL->listenToSocket(2);
		if(currentUser==NULL) {
			continue;
		}

		//we handle the current socket by ip and port
		string currentConnectedIPandPort = currentUser->destIpAndPort();
		cout << "Incoming login req "<< currentConnectedIPandPort << endl;

		//makes the sockets wait for incoming messages and parses commands if sent
		switch(mainServer->recieveCommandFromTCP(currentUser))
		{
			case USER_LOGIN_REQUEST:
			{
				char* user = mainServer->recieveMessageFromTCP(currentUser);
				//parsing the messages for login
				string userName = strtok(user," ");
				string password = strtok(NULL," ");
				//tempAuthentication used to assure the user can connect
				int tempAuthentication = Login(userName,password);

				if(tempAuthentication==0){
					bool isAlreadyConnected=false;
					string tempName;
					//checking if the user is already connected
					for(unsigned int i =0 ; i< mainServer->openPeerVect.size(); i++) {
						cout << "open sock: " << i << endl;
						tempName = mainServer->ipToName(mainServer->openPeerVect.at(i)->destIpAndPort());
						if(userName==tempName) {
							isAlreadyConnected=true;
							break;
						}
					}

					if(!isAlreadyConnected){

						cout<<"User: "<<userName<<" Is Connected to server"<<endl;

						//we insert the newly connected user to a vector so that we can remember he is connected
						mainServer->insertToOpenVector(currentUser);
						string tempIptoName=userName+" "+currentUser->destIpAndPort();
						//We store the details of the user, with a name
						mainServer->ipTOclientName.push_back(tempIptoName);
						//Informs the client that the login has been approved, get ready for UDPManager settings
						mainServer->sendCommandToTCP(LOGIN_RESPONSE_APPROVE,currentUser);
						mainServer->sendMsgToTCP(currentUser->destIpAndPort(),currentUser);//Sends UDPManager settings

						int indexUser = mainServer->findVector(mainServer->connectedSocketsVect,currentUser->destIpAndPort());
						mainServer->connectedSocketsVect.erase(mainServer->connectedSocketsVect.begin() + indexUser);
					}
					else
					{
						//Informs the client that the login has failed
						mainServer->sendCommandToTCP(LOGIN_RESPONSE_ERROR,currentUser);
					}
				}
				else if(tempAuthentication==1)
				{
					//Fail to login, userName was not found in the database
					cout<<"the user name: "<<userName<<" does not exist"<<endl;
					mainServer->sendCommandToTCP(LOGIN_RESPONSE_ERROR,currentUser);
				}
				else{
					//Fail to login, password does not match the userName
					cout<<"the password: "<<password<<" is not correct for the user: "<<userName<<endl;
					mainServer->sendCommandToTCP(LOGIN_RESPONSE_ERROR,currentUser);
				}

				break;

			}
			case CREATE_NEW_USER:
			{
				//Starts the creation process for a new user
				string newUser = mainServer->recieveMessageFromTCP(currentUser);
				//Checks if that user already exists
				int registrationStatus = Register(newUser);
				if(registrationStatus==1)
				{
					//User name approved to be a new one and added to the database
					cout<<newUser<<" has been Registered and Connected to server"<<endl;

					char* tempUserAndPass = strdup(newUser.c_str());
					string userName = strtok(tempUserAndPass," ");
					string password = strtok(NULL, " ");


					//we insert the newly connected user to a vector so that we can remember he is connected
					mainServer->insertToOpenVector(currentUser);
					string tempIptoName=userName+" "+currentUser->destIpAndPort();
					//We store the details of the user, with a name
					mainServer->ipTOclientName.push_back(tempIptoName);
					//Informs the client that the login has been approved, get ready for UDPManager settings
					mainServer->sendCommandToTCP(LOGIN_RESPONSE_APPROVE,currentUser);
					mainServer->sendMsgToTCP(currentUser->destIpAndPort(),currentUser);//Sends UDPManager settings

					int indexUser = mainServer->findVector(mainServer->connectedSocketsVect,currentUser->destIpAndPort());
					mainServer->connectedSocketsVect.erase(mainServer->connectedSocketsVect.begin() + indexUser);

				}
				else {
					//User name already exists, send failure message to the client
					mainServer->sendCommandToTCP(NEW_USER_DENIED,currentUser);
					cout << "failed to register" << endl;
					//mainServer->SendMsgToTCP(currentUser->destIpAndPort(),currentUser);
				}

				break;
			}
			case DISCONNECT:
			{
				int indexUser = mainServer->findVector(mainServer->connectedSocketsVect,currentUser->destIpAndPort());
				mainServer->connectedSocketsVect.erase(mainServer->connectedSocketsVect.begin() + indexUser);
				break;
			}
		}
	}
}

/*
 * This function handles the login
 */
int LoginManager::Login(string userName, string password)
{
	ifstream myUsersFile;
	ifstream myPasswordsFile;
	myUsersFile.open("users.txt");
	myPasswordsFile.open("passwords.txt");
	string userNameBuffer[256];
	string passwordBuffer[256];
	int location;
	int numOfUsers=0;
	int numOfPasswords=0;

	//Extracts all user names from a text to a strings array and stores the size of it in numOfUsers
	if (myUsersFile.is_open()) {
		while (!myUsersFile.eof()) {
			myUsersFile >> userNameBuffer[numOfUsers];
			numOfUsers++;
		}
	}
	//Extracts all passwords from a text to a strings array and stores the size of it in numOfPasswords
	if (myPasswordsFile.is_open()) {
		while (!myPasswordsFile.eof()) {
			myPasswordsFile >> passwordBuffer[numOfPasswords];
			numOfPasswords++;
		}
	}
	//Checks if the userName exists in the users' database
	for(location=0;location<numOfUsers;location++)
	{
		if (strcmp(userName.c_str(), userNameBuffer[location].c_str()) ==0){
			{
				break;
			}
		}
	}
	//userName does not appear in database, meaning we broke the loop and location didnt iterate all the way to numOfUsers
	if(location==numOfUsers)
	{
		myUsersFile.close();
		myPasswordsFile.close();
		return 1;
	}
	//Checks if the password suits the userName's password (by the userName's location)
	if (strcmp(password.c_str(), passwordBuffer[location].c_str()) ==0)
	{
		myUsersFile.close();
		myPasswordsFile.close();
		return 0;
	}
	//The given password does not match the userName's password
	else
	{
		myUsersFile.close();
		myPasswordsFile.close();
		return 2;
	}
}

/*
 * Function that handles the registration
 */
int LoginManager::Register(string userNamePlusPassword){

	char* tempUserAndPass = strdup(userNamePlusPassword.c_str());
	string userName = strtok(tempUserAndPass," ");
	string password = strtok(NULL, " ");

	//Extracts all user names from a text to a strings array and checks if a given userName is already exists.
	ifstream myReadFile;
	myReadFile.open("users.txt");
	string userNameBuffer[256];
	int location;
	int numOfUsers=0;
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> userNameBuffer[numOfUsers];
			numOfUsers++;
		}
	}

	for(location=0;location<numOfUsers;location++)
	{
		if (strcmp(userName.c_str(), userNameBuffer[location].c_str()) ==0)
		{
			myReadFile.close();
			return 0;
		}
	}
	myReadFile.close();

	//Registers the new userName to the users' text file
	ofstream myWriteFile;
	myWriteFile.open("users.txt",ofstream::app);
	myWriteFile<<userName<<endl;
	myWriteFile.close();
	//Registers the new user name's password to the passwords' text file
	myWriteFile.open("passwords.txt",ofstream::app);
	myWriteFile<<password<<endl;
	myWriteFile.close();

	free(tempUserAndPass);
	return 1;
}

LoginManager::~LoginManager() {

}

