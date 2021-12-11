#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // основные функции типа fork, sleep

int main(int argc, char ** argv)
{
    FILE* file;

    if (argc != 2) // ошибка количества аргументов
    {
        printf("Error! (Wrong count of arguments)\n");
        return 1;
    }

    if ((file = fopen(argv[1], "r")) == NULL) // ошибка чтения файла
    {
        printf("Error! (File can't be opened)\n");
        return 2;
    }

    char name[100];
    int time;
    pid_t pid;

    while ((fscanf(file, "%d %s", &time, name)) != EOF)
    {
        pid = fork();

        if (pid < 0) // ошибка создания процесса
        {
            printf("Error! (Process can't be created)\n");
            return 3;
        }

        if (pid == 0) // child process
        {
            sleep(time);
            printf("\n");
            execlp(name, name, NULL);
            return 0;
        }
    }

    return 0;
}
