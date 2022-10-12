#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t child_pid[2];
    int stat;

    for (int i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Failed to fork");
            exit(1);
        }

        else if (child_pid[i] == 0)
        {
            printf("I am %d; my group is %d; my parent is %d.\n",
                getpid(), getpgrp(), getppid());

            return 0;
        }

        else
        {
            printf("I am %d; my group is %d; ",
                getpid(), getpgrp());

            printf("my child is %d.\n",
                child_pid[i]);
        }
    }

    for (int i = 0; i < 2; i++)
    {
        pid_t child = wait(&stat);

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
