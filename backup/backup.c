#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // основные функции типа fork
#include <dirent.h> // работа с директориями
#include <time.h> // библиотека для waitpid
#include <sys/types.h> // различные типы данных
#include <sys/stat.h> // информация о файлах

/* Создает копию файла и сжимает ее */
int copyfile(char* from_address, char* to_address)
{
    FILE* file;

    if ((file = fopen(from_address, "r")) == NULL) // ошибка чтения исходного файла
    {
        printf("Error! (File can't be opened: %s)\n", from_address);
        exit(-1);
    }

    char* to_gz = (char*)calloc(1, 100);
    strcpy(to_gz, to_address);
    strcat(to_gz, ".gz");

    if ((file = fopen(to_gz, "r")) == NULL) // сжатой копии файла еще не существует
    {
        pid_t pid;
        pid = fork();

        if (pid == 0) // child process
        {
            printf("Creating copy of file: %s\n", to_address);
            execlp("cp", "cp", from_address, to_address, NULL);
        }

        waitpid(pid);
        pid = fork();

        if (pid == 0) // child process x2
            execlp("gzip", "gzip", to_address, NULL);

        return 0;
    }

    /* буфер для инфорфмации об исходном файле */
    struct stat* buf = (struct stat*)malloc(sizeof(struct stat));

    /* буфер для информации о сжатой копии файла */
    struct stat* bufgz = (struct stat*)malloc(sizeof(struct stat));

    stat(from_address, buf); // получаем информацию об исходном файле
    time_t fl_time = buf->st_mtime; // время последней модификации
    stat(to_gz, bufgz); // получаем информацию о сжатой копии файла
    time_t gz_time = bufgz->st_ctime; // время последнего изменения (создания)

    free(buf);
    free(bufgz);

    if (gz_time < fl_time) // файл был модифицирован с момента последнего backup'а
    {
        pid_t pid;
        pid = fork();

        if (pid == 0) // child process
            execlp("rm", "rm", to_gz, NULL);

        waitpid(pid);
        pid = fork();

        if (pid == 0) // child process x2
        {
            printf("Creating copy of modified file: %s\n", to_address);
            execlp("cp", "cp", from_address, to_address, NULL);
        }

        waitpid(pid);
        pid = fork();

        if (pid == 0) // child process x3
            execlp("gzip", "gzip", to_address, NULL);

        waitpid(pid);

        return 0;
    }

    free(to_gz);

    return 0;
}

int back_up(char* from_dir, char* to_dir)
{
    DIR* dir;
    struct dirent* file;

    if ((dir = opendir(from_dir)) == NULL) // исходная директория не может быть открыт
    {
        printf("Error! (Source directory can't be opened: %s)\n", from_dir);
        exit(1);
    }

    while ((file = readdir(dir)) != NULL) // читаем файлы из директории
    {
        char* from_address = (char*)calloc(1, 100);
        char* to_address = (char*)calloc(1, 100);

        strcpy(from_address, from_dir);
        strcat(from_address, "/");
        strcat(from_address, file->d_name); // добавляем имя файла

        strcpy(to_address, to_dir);
        strcat(to_address, "/");
        strcat(to_address, file->d_name); // добавляем имя файла

        /* буфер для информации об исходном файле */
        struct stat* buf = (struct stat*)malloc(sizeof(struct stat));
        stat(from_address, buf); // получаем информацию об исходном файле

        if (strcmp(file->d_name, ".") && strcmp(file->d_name, "..")) // файл имеет допустимое имя
        {
            if (S_ISDIR(buf->st_mode)) // файл является директорией
            {
                if (opendir(to_address) == NULL) // копии директории не существует
                {
                    pid_t pid;
                    pid = fork();

                    if (pid == 0) // child process
                        execlp("mkdir", "mkdir", to_address, NULL); // создаем директорию

                    waitpid(pid);
                }

                back_up(from_address, to_address); // рекурсивно делаем backup каталога
            }

            if (S_ISREG(buf->st_mode)) // обычный файл
                copyfile(from_address, to_address); // делаем backup файла

            free(buf);
            free(from_address);
            free(to_address);
        }
    }

    closedir(dir);
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("Error! (Wrong count of arguments)\n");
        return 1;
    }

    if (opendir(argv[1]) == NULL)
    {
        printf("Error! (Source directory can't be opened: %s)\n", argv[1]);
        return 2;
    }

    if (opendir(argv[2]) == NULL)
    {
        pid_t pid;
        pid = fork();

        if (pid == 0)
            execlp("mkdir", "mkdir", argv[2], NULL);

        waitpid(pid);
        printf("Copy of source directory was created\n");
    }

    back_up(argv[1], argv[2]);

    return 0;
}
