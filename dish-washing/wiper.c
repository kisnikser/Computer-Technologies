//------------------------------------------------------------------------------
// Программа моделирует работу персонала ресторана: один человек моет посуду,
// а другой вытирует уже помытую. Времена выполнения операций мытья и вытирания
// посуды меняются в зависимости от того, что моется. Стол для вымытой, но не
// вытертой посуды имеет ограниченные размеры и вмещает N предметов независимо
// от их наименования. Значение N задается как параметр среды TABLE_LIMIT перед
// стартом процессов.
//                                 made by Daniil Dorin & Nikita Kiselev
//------------------------------------------------------------------------------
// Данная часть программы отвечает за работу вытиральщика посуды.
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
} wiper_struct;

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
    wiper_struct wiper_features[10]; // массив данных о времени работы мойщика
    int i = 0, wipe_time, dish_number, len, mslen;
    char search_type[100];

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

    // открываем файлы, "wiper.dat", "dishes.dat"
    FILE *wiper, *dishes;

    if ((wiper = fopen("wiper.dat", "r")) == NULL)
    {
        printf("Error! (wiper.dat can't be opened)\n");
        exit(-1);
    }

    if ((dishes = fopen("dishes.dat", "r")) == NULL)
    {
        printf("Error! (dishes.dat can't be opened)\n");
        exit(-1);
    }

    while (fgets(buf, sizeof(buf), wiper) != NULL)
    {
        sscanf(buf, "%s : %d", wiper_features[i].dish_type, &wiper_features[i].time);
        i++;
    }

    int count = i;

    mymsgbuf mybuf1, mybuf2;

    int maxlen = sizeof(struct table);
    len = sizeof(int);
    mybuf2.mtype = 2;
    mybuf2.table.table_capacity = 1;

    for (int i = 0; i < TABLE_LIMIT; i++)
    {
		if (msgsnd(msqid, (mymsgbuf*) &mybuf2, len, 0) < 0)
        {
			printf("Can't send message to queue\n");
			msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
			exit(-1);
		}
	}

    while (1)
    {
		if (mslen = msgrcv(msqid, (mymsgbuf*) &mybuf1, maxlen, 1, 0) < 0)
        {
			printf("Can't receive message from queue\n");
			exit(-1);
		}

        if (mybuf1.table.stop_flag == LAST_MESSAGE)
        {
			printf("\nThe work is done.\n");
			msgctl(msqid, IPC_RMID,(struct msqid_ds *) NULL);
			break;
		}

        strcpy(search_type, mybuf1.table.dish_type);

        for (i = 0; i < count; i++)
            if (strcmp(wiper_features[i].dish_type, search_type))
                wipe_time = wiper_features[i].time;

		printf("Wiper: I'm wiping one %s.\n", mybuf1.table.dish_type);

		if (msgsnd(msqid, (mymsgbuf*) &mybuf2, len, 0) < 0)
        {
			printf("Can't send message to queue\n");
			msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
			exit(-1);
		}

		sleep(wipe_time);
		printf("Wiper: It's ready.\n");
	}

	fclose(dishes);
	fclose(wiper);
	return 0;
}
