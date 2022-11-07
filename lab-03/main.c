#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <unistd.h>

void daemonize(const char *cmd)
{
    int fd[3];
    pid_t pid;

    struct rlimit rl;
    struct sigaction sa;

    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
    {
        perror("Failed to getrlimit");
        exit(1);
    }

    if ((pid = fork()) == -1)
    {
        perror("Failed to fork");
        exit(1);
    }

    else if (pid != 0)
        exit(0);

    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        perror("Failed to sigaction");
        exit(1);
    }

    if ((pid = fork()) == -1)
    {
        perror("Failed to fork");
        exit(1);
    }

    else if (pid != 0)
        exit(0);

    if (chdir("/") == -1)
    {
        perror("Failed to chdir");
        exit(1);
    }

    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for (int i = 0; i < rl.rlim_max; i++)
        close(i);

    fd[0] = open("/dev/null", O_RDWR);
    fd[1] = dup(0);
    fd[2] = dup(0);

    openlog(cmd, LOG_CONS, LOG_DAEMON);

    if (fd[0] != 0 || fd[1] != 1 || fd[2] != 2)
    {
        syslog(LOG_ERR, "Неверные дескрипторы %d, %d, %d", fd[0], fd[1], fd[2]);
        exit(1);
    }
}
