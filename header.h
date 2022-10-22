#ifndef __PRODCON_H_
#define __PRODCON_H_

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <queue>
#include <cstring>
using namespace std;

// the queue to hold all transactions from producer
extern queue<int> taskQueue;
extern int maxQueSize;

extern FILE *fp;
extern bool producerCompleted;

// mutext
extern pthread_mutex_t tqMutex;
extern pthread_mutex_t wrtMutex;

// conditions
extern pthread_cond_t tqNotEmptyCond;
extern pthread_cond_t tqNotFullCond;

// Functions provided from class
void Trans(int n);
void Sleep(int n);

void logEvent(int id, char eventType, int Q, int n);
int getTaskQueSize();

class Consumer
{
public:
    /*
    This is the constructor of Consumber class
    It spawn a new process which start the consume
    function
    */
    Consumer(int id);
    int id;
    pthread_t getPthreadId();

private:
    /*
    consume the task it received after ask
    */
    pthread_t pthreadId;
    static void *consume(void *vargp);
};
#endif