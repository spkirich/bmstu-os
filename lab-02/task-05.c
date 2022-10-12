#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

volatile int flag = 0;

void handler(int sig)
{
    flag = 1;
}

int main()
{
    pid_t child_pid[2];
    int fd[2], stat;

    signal(SIGUSR1, handler);

    if (pipe(fd) == -1)
    {
        perror("Failed to pipe");
        exit(1);
    }

    for (int i = 0; i < 2; i++)
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

        else
        {
            printf("I am %d; my group is %d; ",
                getpid(), getpgrp());

            printf("my child is %d.\n",
                child_pid[i]);
        }
    }

    if (close(fd[1]) == -1)
    {
        perror("Failed to close");
        exit(1);
    }

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

    for (int i = 0; i < 2; i++)
    {
        pid_t child = waitpid(child_pid[i], &stat, 0);

        printf("My child %d has ", child);

        if (WIFEXITED(stat))
            printf("finished normally with return code %d.\n", WEXITSTATUS(stat));

        else if (WIFSIGNALED(stat))
            printf("terminated, received signal %d.\n", WTERMSIG(stat));
        
        else if (WIFSTOPPED(stat))
            printf("stopped, received signal %d.\n", WSTOPSIG(stat));
    }

    return 0;
}
