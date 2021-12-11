#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

int main()
{
    int flags, fd[2];

    if (pipe(fd) < 0)
    {
        printf("Can\'t create pipe\n");
        exit(-1);
    }

    if ((flags = fcntl(fd[1], F_GETFL)) < 0)
        exit(-1);

    flags |= O_NONBLOCK;

    if (fcntl(fd[1], F_SETFL, flags))
        exit(-1);

    int i = 0;

    while (write(fd[1], "1", 1) >= 0)
        i++;

    if (errno == EAGAIN)
        printf("pipe size = %d KB\n", i / 1024);

    close(fd[0]);
    close(fd[1]);

    return 0;
}
