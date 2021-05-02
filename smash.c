//_______________________________Includes______________________________________
#include <stdio.h>
#include <string.h>
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"
#include "includes/config.h"

//__________________________________Variables__________________________________


//______________________________________TODO___________________________________
/*
    Implement a function that frees token vector. (Maybe)
    

    




*/

int main(int argc, char** argv)
{

    char* input;
    int token_num, error;
    char** tokens;


    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {


            if ((tokens = tokens_get(input, &token_num, &error)) == NULL)
            {
                if (error == MEMORY_ERROR)
                    fprintf(stderr, "Error while allocating memory.\n");

                if (error == BUFFER_OVERFLOW_ERROR)
                    fprintf(stderr, "Did a token exceed its buffer size of %d.\n",TOKEN_SIZE);

                if (error == LENGTH_ERROR)
                    fprintf(stderr, "Invalid Input. Do you have an escape character alone?\n");
                continue;
            }
            printf("Number of tokens: %d\n",token_num);
            for(int i = 0; i < token_num; i++)
            {
                printf("%s\n",tokens[i]);
                free(tokens[i]);

            }


        }
        free(input);
    }


    return 0;
}