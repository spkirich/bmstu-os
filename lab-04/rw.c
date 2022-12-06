#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SEMRE 0
#define SEMRC 1
#define SEMWE 2
#define SEMWC 3
#define SEMMX 4

struct sembuf readerStart[5] = {
    {SEMWE, 0, 0}, {SEMRE, +1, 0}, {SEMWC, 0, 0}, {SEMRC, +1, 0}, {SEMRE, -1, 0},
};

struct sembuf readerStop[1] = {
    {SEMRC, -1, 0},
};

struct sembuf writerStart[6] = {
    {SEMWC, +1, 0}, {SEMRC, 0, 0}, {SEMWE, 0, 0}, {SEMWE, +1, 0}, {SEMWC, -1, 0}, {SEMMX, -1, 0},
};

struct sembuf writerStop[2] = {
    {SEMWE, -1, 0},
    {SEMMX, +1, 0},
};

void readerRun(int readerId, int *shm, int semid)
{
    srand(time(NULL) + readerId);

    for (int i = 0; i < 10; i++)
    {
        sleep(rand() % 7 + 1);

        if (semop(semid, readerStart, 5) == -1)
        {
            perror("Reader failed to start");
            exit(1);
        }

        printf("Reader #%d got: %2d\n", readerId, *shm);

        if (semop(semid, readerStop, 1) == -1)
        {
            perror("Reader failed to stop");
            exit(1);
        }
    }
}

void writerRun(int writerId, int *shm, int semid)
{
    srand(time(NULL) + writerId);

    for (int i = 0; i < 10; i++)
    {
        sleep(rand() % 3 + 1);

        if (semop(semid, writerStart, 6) == -1)
        {
            perror("Writer failed to start");
            exit(1);
        }

        printf("Writer #%d put: %2d\n", writerId, ++(*shm));

        if (semop(semid, writerStop, 2) == -1)
        {
            perror("Writer failed to stop");
            exit(1);
        }
    }
}

int main()
{
    int perm = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | perm);

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

    int semid = semget(IPC_PRIVATE, 5, IPC_CREAT | perm);

    if (semid == -1)
    {
        perror("Failed to get a semaphore set");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, 0) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    if (semctl(semid, 1, SETVAL, 0) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    if (semctl(semid, 2, SETVAL, 0) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    if (semctl(semid, 3, SETVAL, 0) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    if (semctl(semid, 4, SETVAL, 1) == -1)
    {
        perror("Failed to set the semaphore value");
        exit(1);
    }

    for (int writerId = 1; writerId < 4; writerId++)
    {
        int writerPid;

        if ((writerPid = fork()) == -1)
        {
            perror("Writer failed to fork");
            exit(1);
        }

        else if (writerPid == 0)
        {
            writerRun(writerId, shm, semid);
            return 0;
        }
    }

    for (int readerId = 1; readerId < 5; readerId++)
    {
        int readerPid;

        if ((readerPid = fork()) == -1)
        {
            perror("Reader failed to fork");
            exit(1);
        }

        else if (readerPid == 0)
        {
            readerRun(readerId, shm, semid);
            return 0;
        }
    }

    for (int i = 0; i < 7; i++)
    {
        int stat;

        if (wait(&stat) == -1)
        {
            perror("Failed to wait");
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
