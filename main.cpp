/*
TODO:
1. deal with consumer logging data
2. log data properly
3. log summary
*/

#include "header.h"
queue<int> taskQueue;
int maxQueSize;

FILE *fp;
bool producerCompleted;

// mutext
pthread_mutex_t tqMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t wrtMutex = PTHREAD_MUTEX_INITIALIZER;

// conditions
pthread_cond_t tqNotEmptyCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t tqNotFullCond = PTHREAD_COND_INITIALIZER;

int workCount, askCount, completeCount, sleepCount, receiveCount;

// Starting time
Clock::time_point startTime;

int getTaskQueSize()
{
    pthread_mutex_lock(&tqMutex);
    int size = taskQueue.size();
    pthread_mutex_unlock(&tqMutex);
    return size;
}

/**
 * @return return time passed in seconds
 * since the process started
 */
float getTimeDuration()
{
    Clock::time_point now = Clock::now();
    duration<double> duration = now - startTime;
    return duration.count();
}
/**
 * @param id the id of the thread
 * @param evenType
 * Ask/Receive/Work/Sleep/Complete/End
 * @param n The number received from work T or S
 **/
void logEvent(int id, char eventType, int queSize, int n)
{
    pthread_mutex_lock(&wrtMutex);
    float duration = getTimeDuration();
    char nStr[] = "    ";
    char qStr[] = "    ";
    char eventStr[] = "          ";
    if (n != NULL)
    {
        sprintf(nStr, "%4d", n);
    }
    switch (eventType)
    {
    case 'W':
        workCount++;
        sprintf(eventStr, "%s", "Work");
        sprintf(qStr, "Q=%2d", queSize);
        break;
    case 'A':
        askCount++;
        sprintf(eventStr, "%s", "Ask");
        break;
    case 'R':
        receiveCount++;
        sprintf(eventStr, "%s", "Receive");
        sprintf(qStr, "Q=%2d", queSize);
        break;
    case 'S':
        sleepCount++;
        sprintf(eventStr, "%s", "Sleep");
        break;
    case 'C':
        completeCount++;
        sprintf(eventStr, "%s", "Completed");
        break;
    case 'E':
        sprintf(eventStr, "%s", "End");
        break;
    }

    fprintf(fp, OUT_PUT_TEMPLATE, duration, id, qStr, eventStr, nStr);

    pthread_mutex_unlock(&wrtMutex);
}

void logSummery(int nthreads, Consumer *consumers[])
{
    fprintf(fp, "Summary:\n");
    fprintf(fp, "   Work        %2d\n", workCount);
    fprintf(fp, "   Ask         %2d\n", askCount);
    fprintf(fp, "   Receive     %2d\n", receiveCount);
    fprintf(fp, "   Completed   %2d\n", completeCount);
    fprintf(fp, "   Sleep       %2d\n", sleepCount);
    for (int i = 0; i < nthreads; i++)
    {
        fprintf(fp, "   Thread %2d   %2d\n", consumers[i]->getConsumerId(), consumers[i]->getTaskCount());
    }
    float totalDuration = getTimeDuration();
    fprintf(fp, "Transactions per second: %.2f", (float)workCount / totalDuration);
}

int main(int argc, char *argv[])
{
    startTime = Clock::now();

    if (argc < 2)
    {
        cerr << "missing arguements for nthreads" << endl;
        return 0;
    }
    int nthreads = atoi(argv[1]);
    Consumer *consumers[nthreads];
    int id = 0;
    char outPutFile[20];
    workCount = 0;
    askCount = 0;
    completeCount = 0;
    sleepCount = 0;
    receiveCount = 0;

    // set up output file name
    if (argc >= 3)
    {
        id = atoi(argv[2]);
        sprintf(outPutFile, "prodcon.%d.log", id);
    }
    else
    {
        strcpy(outPutFile, "prodcon.log");
    }

    // initialized the flag to be false
    producerCompleted = false;
    fp = fopen(outPutFile, "w");

    // create consumers
    for (int i = 0; i < nthreads; i++)
    {
        consumers[i] = new Consumer(i + 1);
    }

    maxQueSize = nthreads * 2;

    // reads input
    char command_type;
    int command_n;
    int queSize;

    while (scanf("%c%u", &command_type, &command_n) > 0)
    {
        if (command_type == 'T')
        {
            // start modifying the queue
            pthread_mutex_lock(&tqMutex);
            while (taskQueue.size() >= maxQueSize)
            {
                pthread_cond_wait(&tqNotFullCond, &tqMutex);
            }
            taskQueue.push(command_n);

            // check if the queue were just empty
            if (taskQueue.size() == 1)
            {
                pthread_cond_signal(&tqNotEmptyCond);
            }

            // log parent received word
            logEvent(0, 'W', taskQueue.size(), command_n);

            pthread_mutex_unlock(&tqMutex);
        }
        else if (command_type == 'S')
        {
            // need to log this as well;
            logEvent(0, 'S', NULL, command_n);
            Sleep(command_n);
        }
    }
    logEvent(0, 'E', NULL, command_n);

    // producer no longer add new tasks
    producerCompleted = true;
    pthread_mutex_lock(&tqMutex);
    pthread_cond_broadcast(&tqNotEmptyCond);
    pthread_mutex_unlock(&tqMutex);

    // wait for all consumers to terminate
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(consumers[i]->getPthreadId(), nullptr);
    }

    // TODO:implete print summery, use it here
    logSummery(nthreads, consumers);

    fclose(fp);
    for (int i = 0; i < nthreads; i++)
    {
        delete consumers[i];
    }

    return 0;
}