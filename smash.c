//_______________________________Includes______________________________________
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"


int main(void)
{
    char* input;
    r.start = -2;
    r.end = -2;
    p.start = -1;
    p.end = -1;

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
        //Get the input from a file, or else from the command prompt.
        if (read_from_file)
        {
            if(!(input = get_input_from_file(fp)))
            {
                fclose(fp);
                read_from_file = false;

                if (stdin_fd > 0)
                {
                    dup2(stdin_fd, STDIN_FILENO);
                    close(stdin_fd);
                }

                if (stdout_fd > 0)
                {
                    dup2(stdout_fd, STDOUT_FILENO);
                    close(stdout_fd);
                }

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

        if (input[0] != '\0' && input[0] != '/')
        {
            //Split the input into tokens
            if (!(tokens = tokens_get(input, &token_num, &var_indices, &var_indices_len)))
                goto end;
            
            // printf("%d\n",r.count);
            // printf("%d\n",r.state);
            //Seperate tokens using the pipe symbol as a delimiter.



            //For every redirect save the filename corresponding to it.
            //Note: The last instance of a redirect has priority over its predeccessors. 
            for (int i = 0; i < r.count; i++) 
            {
                if (error = handle_redirect(tokens, r.array[i], i))
                    goto end;
            }

            //The filenames for redirects should not be treated as additional arguments and are removed from tokens, and token_num is updated.
            //They served their purpose in specifiying the files for input and output, and hence are no longer needed.
            if (r.count) 
                tokens[token_num = r.start] = NULL;

            //If the first argument contains an =, then it means the user is doing variable assignment
            //...and the tokens should not be interpreted as [cmd arg0 arg1 ...], but just as a series of variable assignments.
            if (contains_char(tokens[0],'=') != -1) 
                interpret_vars_assign = true;
            
            //Loop through all tokens, performing variable expansion and assignment per token.
            
            int i,j;
            for (i = 0, j = 0; i < token_num; i++)
            {
                //Perform variable expansion, if applicable.
                if ((j < var_indices_len) && (i == var_indices[j].token_index) && (error = expand_vars(tokens, var_indices, var_indices_len, j++))) 
                    goto end;         

                //Looks for any '=' within the token and assigns accordingly, if applicable.
                if ((interpret_vars_assign) && (error = assign_vars(tokens, token_num, i)))
                    goto end;         
                    
            }
            //Variables assignment operation was handled, so proceed to the next iteration.
            if (interpret_vars_assign)
                goto end;

            //From this comment forward, the first argument the user entered is sure to be a command, and not a variable assignment statement.
            //The first token is the command, all other subsequent tokens are arguments to the command.

            //If there is the word source while reading from a file raise an error

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
                interpret_vars_assign = false;
                p.start = -1;
                p.end = -1;
                p.count = 0;

                
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