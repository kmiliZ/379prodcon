#include "header.h"

Consumer::Consumer(int id)
{
    cout << "consumer with id:" << id << endl;
    this->id = id;

    pthread_create(&pthreadId, NULL, consume, this);
}

pthread_t Consumer::getPthreadId()
{
    return pthreadId;
}
// ask -> receive -> work -> complete -> ask ->.....loop until exit
// so TODO: there should be a while loop here;
void *Consumer::consume(void *args)
{
    Consumer *consumer = (Consumer *)args;
    int n;

    while (!producerCompleted || getTaskQueSize() > 0)
    {
        // cout << "consuming started" << endl;

        logEvent(consumer->id, 'A', NULL, NULL);
        pthread_mutex_lock(&tqMutex);
        cout << "quesize is " << taskQueue.size() << endl;

        while (taskQueue.size() == 0 && !producerCompleted)
        {
            // cout << "consumer cannot get a task: queue size zero";
            pthread_cond_wait(&tqNotEmptyCond, &tqMutex);
        }

        n = taskQueue.back();
        taskQueue.pop();

        // inform producer that the que is not full anymore
        if (taskQueue.size() == maxQueSize - 1)
        {
            pthread_cond_signal(&tqNotFullCond);
        }
        pthread_mutex_unlock(&tqMutex);

        // log received
        logEvent(consumer->id, 'R', NULL, n);

        // doing work
        Trans(n);

        // log completed
        logEvent(consumer->id, 'C', NULL, n);
    }

    return NULL;
}