//_______________________________Includes______________________________________
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"
#include "includes/config.h"
#include "includes/linkedlist.c"


//__________________________________Variables__________________________________


//______________________________________TODO___________________________________
/*



*/

int main(int argc, char** argv)
{

    char* input;
    int token_num;
    char** tokens = NULL;
    tokenchar_pair* var_indices = NULL;
    int var_indices_len;
    byte error;


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
            byte i,j;

            for (i = 0, j = 0; i < token_num; i++)  //Loop through all tokens, performing variable expansion and assignment per token.
            {
                if ((j != var_indices_len) && (i == var_indices[j].token_index)) //If variable expansion has to be performed on the token at i
                {
                    if (error = expand_vars(tokens, var_indices, var_indices_len, j++))
                    {
                        handle_error(error);
                        continue;
                    }
                }

                if (error = assign_vars(tokens, token_num, i))
                {
                    handle_error(error);
                    continue;
                }
                    
            }



            // if (error = expand_vars(tokens, var_indices, var_indices_len))
            // {
            //     handle_error(error);
            //     continue;
            // }

            // if(error = assign_vars(tokens, token_num))
            // {
            //     printf("error\n");
            // }


            for (byte i = 0; i < token_num; i++)
                printf("%s\n",tokens[i]);
            
            free(var_indices);
            tokens_free(tokens, token_num);

        }
        free(input);
    }


    return 0;
}