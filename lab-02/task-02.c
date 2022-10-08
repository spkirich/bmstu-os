#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    int child_pid[2];

    for (size_t i = 0; i < 2; i++)
    {
        if ((child_pid[i] = fork()) == -1)
        {
            perror("Failed to fork");
            return 1;
        }

        else if (child_pid[i] == 0)
        {
            printf("I am %d; my group is %d; my parent is %d.\n",
                getpid(), getpgrp(), getppid());

            sleep(1);

            return 0;
        }
    }

    printf("I am %d; my group is %d; ",
        getpid(), getpgrp());

    printf("my children are %d and %d\n",
        child_pid[0], child_pid[1]);

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
