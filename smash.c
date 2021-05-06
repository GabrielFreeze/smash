//_______________________________Includes______________________________________
#include <stdio.h>
#include <string.h>
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"
#include "includes/config.h"


//__________________________________Variables__________________________________


//______________________________________TODO___________________________________
/*



*/

int main(int argc, char** argv)
{

    char* input;
    int token_num, error;
    char** tokens;
    int* var_indices;
    int var_indices_length;

    

    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {
            

            if ((tokens = tokens_get(input, &token_num, &error, &var_indices, &var_indices_length)) == NULL)
            {
                handle_error(error);
                continue;     
                
            }

            printf("Number of tokens: %d\n",token_num);

            for(int i = 0; i < var_indices_length; i++)
                printf("%d\n",var_indices[i]);
                
            free(var_indices);
            tokens_free(tokens, token_num);

        }
        free(input);
    }


    return 0;
}