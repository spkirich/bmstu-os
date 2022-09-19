#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

/*!
 * Код процесса-предка
 */

void parent(int child1_pid, int child2_pid)
{
    printf("I am %d, my group is %d, my children are %d and %d.\n",
            getpid(), getpgrp(), child1_pid, child2_pid);

    int stat_loc;

    pid_t child_pid = wait(&stat_loc);

    printf("My child %d has finished ", child_pid);

    if (WIFEXITED(stat_loc))
        printf("with code %d.\n", WEXITSTATUS(stat_loc));

    else
        printf("abnormally.\n");
}

/*!
 * Код процесса-потомка
 */

void child()
{
    printf("I am %d, my group is %d, my parent is %d.\n",
        getpid(), getpgrp(), getppid());

    execl("/bin/ls", "ls", "-al", 0);
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
