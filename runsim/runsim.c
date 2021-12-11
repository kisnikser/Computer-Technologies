//------------------------------------------------------------------------------
// Программа осуществляет контроль количества одновременно работающих
// UNIX-приложений. Читает UNIX-команду со стандартного ввода и запускает ее
// на выполнение. Количество одновременно работающих команд не должно
// превышать N, где N - параметр командной строки при запуске runsim.
// При попытке запустить более чем N приложений выводится сообщение об ошибке.
// runsim прекращает свою работу при возникновении признака конца файла на
// стандартном вводе.
//                                 made by Daniil Dorin & Nikita Kiselev
//------------------------------------------------------------------------------
// Для успешной компиляции необходимо дописать -lpthread
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

int n = 0; // счетчик количества одновременно работающих программ
pthread_mutex_t mutex; // глобальный объект мьютекс, доступный всем потокам

void* run_command(void* command) // исполнение команды
{
    pthread_mutex_lock(&mutex); // получаем доступ к mutex
    n++;
    pthread_mutex_unlock(&mutex); // выходим из критической секции

    system(command); // выполняем команду
    free(command); // освобождаем динамическую память

    pthread_mutex_lock(&mutex);
    n--;
    pthread_mutex_unlock(&mutex);

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Error! (Wrong count of arguments)\n");
        return 1;
    }

    int N; // максимальное число одновременно работающих UNIX-приложений
    N = atoi(argv[1]);
    //printf("%d\n", N);

    if (errno)
    {
        printf("Error! (Bad arguments)\n");
        return 2;
    }

    pthread_mutex_init(&mutex, NULL); // инициализируем mutex

    while(!feof(stdin)) // пока не достигнут конец файла на стандартном вводе
    {
        char* command = malloc(80); // выделение динамической памяти для команды

        if (!fgets(command, 80, stdin))
        {
            printf("\nBad command: %s\n", command);
            continue;
        }

        // получаем текущее значение количества работающих приложений
        pthread_mutex_lock(&mutex);
        int n_current = n;
        pthread_mutex_unlock(&mutex);

        if (n_current == N) // сравниваем текущее число с максимальным
        {
            printf("The maximum number of commands has been reached\n");
            free(command);
        }

        pthread_t thread; // объявляем новый поток
        pthread_create(&thread, 0, run_command, command); // создаем новый поток
    }

    pthread_mutex_destroy(&mutex); // уничтожаем mutex
    return 0;
}
