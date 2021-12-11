#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	printf("%d, %d", getuid(), getgid());

	return 0;
}
