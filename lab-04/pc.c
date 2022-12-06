#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SEMCE 0
#define SEMCF 1
#define SEMMX 2

#define N 24

struct sembuf producerLock[2] = {
    {SEMCE, -1, 0},
    {SEMMX, -1, 0},
};

struct sembuf producerRelease[2] = {
    {SEMMX, +1, 0},
    {SEMCF, +1, 0},
};

void producerInitialize(char **produced)
{
    **produced = 'a' - 1;
}

void producerRun(int producerId, char **produced, int semid)
{
    sleep(rand() % 5 + 2);

    if (semop(semid, producerLock, 2) == -1)
    {
        perror("Producer failed to lock");
        exit(1);
    }

    char producedChar = *(*produced)++;
    **produced = ++producedChar;

    printf("Producer #%d put: %c\n", producerId, producedChar);

    if (semop(semid, producerRelease, 2) == -1)
    {
        perror("Producer failed to release");
        exit(1);
    }
}

void producerCreate(int producerId, char **produced, int semid)
{
    pid_t producerPid;

    if ((producerPid = fork()) == -1)
    {
        perror("Producer failed to fork");
        exit(1);
    }

    else if (producerPid == 0)
    {
        srand(time(NULL) + producerId);

        for (int i = 0; i < 8; i++)
            producerRun(producerId, produced, semid);

        exit(0);
    }
}

struct sembuf consumerLock[2] = {
    {SEMCF, -1, 0},
    {SEMMX, -1, 0},
};

struct sembuf consumerRelease[2] = {
    {SEMMX, +1, 0},
    {SEMCE, +1, 0},
};

void consumerRun(int consumerId, char **consumed, int semid)
{
    sleep(rand() % 7 + 1);

    if (semop(semid, consumerLock, 2) == -1)
    {
        perror("Consumer failed to lock");
        exit(1);
    }

    printf("Consumer #%d got: %c\n", consumerId, *((*consumed)++));

    if (semop(semid, consumerRelease, 2) == -1)
    {
        perror("Consumer failed to release");
        exit(1);
    }
}

void consumerCreate(int consumerId, char **consumed, int semid)
{
    pid_t consumerPid;

    if ((consumerPid = fork()) == -1)
    {
        perror("Consumer failed to fork");
        exit(1);
    }

    if (consumerPid == 0)
    {
        srand(time(NULL) + consumerId);

        for (int i = 0; i < 8; i++)
            consumerRun(consumerId, consumed, semid);

        exit(0);
    }
}

int main()
{
    int perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int shmid = shmget(101, 2 * sizeof(char *) + 25 * sizeof(char), IPC_CREAT | perm);

    if (shmid == -1)
    {
        perror("Failed to get shared memory");
        exit(1);
    }

    void *shm = shmat(shmid, 0, 0);

    if (shm == (void *)-1)
    {
        perror("Failed to attach the shared memory");
        exit(1);
    }

    char **produced = shm;
    char **consumed = shm + sizeof(char *);

    *produced = shm + 2 * sizeof(char *);
    *consumed = shm + 2 * sizeof(char *) + sizeof(char);

    int semid = semget(10, 3, IPC_CREAT | perm);

    if (semid == -1)
    {
        perror("Failed to get a semaphore set");
        exit(1);
    }

    if (semctl(semid, SEMCF, SETVAL, 0) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    if (semctl(semid, SEMCE, SETVAL, N) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    if (semctl(semid, SEMMX, SETVAL, 1) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    producerInitialize(produced);

    for (int producerId = 1; producerId < 4; producerId++)
        producerCreate(producerId, produced, semid);

    for (int consumerId = 1; consumerId < 4; consumerId++)
        consumerCreate(consumerId, consumed, semid);

    for (int i = 0; i < 6; i++)
    {
        int stat;

        if (wait(&stat) == -1)
        {
            perror("Failed to wait for a child");
            exit(1);
        }

        if (!WIFEXITED(stat))
            fprintf(stderr, "A child has terminated abnormally!\n");
    }

    if (shmdt(shm) == -1)
    {
        perror("Failed to detach the shared memory");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("Failed to remove the shared memory identifier");
        exit(1);
    }

    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        perror("Failed to remove the semaphore set identifier");
        exit(1);
    }

    return 0;
}
