//_______________________________Includes______________________________________
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "includes/headers.h"
#include "includes/linenoise-master/linenoise.h"

const char* const error_msg[100] = {
                    MEMORY_ERROR_MSG,
                    BUFFER_ERROR_MSG,
                    PARSE_ERROR_MSG,
                    VARIABLE_DECLARATION_MSG,
                    VARIABLE_EXPANSION_MSG,
                    VARIABLE_ASSIGNMENT_MSG,
                    VARIABLE_NAME_MSG,
                    NODE_NOT_FOUND_MSG,
                    NODE_ASSIGNMENT_MSG,
                    STACK_FULL_MSG,
                    STACK_EMPTY_MSG,
                    TOKENS_MEMORY_MSG,
                    VARINDICES_MEMORY_MSG,
                    INVALID_ARGS_MSG,
                    ENV_VARIABLE_NOT_FOUND_MSG,
                    ENV_VARIABLE_ASSIGNMENT_MSG,
                    CWD_NOT_FOUND_MSG,
                    NULL_GIVEN_MSG,
                    NOT_A_DIR_MSG
                    };

char* const prompt_default = {"init> "};
const char* const exit_keyword = {"exit"};
const char* const metacharacters = {" |;<>\t"};
const char* const quotes = {"\""};
const char* const internal_commands[TOKEN_SIZE] = {"exit", "echo","cd",
                                                   "showvar","export","unset",
                                                   "showenv","pushd","popd",
                                                   "dirs","source"};

int main(void)
{
    // Initalising variables
    char* input;
    redirect_ext ex;
    redirect_int in;
    int token_num;
    char** tokens = NULL;

    int* assign_indices;
    int assign_count = 0;

    int var_indices_len;
    tokenchar_pair* var_indices = NULL;

    char* prompt;
    
    setbuf(stdout, NULL);

    if (signal(SIGINT, SIGINT_handler) == SIG_ERR)
    {
        perror("Signal Error");
        exit(1);
    }

    reset_in(&in);
    reset_ex(&ex);

    top = -1;  //Will always point to the last element of stack. -1 if stack is empty.

    stdin_fd = -1;
    stdout_fd = -1;

    //Initialise all shell variables.
    if (exit_program = (error = init_vars()))
        handle_error();
    else 
        prompt = node_search("PROMPT")->value;
    

    while (!exit_program)
    {   
        //Get the input from a file, or else from the command prompt.
        if (fp)
        {
            if(!(input = get_input_from_file(fp)))
            {
                fclose(fp);
                fp = NULL;

                //Prompt the user to enter text since source command is over.
                input = linenoise(prompt);
            }
        }
        else
            input = linenoise(prompt);


        if (input && input[0] != '\0' && input[0] != '/')
        {
            //Tokenise the input.
            if (!(tokens = tokens_get(input, &token_num, &var_indices, &var_indices_len, &assign_indices, &assign_count, &in, &ex)))
                goto end;
            
            int i = 0;
            int j = 0;
            int k = 0;

            //Loop through all tokens, performing variable expansion and assignment per token. 
            for (i = 0, j = 0; i < token_num; i++)
            {
                //Perform variable expansion, if applicable.

                if ((j < var_indices_len) && (i == var_indices[j].token_index))
                {
                    do 
                    {
                        if (error = expand_vars(tokens, var_indices, var_indices_len, j)) 
                            goto end;         

                    } while (j < var_indices_len-1 && var_indices[j].token_index == var_indices[++j].token_index);
                    //Expand all variables of token i
                }

                //Function 'assign_vars' looks for any '=' within the token and assigns variables accordingly.
                if ((i < assign_count) && (error = assign_vars(tokens, token_num, i, assign_indices[i])))
                    goto end;             
            }

            if (assign_count < token_num)
            {
                // There are 2 systems in place for handling redirects
                // One is for internal commands, the other is for external commands.

                //If there are pipes then it its surely not an internal command.
                if (ex.pipe_count)
                    error = execute_external(tokens, &ex);

                //Check if the first command is an internal command or not
                else
                {
                    int match = 1;
                    /* This for loops breaks if match equals 0
                    and the value of j points to the command it matched to in 'internal_commands' */

                    for (j = 0; j < INTERNAL_COMMANDS_LEN && (match = strcmp(tokens[assign_count],internal_commands[j])); j++);

                    //It is an external command.
                    if (match)
                        error = execute_external(tokens, &ex);

                    //It is an internal command.
                    else
                    {
                        //Raise an error if the word source is included while already executing a source command.
                        if (fp && contains_word(tokens[0],"source"))
                        {
                            fclose(fp);
                            fp = NULL;
                            fprintf(stderr,"Nested source statements are not supported.\n");
                            goto end;
                        }

                        //Configure redirects
                        for (i = 0; i < in.redirect_count; i++) 
                        {
                            if (error = handle_redirect(tokens, in.redirect_indices[i], i, &in))
                                goto end;
                        }

                        //Trim the tokens refering to files.
                        if (in.redirect_start)
                            tokens[in.redirect_start] = NULL;
                        
                        /* Hook any files to standard input/output and execute the internal command if no errors are raised.
                        (in.redirect_start? in.redirect_start:token_num)-1 --> If the tokens were trimmed, then the number of tokens should reflect that.
                        token_num is not simply set to in.redirect_start because token_num must still reflect the actual number of elements dynamically allocated in tokens,
                        so they can be properly freed afterwards using token_num */

                        if (!(error = hook_streams(&in)))
                            error = execute_internal(tokens+assign_count+1, (in.redirect_start? in.redirect_start:token_num)-assign_count-1, (cmdno) j);

                    } 
                }
            }
            
            
            end:
                handle_error(); //Prints Error Message.
                tokens_free(tokens,&token_num); //Frees array holding the tokens.
                var_indices_free(var_indices, &var_indices_len); //Frees array holding variable positions.
                assign_indices_free(assign_indices, &assign_count); //Frees array holding variable assignment positions
                if (reset_streams()) perror("Redirect Error"); // Reverts to standard input/output streams
                reset_ex(&ex); // Resets the 'ex' struct to default values
                reset_in(&in); // Resets the 'in' struct to default values

                //If the user decides to delete the PROMPT variable, the default value should display.
                node* current_node;
                prompt = (!(current_node = node_search("PROMPT")))? prompt_default:current_node->value;
                
                
                
        }
        free(input);
    }

    if (fp) fclose(fp);
    free_vars(); //Free all shell variables
    free_stack(); //Free all items in the directory stack

    return exit_value;
}