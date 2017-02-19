/*
 * Room.h
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#ifndef ROOM_H_
#define ROOM_H_

#include <vector>
#include <string>
#include <stdlib.h>
#include <iostream>
using namespace std;

class Room {
public:
	string hostIp_Port;
	string hostName;
	vector<string> usersInRoom;
	string roomName;

	Room(string roomName,string hostIp_Port,string hostName);
	virtual ~Room();
	void AddUserToRoom(string user);
	void RemoveUserFromRoom(string user);
	int countUsers();
	//void send

};

#endif /* ROOM_H_ */
