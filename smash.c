//_______________________________Includes______________________________________
#include <stdio.h>
#include <string.h>
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"
#include "includes/config.h"

//__________________________________Variables__________________________________


//______________________________________TODO___________________________________
/*
  Function that frees tokens if tokens_get fails during process.  



*/

int main(int argc, char** argv)
{

    char* input;
    int token_num, error;
    char** tokens;
    int* variable_indices;


    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {


            if ((tokens = tokens_get(input, &token_num, &error, &variable_indices)) == NULL)
            {
                handle_error(error);
                continue;     
                
            }



            printf("Number of tokens: %d\n",token_num);
            for(int i = 0; i < token_num; i++)
            {
                printf("%s\n",tokens[i]);
                // printf("%d\n",variable_indices[i]);
                free(tokens[i]);

            }


        }
        free(input);
    }


    return 0;
}