#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILD_COUNT 3

/*!
 * Код родительского процесса
 *
 * \param children - массив идентификаторов дочерних процессов.
 * \param count - количество дочерних процессов.
 */

void parent(const pid_t *children, size_t count)
{
    printf("I am %d; my group is %d; ",
        getpid(), getpgrp());

    if (count == 0)
        printf("I have no children.\n");

    else
    {
        printf("my children are ");

        for (size_t i = 0; i < count - 1; i++)
            printf("%d, ", children[i]);

        printf("%d.\n", children[count - 1]);

        int stat;

        for (size_t i = 0; i < count; i++)
        {
            // Ждём дочерний процесс
            pid_t child = wait(&stat);

            printf("My child %d has finished ", child);

            if (WIFEXITED(stat))
                printf("with exit code %d.\n", WEXITSTATUS(stat));

            else
                printf("abnormally.\n");
        }
    }
}

/*!
 * Код дочернего процесса
 */

void child()
{
    printf("I am %d; my group is %d; my parent is %d.\n",
        getpid(), getpgrp(), getppid());

    sleep(1);
}

int main()
{
    int children[CHILD_COUNT];

    for (size_t i = 0; i < CHILD_COUNT; i++)
    {
        if ((children[i] = fork()) == -1)
        {
            perror("Failed to fork");
            exit(1);
        }

        // Дочерний процесс
        else if (children[i] == 0)
        {
            child();
            exit(0);
        }
    }

    // Родительский процесс
    parent(children, CHILD_COUNT);

    return 0;
}
