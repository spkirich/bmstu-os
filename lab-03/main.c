#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <unistd.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask;

extern int lockfile(int);

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

int already_running()
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    if (fd == -1)
    {
        syslog(LOG_ERR, "failed to open %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    if (lockfile(fd) == -1)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            return 1;
        }

        syslog(LOG_ERR, "failed to lock %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }

    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return 0;
}

void reread()
{
    // TODO
}

void *thr_fn(void *arg)
{
    int err, signo;

    for (;;)
    {
        err = sigwait(&mask, &signo);

        if (err == -1)
        {
            syslog(LOG_ERR, "failed to sigwait");
            exit(1);
        }

        switch (signo)
        {
        case SIGHUP:
            syslog(LOG_INFO, "reading config file");
            reread();
            break;

        case SIGTERM:
            syslog(LOG_INFO, "terminating");
            exit(0);

        default:
            syslog(LOG_INFO, "caught unexpected signal %d", signo);
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;

    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];

    else
        cmd++;

    daemonize(cmd);

    if (already_running())
    {
        syslog(LOG_ERR, "daemon is already running");
        exit(1);
    }

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        perror("failed to sigaction");
        exit(1);
    }

    sigfillset(&mask);

    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) == -1)
    {
        perror("failed to pthread_sigmask");
        exit(1);
    }

    err = pthread_create(&tid, NULL, thr_fn, 0);

    if (err == -1)
    {
        perror("failed to pthread_create");
        exit(1);
    }

    // TODO

    exit(0);
}
