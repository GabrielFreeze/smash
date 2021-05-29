//_______________________________Includes______________________________________
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"



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
    bool interpret_vars_assign = false;
    setbuf(stdout, NULL);



    if (error = init_vars())
    {
        handle_error();
        exit(0);
    }
    
    char* prompt;
    char line[BUFSIZE];


    prompt = node_search("PROMPT")->value;

    while (1)
    {   
        tokens = NULL;
        var_indices = NULL;

        if (read_from_file)
        {
            if(!(input = get_input_from_file(fp)))
            {
                fclose(fp);
                read_from_file = false;
                input = linenoise(prompt);
            }
        }
        else
            input = linenoise(prompt);
  
        if (!input)
        {
            error = NULL_GIVEN_ERROR;
            goto end;
        }

        interpret_vars_assign = false;



        if (input[0] != '\0' && input[0] != '/')
        {

            tokens = tokens_get(input, &token_num, &var_indices, &var_indices_len);
            if (!(tokens))
                goto end;
                
            //If the first argument contains an =, then it means the user is doing variable assignment
            //...and the tokens should not be interpreted as [cmd arg0 arg1 ...], but just as a series of variable assignments.
            if (contains_char(tokens[0],'=') != -1) 
                interpret_vars_assign = true;
            
            int i,j;
            //Loop through all tokens, performing variable expansion and assignment per token.
            
            for (i = 0, j = 0; i < token_num; i++)
            {
                //If variable expansion has to be performed on the token at i
                if ((j < var_indices_len) && (i == var_indices[j].token_index)) 
                {
                    if (error = expand_vars(tokens, var_indices, var_indices_len, j++))
                        goto end;         
                }

                //Looks for any '=' within the token and assigns accordingly.
                if ((interpret_vars_assign) && (error = assign_vars(tokens, token_num, i)))
                    goto end;         
                    
            }

            if (interpret_vars_assign)
                goto end;

            //From this comment forward, the first argument the user entered is sure to be a command, and not a variable assignment statement.
            //The first token is the command, all other subsequent tokens are arguments to the command.


            if (read_from_file && contains_word(tokens[0],"source"))
                {
                    fclose(fp);
                    read_from_file = false;
                    fprintf(stderr,"Nested source statements are not supported.\n");
                    goto end;
                }

            if (error = tokens_parse(tokens, token_num))
                goto end;
            

            end:
                handle_error();
                tokens_free(tokens,token_num);
                var_indices_free(var_indices);
                
                //If the user decides to delete the PROMPT variable, the default value should display.
                node* current_node;
                if (!(current_node = node_search("PROMPT")))
                    prompt = prompt_default;
                else
                    prompt = current_node->value;
                
        }
        free(input);
    }


    return 0;
}