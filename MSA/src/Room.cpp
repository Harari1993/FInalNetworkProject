/*
 * Room.cpp
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#include "Room.h"

/*
 * A constructor for a room, stores the name of the room, the hosts ip and port and the name of the host
 */
Room::Room(string roomName,string hostIp_Port,string roomHostName) {
	this->roomName=roomName;
	hostName=roomHostName;
	this->hostIp_Port=hostIp_Port;
	AddUserToRoom(this->hostIp_Port);
	cout<<"The Room ["<<roomName<<"] Has been Created"<<endl;
}

Room::~Room() {
}

/*
 * Add user to this room
 */
void Room::AddUserToRoom(string user){
	usersInRoom.push_back(user);
}
/*
 * Removing user from this room
 */
void Room::RemoveUserFromRoom(string user){
	for(unsigned int i = 0 ; i<usersInRoom.size();i++){
		if(usersInRoom.at(i)==user)
		{
			usersInRoom.erase(usersInRoom.begin()+i);
		}
	}
}
/*
 *Returns the number of users in the room
 */
int Room::countUsers(){
	return usersInRoom.size();
}
