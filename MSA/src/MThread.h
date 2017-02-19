/*
 * MThread.h
 *
 *  Created on: Feb 13, 2016
 *      Author: colman
 */

#ifndef MTHREAD_H_
#define MTHREAD_H_

#include <pthread.h>

class MThread{
public:

	virtual void run() = 0;
	void start();
	void waitForThread();
	virtual ~MThread();

	pthread_t threadId;
};

#endif /* MTHREAD_H_ */
