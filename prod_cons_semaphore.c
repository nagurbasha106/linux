#include <stdio.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define KEY 786
#define MAX_BUFFER_SIZE 10
//#define SEM_COUNT MAX_BUFFER_SIZE

enum{
    MUTEX,
    FULL,
    EMPTY,
    MAX_SEM
}SEM_TYPE;
int semId = 0;
int buffer[MAX_BUFFER_SIZE]={0};
int semDefaultValue[MAX_SEM]={1,0, MAX_BUFFER_SIZE};
void initSem()
{
   semId = semget(KEY, MAX_SEM, IPC_CREAT | 0666);

   if(semId <= -1)
      perror("creating of semaphore returned error\n");

   int i = MUTEX;
   for(;i<=EMPTY;i++)
   {
       if(semctl(semId, i, SETVAL, semDefaultValue[i]) <= -1)
       {
           perror("setting semaphore error\n");
           return;
       }
   }
   return;
}

void semPost(int sem_num)
{
    struct sembuf sBuf = {sem_num, 1, 0};
    semop(semId, &sBuf, 1);
    return;

}
void semWait(int sem_num)
{
    struct sembuf sBuf = {sem_num, -1, 0};
    semop(semId, &sBuf, 1);
    return;
}

void* producer(void *arg)
{
    int count = 0;
    while(1)
    {
        semWait(EMPTY);
        semWait(MUTEX);
        buffer[count] = count;
	count++;
        count %= MAX_BUFFER_SIZE;
        semPost(MUTEX);
        semPost(FULL);
	usleep(rand()%1000000);
    }
}

void* consumer(void *arg)
{
    int count = 0;
    while(1)
    {
        semWait(FULL);
        semWait(MUTEX);
        printf("%d ", buffer[count]);
        fflush(stdout);
	count++;
        count %= MAX_BUFFER_SIZE;
        semPost(MUTEX);
        semPost(EMPTY);
	usleep(rand()%1000000);
    }
}

int main()
{
    pthread_t pro_task;
    pthread_t con_task;

    initSem();

    pthread_create(&pro_task, NULL, producer, NULL);
    pthread_create(&con_task, NULL, consumer, NULL);

    pthread_join(pro_task, NULL);
    pthread_join(con_task, NULL);

    return 0;
}

