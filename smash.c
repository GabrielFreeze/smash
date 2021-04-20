#include <stdio.h>
#include <string.h>
#include "includes/linenoise-master/linenoise.c"

int main(int argc, char* argv)
{
    printf("Hello World!!!!!\n");

    char* input;
    char* prompt = {"init> "};
    char* exit_keyword = {"exit"};

    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {
            printf("%s\n",input);
        }

        free(input);
    }


    return 0;
}