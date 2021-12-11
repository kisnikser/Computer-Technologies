#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char ** argv)
{
    DIR * dfd;
    struct dirent * dp;
    char filename[NAME_MAX];

    if (argc < 2)
    {
        printf("Error, no input DIR!\n");
        exit(-1);
    }
    else
    {
        strcpy(filename, argv[1]);
        printf("%s\n\n", filename);
        dfd = opendir(filename);

        while ((dp = readdir(dfd)) != NULL)
            printf("%s\n", dp->d_name);

        closedir(dfd);
        return 0;
    }
}
