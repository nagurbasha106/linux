#include <stdio.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define KEY 123
#define MAX_BUFFER_SIZE 10
//#define SEM_COUNT MAX_BUFFER_SIZE

enum{
    FOR_0,
    FOR_1,
    MAX_SEM
}SEM_TYPE;
int semId = 0;
//int buffer[MAX_BUFFER_SIZE]={0};
int semDefaultValue[MAX_SEM]={1,0};
void initSem()
{
   semId = semget(KEY, MAX_SEM, IPC_CREAT | 0666);

   if(semId <= -1)
      perror("creating of semaphore returned error\n");

   int i = FOR_0;
   for(;i<=FOR_1;i++)
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

void* print0(void *arg)
{
    while(1)
    {
        semWait(FOR_0);
        printf("0 ");
        fflush(stdout);
        semPost(FOR_1);
        usleep(rand()%1000000);
    }
}

void* print1(void *arg)
{
    while(1)
    {
        semWait(FOR_1);
        printf("1 ");
        fflush(stdout);
        semPost(FOR_0);
    }
}

int main()
{
    pthread_t pri0_task;
    pthread_t pri1_task;

    initSem();

    pthread_create(&pri0_task, NULL, print0, NULL);
    pthread_create(&pri1_task, NULL, print1, NULL);

    pthread_join(pri0_task, NULL);
    pthread_join(pri1_task, NULL);

    return 0;
}

