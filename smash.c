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


    signal(SIGINT, sigint_handler);

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
        redirect_token_index = -2;
        redirect_char_index = -2;

        if (input[0] != '\0' && input[0] != '/')
        {
            if (!(tokens = tokens_get(input, &token_num, &var_indices, &var_indices_len)))
                goto end;
            
            //For every redirect save the filename corresponding to it.
            //Note: The last instance of a redirect has priority over its predeccessors. 
            for (int i = 0; i < redirect_count; i++) 
            {
                if (error = redirect(tokens, redirect_array[i], i))
                    goto end;
            }

            //The filenames for redirects should not be treated as additional arguments.
            //They served their purpose in specifiying the files for input and output, and hence are no longer needed.
            if (redirect_count) 
                tokens[token_num = redirect_start_index] = NULL;

            //If the first argument contains an =, then it means the user is doing variable assignment
            //...and the tokens should not be interpreted as [cmd arg0 arg1 ...], but just as a series of variable assignments.
            if (contains_char(tokens[0],'=') != -1) 
                interpret_vars_assign = true;
            
            //Loop through all tokens, performing variable expansion and assignment per token.
            
            int i,j;
            for (i = 0, j = 0; i < token_num; i++)
            {
                //Perform variable expansion, if applicable
                if ((j < var_indices_len) && (i == var_indices[j].token_index) && (error = expand_vars(tokens, var_indices, var_indices_len, j++))) 
                    goto end;         

                //Looks for any '=' within the token and assigns accordingly, if applicable
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
                reset_redirect();
                
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