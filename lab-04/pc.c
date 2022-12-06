#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

struct sembuf producerLock[2] = {
    {1, -1, 0},
    {2, -1, 0},
};

struct sembuf producerRelease[2] = {
    {0, 1, 0},
    {2, 1, 0},
};

void producerRun(int producerId, char **ppProducedChar, int semid)
{
    // Set a new random seed.
    srand(time(NULL) + producerId);

    // Sleep for a while.
    sleep(rand() % 5 + 2);

    if (semop(semid, producerLock, 2) == -1)
    {
        perror("Producer failed to lock");
        exit(1);
    }

    char producedChar = *(*ppProducedChar)++;
    **ppProducedChar = ++producedChar;

    printf("Producer #%d put: %c\n", producerId, producedChar);

    if (semop(semid, producerRelease, 2) == -1)
    {
        perror("Producer failed to release");
        exit(1);
    }
}

void producerCreate(int producerId, char **ppProducedChar, int semid)
{
    pid_t producerPid;

    if ((producerPid = fork()) == -1)
    {
        perror("Producer failed to fork");
        exit(1);
    }

    else if (producerPid == 0)
    {
        for (int i = 0; i < 8; i++)
            producerRun(producerId, ppProducedChar, semid);

        exit(0);
    }
}

struct sembuf consumerLock[2] = {
    {0, -1, 0},
    {2, -1, 0},
};

struct sembuf consumerRelease[2] = {
    {1, 1, 0},
    {2, 1, 0},
};

void consumerRun(int consumerId, char **ppConsumedChar, int semid)
{
    srand(time(NULL) + consumerId);
    sleep(rand() % 7 + 1);

    if (semop(semid, consumerLock, 2) == -1)
    {
        perror("Consumer failed to lock");
        exit(1);
    }

    printf("Consumer #%d got: %c\n", consumerId, *((*ppConsumedChar)++));

    if (semop(semid, consumerRelease, 2) == -1)
    {
        perror("Consumer failed to release");
        exit(1);
    }
}

void consumerCreate(int consumerId, char **ppConsumedChar, int semid)
{
    pid_t consumerPid;

    if ((consumerPid = fork()) == -1)
    {
        perror("Consumer failed to fork");
        exit(1);
    }

    if (consumerPid == 0)
    {
        for (int i = 0; i < 8; i++)
            consumerRun(consumerId, ppConsumedChar, semid);

        exit(0);
    }
}

int main()
{
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int shmid = shmget(101, 2 * sizeof(char *) + 25 * sizeof(char), IPC_CREAT | perms);

    if (shmid == -1)
        perror("shmget"), exit(1);

    void *pSharedMemory = shmat(shmid, 0, 0);

    if (pSharedMemory == -1)
        perror("shmat"), exit(1);

    char **ppProducedChar = pSharedMemory;
    char **ppConsumedChar = pSharedMemory + sizeof(char *);

    *ppProducedChar = pSharedMemory + 2 * sizeof(char *);
    *ppConsumedChar = pSharedMemory + 2 * sizeof(char *) + sizeof(char);

    **ppProducedChar = 'a' - 1;

    int semid = semget(10, 3, IPC_CREAT | perms);

    if (semid == -1)
        perror("semget"), exit(1);

    if (semctl(semid, 2, SETVAL, 1) == -1)
        perror("semctl"), exit(1);

    if (semctl(semid, 0, SETVAL, 0) == -1)
        perror("semctl"), exit(1);

    if (semctl(semid, 1, SETVAL, 24) == -1)
        perror("semctl"), exit(1);

    for (int producerId = 1; producerId < 4; producerId++)
        producerCreate(producerId, ppProducedChar, semid);

    for (int consumerId = 1; consumerId < 4; consumerId++)
        consumerCreate(consumerId, ppConsumedChar, semid);

    for (int i = 0; i < 6; i++)
    {
        int stat;

        if (wait(&stat) == -1)
            perror("wait"), exit(1);

        if (!WIFEXITED(stat))
            printf("A child has terminated abnormally!\n");
    }

    if (shmdt(pSharedMemory) == -1)
        perror("shmdt"), exit(1);

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        perror("shmctl"), exit(1);

    if (semctl(semid, 0, IPC_RMID) == -1)
        perror("semctl"), exit(1);

    return 0;
}
