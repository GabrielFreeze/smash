//_______________________________Includes______________________________________
#include "includes/linenoise-master/linenoise.c"
#include "includes/methods.c"


int main(void)
{

    in.redirect_start = 0;
    ex.redirect_end = 0;
    ex.pipe_start = -1;
    ex.pipe_end = -1;

    char* input;

    int token_num;
    char** tokens = NULL;

    char line[BUFSIZE];

    tokenchar_pair* var_indices = NULL;
    int var_indices_len;

    bool interpret_vars_assign = false;

    setbuf(stdout, NULL);
    signal(SIGINT, SIG_IGN);

    //Initialise all shell variables.
    if (error = init_vars())
    {
        handle_error();
        exit(0);
    }
    
    char* prompt;
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

                //Reset to standard input/output.
                reset_streams();

                //Prompt the user to enter text since source command is over.
                input = linenoise(prompt);
            }
        }
        else
            input = linenoise(prompt);


        if (input && input[0] != '\0' && input[0] != '/')
        {
            //Tokenise the input.
            if (!(tokens = tokens_get(input, &token_num, &var_indices, &var_indices_len)))
                goto end;
            
            /*If the first argument contains an '=', then it means the user is doing variable assignment
            ...and the tokens should not be interpreted as [cmd arg0 arg1 ...], but just as a series of variable assignments.
            
            Function returns the index of the first instance of the character it searches for, and -1 on failure.
            No character found: -1 + 1 == 0 == false
            Character is in position 0: 0+1 == 1 == true*/

            interpret_vars_assign = contains_char(tokens[0],'=')+1;
            
            //Loop through all tokens, performing variable expansion and assignment per token. 
            int i,j;
            for (i = 0, j = 0; i < token_num; i++)
            {
                //Perform variable expansion, if applicable.
                if ((j < var_indices_len) && (i == var_indices[j].token_index) &&
                (error = expand_vars(tokens, var_indices, var_indices_len, j++))) 
                    goto end;         

                //Function 'assign_vars' looks for any '=' within the token and assigns variables accordingly.
                if ((interpret_vars_assign) && (error = assign_vars(tokens, token_num, i)))
                    goto end;             
            }

            if (!interpret_vars_assign)
            {

                // There are 2 systems in place for handling redirects.
                // One is for internal commands, the other is for external commands.


                //If there are pipes then it its surely not an internal command.
                if (ex.pipe_count)
                    error = execute_external(tokens, token_num);

                //Check if the first command is an internal command or not
                else
                {
                    int match = 1;
                    int j;
                    /* This for loops breaks if match equals 0
                    and the value of j points to the command it matched to in 'internal_commands' */

                    for (j = 0; j < internal_commands_len && (match = strcmp(tokens[0],internal_commands[j])); j++);

                    //It is an external command.
                    if (match)
                        error = execute_external(tokens, token_num);

                    //It is an internal command.
                    else
                    {
                        //Raise an error if the word source is included while already executing a source command.
                        if (read_from_file && contains_word(tokens[0],"source"))
                        {
                            fclose(fp);
                            read_from_file = false;
                            fprintf(stderr,"Nested source statements are not supported.\n");
                            goto end;
                        }

                        //Configure redirects
                        for (int i = 0; i < in.redirect_count; i++) 
                        {
                            if (error = handle_redirect(tokens, in.redirect_indices[i], i))
                                goto end;
                        }

                        //Trim the tokens refering to files, and update token_num accordingly.
                        if (in.redirect_start)
                            tokens[token_num = in.redirect_start] = NULL;
                        
                        // Hook any files to standard input/output and execute the internal command if no errors are raised. 
                        if (!(error = hook_streams()))
                            error = execute_internal(tokens+1, token_num-1, j);

                    } 
                }
            }
                

        
            

            end:
                handle_error(); //Prints Error Message.
                tokens_free(tokens,&token_num); //Frees array holding the tokens.
                var_indices_free(var_indices,&var_indices_len); //Frees array holding variable positions.
                reset_streams(); // Reverts to standard input/output streams
                reset_ex(); // Resets the 'ex' struct to default values
                reset_in(); // Resets the 'in' struct to default values

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