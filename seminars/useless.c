#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char ** argv)
{
    FILE *file;

    if (argc != 2) // Ошибка ввода
    {
        printf("Error! (Few arguments)\n");
        return 1;
    }

    if ((file = fopen(argv[1], "r")) == NULL) // Ошибка чтения файла
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

        if (pid < 0) // fork возвращает -1 в случае ошибки
        {
            printf("Error! (Process can't be created)\n");
            return 3;
        }

        if (pid == 0) // fork возвращает 0 для child process
        {
            sleep(time);
            printf("\n");
            execlp(name, NULL);
            return 0;
        }
    }

    return 0;
}
