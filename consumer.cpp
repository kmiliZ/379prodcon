#include "header.h"

Consumer::Consumer(int id)
{
    this->id = id;
    taskCompletedCount = 0;

    pthread_create(&pthreadId, NULL, consume, this);
}

pthread_t Consumer::getPthreadId()
{
    return this->pthreadId;
}

int Consumer::getConsumerId()
{
    return this->id;
}

void Consumer::increamentTaskCount()
{
    this->taskCompletedCount++;
}

int Consumer::getTaskCount()
{
    return this->taskCompletedCount;
}

// ask -> receive -> work -> complete -> ask ->.....loop until exit
void *Consumer::consume(void *args)
{
    Consumer *consumer = (Consumer *)args;
    int n;

    while (!producerCompleted || getTaskQueSize() > 0)
    {

        logEvent(consumer->id, 'A', NULL, NULL);
        pthread_mutex_lock(&tqMutex);
        while (taskQueue.size() == 0 && !producerCompleted)
        {
            pthread_cond_wait(&tqNotEmptyCond, &tqMutex);
        }
        if (producerCompleted && taskQueue.size() == 0)
        {

            pthread_mutex_unlock(&tqMutex);
            break;
        }
        n = taskQueue.back();
        taskQueue.pop();

        // inform producer that the que is not full anymore
        if (taskQueue.size() == maxQueSize - 1)
        {
            pthread_cond_signal(&tqNotFullCond);
        }

        // log received
        logEvent(consumer->id, 'R', taskQueue.size(), n);
        pthread_mutex_unlock(&tqMutex);

        // doing work
        Trans(n);

        // log completed
        logEvent(consumer->id, 'C', NULL, n);
        consumer->increamentTaskCount();
    }
    return NULL;
}