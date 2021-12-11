//------------------------------------------------------------------------------
// Программа моделирует работу персонала ресторана: один человек моет посуду,
// а другой вытирует уже помытую. Времена выполнения операций мытья и вытирания
// посуды меняются в зависимости от того, что моется. Стол для вымытой, но не
// вытертой посуды имеет ограниченные размеры и вмещает N предметов независимо
// от их наименования. Значение N задается как параметр среды TABLE_LIMIT перед
// стартом процессов.
//                                 made by Daniil Dorin & Nikita Kiselev
//------------------------------------------------------------------------------
// Данная часть программы отвечает за работу мойщика посуды.
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define TABLE_LIMIT 3
#define LAST_MESSAGE 255

typedef struct {
    char dish_type[100];
    int time;
} washer_struct;

// Пользовательская структура для сообщения
typedef struct mymsgbuf
{
    long mtype;
    struct table
    {
        char dish_type[100];
        int table_capacity;
        int stop_flag;
    } table;
} mymsgbuf;

int main()
{
    int msqid; // IPC-дескриптор для очереди сообщений
    char pathname[] = "dishes.dat"; // имя файла, использующееся для генерации ключа
    key_t key; // IPC-ключ
    char buf[100]; // буфер для чтения файла
    washer_struct washer_features[10]; // массив данных о времени работы мойщика
    int i = 0, wash_time, len, dish_number;
    char dish_type[100], search_type[100];

    mymsgbuf mybuf1, mybuf2;

    mybuf1.mtype = 1;
    int maxlen = sizeof(struct table);

    // Генерируем IPC-ключ
    if ((key = ftok(pathname, 0)) < 0)
    {
        printf("Error! (Can't generate key)\n");
        exit(-1);
    }

    // Пытаемся получить доступ по ключу к очереди сообщений, если она
    // существует, или создать ее, с правами доступа read & write для
    // всех пользователей
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) < 0)
    {
        printf("Error! (Can't get msqid)\n");
        exit(-1);
    }

    // открываем файлы "washer.dat", "dishes.dat"
    FILE *washer, *dishes;

    if ((washer = fopen("washer.dat", "r")) == NULL)
    {
        printf("Error! (washer.dat can't be opened)\n");
        exit(-1);
    }

    if ((dishes = fopen("dishes.dat", "r")) == NULL)
    {
        printf("Error! (dishes.dat can't be opened)\n");
        exit(-1);
    }

    while (fgets(buf, sizeof(buf), washer) != NULL)
    {
        sscanf(buf, "%s : %d", washer_features[i].dish_type, &washer_features[i].time);
        i++;
    }

    int count = i; // количество видов посуды, с которыми умеет работать мойщик

    while(fscanf(dishes, "%s : %d", dish_type, &dish_number) && (!feof(dishes)))
    {
        strcpy(search_type, dish_type);

        for (i = 0; i < count; i++)
            if (strcmp(washer_features[i].dish_type, search_type))
                wash_time = washer_features[i].time;

        printf("Washer: I'm washing %s in the amount of %d.\n", dish_type, dish_number);

        while (dish_number)
        {
			strcpy(mybuf1.table.dish_type, dish_type);
			len = strlen(mybuf1.table.dish_type) + 1;
            sleep(wash_time);
			dish_number--;
            printf("Washer: It's ready.\n");

            // проверяем, есть ли на столе место
            if (msgrcv(msqid, (mymsgbuf*) &mybuf2, maxlen, 2, 0) < 0)
            {
				printf("Can't receive message from queue\n");
				exit(-1);
			}

            // кладем один предмет на стол
			if (msgsnd(msqid, (mymsgbuf*) &mybuf1, len, 0) < 0)
            {
				printf("Can't send message to queue\n");
				msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
				exit(-1);
			}
        }
    }

    // Генерируем сообщение об окончании посуды
    mybuf1.mtype = 1;
	mybuf1.table.stop_flag = LAST_MESSAGE;
	len = sizeof(struct table);

	if (msgsnd(msqid, (mymsgbuf*) &mybuf1, len, 0) < 0)
    {
		printf("Can't send message to queue\n");
		msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
		exit(-1);
	}

	fclose(dishes);
	fclose(washer);
	return 0;
}
