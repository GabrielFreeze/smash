//_______________________________Includes______________________________________
#include <stdio.h>
#include <string.h>
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"

//__________________________________Variables__________________________________
char* input;
char* prompt = {"init> "};
char* exit_keyword = {"exit"};
char* metacharacters = {" |;<>\t"};

//______________________________________TODO___________________________________
/*
    1. Implement a function that returns a pointer to an array of tokens. Make sure
    you keep in mind special parse symbols as mentioned in brief.

    int length = get_tokens(args, input);

    if (length == -1)
        *error*





*/

int main(int argc, char** argv)
{
    char** args;

    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {

            int token_num = tokenlen(input);
            int index = 0;
            args = (char**) malloc(token_num * sizeof(char*));

            for (char* token = strtok(input, metacharacters); token != NULL; token = strtok(NULL, metacharacters))
            {
                args[index] = (char*) malloc(50);
                strcpy(args[index], token);
                index++;
            }


            for(int i = 0; i < token_num; i++)
            {
                printf("%s\n",args[i]);
                free(args[i]);
            }



        }
        free(args);
        free(input);
    }


    return 0;
}