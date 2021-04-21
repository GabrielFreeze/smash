#include <stdio.h>
#include <string.h>
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"

    char* input;
    char* prompt = {"init> "};
    char* exit_keyword = {"exit"};
    char* delimiters = {" |;<>\t"};

int main(int argc, char** argv)
{
    int count = 0;

    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {
            printf("Predicted Token Count: %d\n",tokenlen(input));

            for (char* token = strtok(input, delimiters); token != NULL; token = strtok(NULL, delimiters))
            {
                count ++;
            }

            printf("Actual Token Count: %d\n",count);
        }
        count = 0;
        free(input);
    }


    return 0;
}