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

/**
 * @param id the id of the thread
 * @param evenType
 * Ask/Receive/Work/Sleep/Complete
 * @param Q number of transactions received and waiting to be consumed
 * @param n The number received from work T or S
 **/
void logEvent(int id, char eventType, int Q, int n)
{
    pthread_mutex_lock(&wrtMutex);

    switch (eventType)
    {
    case 'W':
        cout << "work  id:" << id << "  n:" << n << endl;
        fprintf(fp, "Work");
        break;
    case 'A':
        cout << "Ask  id:" << id << endl;
        fprintf(fp, "Ask");
        break;
    case 'R':
        cout << "Receive  id:" << id << "  n:" << n << endl;
        fprintf(fp, "Receive");
        break;
    case 'S':
        cout << "Sleep  id:" << id << endl;
        fprintf(fp, "Sleep");
        break;
    case 'C':
        cout << "Completed  id:" << id << "  n:" << n << endl;
        fprintf(fp, "Completed");
        break;
    }

    pthread_mutex_unlock(&wrtMutex);
}

int getTaskQueSize()
{
    pthread_mutex_lock(&tqMutex);
    int size = taskQueue.size();
    pthread_mutex_unlock(&tqMutex);
    return size;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "missing arguements for nthreads" << endl;
        return 0;
    }
    int nthreads = atoi(argv[1]);
    Consumer *consumers[nthreads];
    int id = 0;
    char outPutFile[20];

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
    for (int i = 0; i <= nthreads; i++)
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
        cout << "Producer while looping" << endl;
        if (command_type == 'T')
        {
            // log parent received word
            logEvent(0, 'W', getTaskQueSize(), command_n);

            // start modifying the queue
            pthread_mutex_lock(&tqMutex);
            cout << "adding to queue" << endl;

            while (taskQueue.size() >= maxQueSize)
            {
                pthread_cond_wait(&tqNotFullCond, &tqMutex);
            }
            taskQueue.push(command_n);
            cout << "new task added to que" << endl;

            // check if the queue were just empty
            if (taskQueue.size() == 1)
            {
                pthread_cond_signal(&tqNotEmptyCond);
            }
            cout << "pthread_cond_broadcast(&tqNotEmptyCond) done" << endl;

            pthread_mutex_unlock(&tqMutex);
        }
        else if (command_type == 'S')
        {
            // need to log this as well;
            logEvent(0, 'S', NULL, command_n);
            Sleep(command_n);
        }
    }
    cout << "while loop ended" << endl;

    // producer no longer add new tasks
    pthread_mutex_lock(&tqMutex);
    producerCompleted = true;
    pthread_cond_broadcast(&tqNotEmptyCond);
    pthread_mutex_unlock(&tqMutex);

    // wait for all consumers to terminate
    for (int i = 0; i <= nthreads; i++)
    {
        pthread_join(consumers[i]->getPthreadId(), nullptr);
    }

    // TODO:implete print summery, use it here
    fclose(fp);
    for (int i = 0; i <= nthreads; i++)
    {
        delete consumers[i];
    }

    return 0;
}