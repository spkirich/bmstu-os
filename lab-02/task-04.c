#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CHILD_COUNT 8

/*!
 * Вывести сообщение из канала в терминал
 *
 * \param fd - дескриптор программного канала.
 */

void echo(int fd)
{
    char buf;

    while (1)
    {
        // Читаем из канала
        int count = read(fd, &buf, 1);

        if (count == -1)
        {
            perror("Failed to read");
            exit(1);
        }

        else if (count == 1)
        {
            if (buf)
                putchar(buf);

            else
            {
                putchar('\n');
                break;
            }
        }

        else break;
    }
}

/*!
 * Обработать завершение дочернего процесса
 *
 * \param p_child - указатель на идентификатор дочернего процесса.
 */

void join(int *p_child)
{
    int stat;

    // Ждём дочерний процесс
    pid_t child = waitpid(*p_child, &stat, WNOHANG);

    if (child == -1)
    {
        perror("Failed to wait");
        exit(1);
    }

    if (child == *p_child)
    {
        printf("My child %d has finished ", child);

        if (WIFEXITED(stat))
            printf("with exit code %d.\n", WEXITSTATUS(stat));

        else
            printf("abnormally.\n");

        *p_child = 0;
    }
}

/*!
 * Код родительского процесса
 *
 * \param children - массив идентификаторов дочерних процессов.
 * \param count - количество дочерних процессов.
 * \param fd - дескриптор программного канала.
 */

void parent(pid_t *children, size_t count, int fd)
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

        int done;

        do
        {
            echo(fd);
            done = 1;

            for (size_t i = 0; i < count; i++)
            {
                if (children[i])
                {
                    join(&children[i]);
                    done = 0;
                }
            }
        }
        while (!done);
    }
}

/*!
 * Код дочернего процесса
 *
 * \param fd - дескриптор программного канала.
 * \param message - содержимое сообщения.
 */

void child(int fd, const char *message)
{
    printf("I am %d; my group is %d; my parent is %d.\n",
        getpid(), getpgrp(), getppid());

    // Длина сообщения
    size_t len = strlen(message) + 1;

    if (write(fd, message, len) != len)
    {
        perror("Failed to write");
        exit(1);
    }
}

int main(int argc, const char **argv)
{
    if (argc > MAX_CHILD_COUNT)
    {
        fprintf(stderr, "Too many arguments!\n");
        exit(1);
    }

    int children[MAX_CHILD_COUNT], fd[2];

    pipe(fd);

    for (size_t i = 0; i < argc - 1; i++)
    {
        if ((children[i] = fork()) == -1)
        {
            perror("Failed to fork");
            exit(1);
        }

        else if (children[i] == 0)
        {
            close(fd[0]);

            // Дочерний процесс
            child(fd[1], argv[i + 1]);

            exit(0);
        }
    }

    close(fd[1]);

    // Родительский процесс
    parent(children, argc - 1, fd[0]);

    return 0;
}
