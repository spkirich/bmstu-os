#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

volatile int flag = 0;

void handler(int sig)
{
    flag = 1;
}

int main()
{
    int child_pid[2], fd[2];

    signal(SIGUSR1, handler);

    if (pipe(fd) == -1)
    {
        perror("Failed to pipe");
        exit(1);
    }

    for (size_t i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Failed to fork");
            exit(1);
        }

        else if (child_pid[i] == 0)
        {
            if (close(fd[0]) == -1)
            {
                perror("Failed to close");
                exit(1);
            }

            printf("I am %d; my group is %d; my parent is %d.\n",
                getpid(), getpgrp(), getppid());

            while (!flag);

            switch (i)
            {
                case 0:
                {
                    char message[] = "Hello world!";

                    if (write(fd[1], message, sizeof(message)) == -1)
                    {
                        perror("Failed to write");
                        exit(1);
                    }

                    return 0;
                }

                case 1:
                {
                    char message[] = "Terve maailma!";

                    if (write(fd[1], message, sizeof(message)) == -1)
                    {
                        perror("Failed to write");
                        exit(1);
                    }

                    return 0;
                }
            }
        }
    }

    if (close(fd[1]) == -1)
    {
        perror("Failed to close");
        exit(1);
    }

    printf("I am %d; my group is %d; ",
        getpid(), getpgrp());

    printf("my children are %d and %d\n",
        child_pid[0], child_pid[1]);

    char message[16];

    if (read(fd[0], message, sizeof(message)) == -1)
    {
        perror("Failed to read");
        exit(1);
    }

    printf("%s\n", message);

    if (read(fd[0], message, sizeof(message)) == -1)
    {
        perror("Failed to read");
        exit(1);
    }

    printf("%s\n", message);

    int stat;

    for (size_t i = 0; i < 2; i++)
    {
        pid_t child = wait(&stat);

        printf("My child %d has finished ", child);

        if (WIFEXITED(stat))
            printf("with return code %d.\n", WEXITSTATUS(stat));

        else
            printf("abnormally.\n");
    }

    return 0;
}
