#include "header.h"

Consumer::Consumer(int id)
{
    cout << "consumer with id:" << id << endl;
    this->id = id;
    pthread_t pthreadId;
    pthread_create(&pthreadId, NULL, consume, this);
}

// ask -> receive -> work -> complete -> ask ->.....loop until exit
// so TODO: there should be a while loop here;
void *Consumer::consume(void *args)
{
    Consumer *consumer = (Consumer *)args;
    int n;
    cout << "consume function called" << endl;
    cout << "producerCompleted: " << !producerCompleted << endl;
    cout << (!producerCompleted && getTaskQueSize() > 0) << endl;

     while (!producerCompleted || getTaskQueSize() > 0)
    {
        cout << "in consumer's while loop" << endl;
        logEvent(consumer->id, 'A', NULL, NULL);
        pthread_mutex_lock(&tqMutex);
        if (getTaskQueSize() == 0)
        {
            pthread_cond_wait(&tqNotEmptyCond, &tqMutex);
        }

        n = taskQueue.back();
        taskQueue.pop();
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