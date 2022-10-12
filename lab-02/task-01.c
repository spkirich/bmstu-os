#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    pid_t child_pid[2];

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

            sleep(1);

            printf("I am %d; my group is %d; my parent is %d.\n",
                getpid(), getpgrp(), getppid());

            return 0;
        }

        else
        {
            printf("I am %d; my group is %d; ",
                getpid(), getpgrp());

            printf("my child is %d\n",
                child_pid[i]);
        }
    }

    return 0;
}
