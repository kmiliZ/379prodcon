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
#include <chrono>
using namespace std;
using namespace chrono;
typedef system_clock Clock;

#define OUT_PUT_TEMPLATE_Q "%.3f ID=%2d Q=%2d %-10s %5s\n"
#define OUT_PUT_TEMPLATE "%.3f ID=%2d %4s %-10s %5s\n"

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

// summary counts
extern int workCount, askCount, completeCount, sleepCount, receiveCount;

// Functions provided from class
void Trans(int n);
void Sleep(int n);

void logEvent(int id, char eventType, int queSize, int n);
int getTaskQueSize();

class Consumer
{
public:
    /*
    This is the constructor of consumer class
    It spawn a new process which start the consume function
    */
    Consumer(int id);
    pthread_t getPthreadId();
    int getConsumerId();
    /*
    returns the total number
    of tasks the consumer completed
    */
    int getTaskCount();

private:
    pthread_t pthreadId;
    int id;
    int taskCompletedCount;
    static void *consume(void *vargp);
    void increamentTaskCount();
};
#endif