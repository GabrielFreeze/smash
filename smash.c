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
    int error;
    bool try_again = false;
    bool interpret_vars_assign = false;


    if (error = init_vars())
    {
        handle_error(error);
        exit(0);
    }


    while (((input = linenoise(prompt)) != NULL))
    {
        if (input[0] != '\0' && input[0] != '/')
        {
            tokens = tokens_get(input, &token_num, &error, &var_indices, &var_indices_len);
            if (error)
                goto end;    
                
            if (contains_char(tokens[0],'=') != -1) //If the first argument contains an =, then it means the user is doing variable assignment, and the tokens should not be interpreted as [cmd arg0 arg1 ...]
                interpret_vars_assign = true;
            

            printf("Number of tokens: %d\n",token_num);

            int i,j;
            for (i = 0, j = 0; i < token_num; i++)  //Loop through all tokens, performing variable expansion and assignment per token.
            {
                if ((j != var_indices_len) && (i == var_indices[j].token_index)) //If variable expansion has to be performed on the token at i
                {
                    if (error = expand_vars(tokens, var_indices, var_indices_len, j++))
                        goto end;
                          
                }

                
                if ((interpret_vars_assign) && (error = assign_vars(tokens, token_num, i)))
                    goto end;
                
                    
            }

            if (interpret_vars_assign)
                goto end;

            //From this comment forward, the first argument the user entered is sure to be a command, and not a variable assignment statement

            //The first token is the command, all other subsequent tokens are arguments to the command

            //Function that parses tokens. 

            tokens_parse(tokens, token_num);
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            for (int i = 0; i < token_num; i++)
                printf("%s\n",tokens[i]);
            
            end:
                handle_error(error);
                tokens_free(tokens,token_num);
                var_indices_free(var_indices);

        }
        free(input);
    }


    return 0;
}