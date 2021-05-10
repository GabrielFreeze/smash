//_______________________________Includes______________________________________
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
    char** tokens = NULL;
    tokenchar_pair* var_indices = NULL;
    int var_indices_len;

    if (error = init_vars())
    {
        handle_error(error);
        exit(0);
    }


    while (((input = linenoise(prompt)) != NULL) && strcmp(input, exit_keyword))
    {
        if (input[0] != '\0' && input[0] != '/')
        {


            if ((tokens = tokens_get(input, &token_num, &error, &var_indices, &var_indices_len)) == NULL)
            {
                handle_error(error);
                continue;     
                
            }

            printf("Number of tokens: %d\n",token_num);


            // for (byte i = 0; i < var_indices_len; i++)
            // {
            //     printf("%d\n",var_indices[i].token_index);
            //     printf("%d\n",var_indices[i].char_index);
            // }
                


            if (error = expand_vars(tokens, var_indices, var_indices_len))
            {
                handle_error(error);
                continue;
            }

            for (byte i = 0; i < token_num; i++)
                printf("%s\n",tokens[i]);
            
            free(var_indices);
            tokens_free(tokens, token_num);

        }
        free(input);
    }


    return 0;
}