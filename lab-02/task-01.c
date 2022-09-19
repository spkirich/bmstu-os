#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>

/*!
 * Код процесса-предка
 */

void parent(int child1_pid, int child2_pid)
{
    while (1)
        printf("I am %d, my group is %d, my children are %d and %d.\n",
            getpid(), getpgrp(), child1_pid, child2_pid);
}

/*!
 * Код процесса-потомка
 */

void child()
{
    while (1)
        printf("I am %d, my group is %d, my parent is %d.\n",
            getpid(), getpgrp(), getppid());
}

int main()
{
    int child1_pid;

    if ((child1_pid = fork()) == -1)
    {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }

    else if (child1_pid == 0)
        child();

    else
    {
        int child2_pid;

        if ((child2_pid = fork()) == -1)
        {
            perror("Failed to fork!");
            exit(EXIT_FAILURE);
        }

        else if (child2_pid == 0)
            child();

        else
            parent(child1_pid, child2_pid);
    }

    return 0;
}
