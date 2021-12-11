#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Error! (Wrong count of arguments)\n");
        return 1;
    }

    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    char buf[512];
    mydir = opendir(argv[1]);

    while ((myfile = readdir(mydir)) != NULL)
    {
        sprintf(buf, "%s/%s", argv[1], myfile->d_name);
        stat(buf, &mystat);
        printf("%zu %s\n", mystat.st_size, myfile->d_name);
    }

    closedir(mydir);

    return 0;
}
