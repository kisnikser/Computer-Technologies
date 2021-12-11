#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

void daemon_process()
{
    int i;

    fprintf(stderr, "Daemon started!\n");
    /* Используем для тестирования конечный цикл */
    for (i = 0; i < 10; i++)
    {
        fprintf(stderr, "Hello %d!\n", i);
        sleep(2);
    }
}

int main()
{
    pid_t pid;

    /* Клонировать себя */
    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "Cannot fork!\n");
        return 1;
    }
    else if (pid != 0)
    {
        /*Завершить родительский процесс*/
        return 0;
    }

    /* Процесс-потомок продолжает работу */
    /* Клонировать себя */
    pid = fork();
    if (pid == -1)
    {
        fprintf(stderr, "Cannot fork!\n");
        return 1;
    }
    else if (pid != 0)
    {
        /* Завершить родительский процесс */
        return 0;
    }

    /* Процесс-потомок продолжает работу */
    daemon_process();

    return 0;
}
