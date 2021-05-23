#include <stdio.h>
#include <unistd.h>

int main()
{
    char shell[1000];

    if(readlink("/proc/self/exe", shell, 1000) == -1)
        return -1;

    printf("Hello: %s\n",shell);
    return 0;
}
