#include "headers.h"
#include "linkedlist.c"
#include "stack.c"

// Tokenisation.
int tokens_init(char* string)
{
    int count = 0;
    bool in_quotes = false;
    int type, prev_type = NONE;
    bool is_output_cat = false;
    bool special_before = false;

    for (int i = 0; i < strlen(string); i++)
    {
        type = char_type(string,i);

        if (in_quotes && type != ESCAPE && type != VARIABLE && type != QUOTE)
            type = NORMAL;
        
        if (type == NORMAL)
            special_before = false;

        if (type == OUTPUT && is_output_cat)
            is_output_cat = false;
        else
        {
            is_output_cat = false;

            if (type == PIPE)
            {
                if (special_before)
                    return 0;

                ex.pipe_count++;
                
                ex.section[ex.pipe_count]->output = 0;
                ex.section[ex.pipe_count]->input = 0;
                ex.section[ex.pipe_count]->redirect_count = 0;


            
                special_before = true;
                ex.pipe_end =  (prev_type == META)? count:count+1;

                 
                if (ex.pipe_start < 0)
                    ex.pipe_start = ex.pipe_end;
                   
                if (!i || !ex.pipe_start)
                    return 0;
                
                
                ex.pipe_indices[ex.pipe_count-1] = ex.pipe_end;
                ex.pipe_indices[ex.pipe_count] = 0;

            }

            if (i != strlen(string)-1 && type == OUTPUT && char_type(string, i+1) == OUTPUT)
            {
                type = OUTPUT_CAT;
                is_output_cat = true;
            }

             //If the type is a redirect
            if (type >= OUTPUT && type < PIPE)
            {
                if (special_before)
                    return 0;

                in.redirect_count++;

                special_before = true;

                ex.redirect_end = (prev_type == META)? count:count+1;

                
                if (!in.redirect_start)
                    in.redirect_start = ex.redirect_end;
                
                if (!i || !in.redirect_start)
                    return 0;
                
                if (ex.pipe_count >= BUFSIZE-1)
                    return 0;
                
                in.redirect_indices[in.redirect_count-1] = type;
                ex.section[ex.pipe_count]->redirect_count ++;
                
                if (type == INPUT)
                    ex.section[ex.pipe_count]->input = ex.redirect_end;
                if (type == OUTPUT)
                {
                    ex.section[ex.pipe_count]->output = ex.redirect_end;
                    ex.section[ex.pipe_count]->cat = false;
                }
                if (type == OUTPUT_CAT)
                {
                    ex.section[ex.pipe_count]->output = ex.redirect_end;
                    ex.section[ex.pipe_count]->cat = true;

                }
            }

            if (type == QUOTE)
                in_quotes ^= true;
            
            //If: The character is a meta character, and its previous characters are either normal or quote
            //Then: This means a token just ended, so increment token_count

            if ((type == QUOTE && prev_type == QUOTE) ||
            (prev_type == VARIABLE && type != NORMAL && type != QUOTE))
                return 0;
            

            if (!in_quotes && is_meta(string,i) && (prev_type == NORMAL || prev_type == QUOTE))
                ++count;
        }
        prev_type = type;
    }

    //Since the only way a token is considered is if a string of characters is terminated with a meta character...
    //Check if the last character was META or not. If it wasn't meta, that means there is one final token that we need to consider
    if (type == NORMAL || type == QUOTE) 
        count++;
    
    //There mustn't be an odd amount of quotes or a redirect or a pipe character in the last token.
    if (in_quotes || ex.redirect_end == count || ex.pipe_end == count)
        return 0;

    ex.pipe_indices[ex.pipe_count] = count;
    return count;

    
}
int char_type(char* string, int j)
{
    if (j < 0){
        return NONE;
    }
        
    for (int i = 0; i < strlen(quotes); i++)
    {
        if (string[j] == quotes[i])
        {
            if (is_deref(string,j))
                return NORMAL;
            else
                return QUOTE;
        }
    }
    
    if (string[j] == '\\')
    {
        if (is_deref(string, j))
            return NORMAL;
        else
            return ESCAPE;
    }
    
    if (string[j] == '$')
    {
        if (is_deref(string,j))
            return NORMAL;
        else
            return VARIABLE;
    }

    if (string[j] == '>')
    {
        if (is_deref(string,j))
            return NORMAL;
        else
            return OUTPUT;
    }

    if (string[j] == '<')
    {
        if (is_deref(string,j))
            return NORMAL;
        else
            return INPUT;
    }

    if (string[j] == '|')
    {
        if (is_deref(string, j))
            return NORMAL;
        else
            return PIPE;
    }

    return is_meta(string,j)? META:NORMAL;

}
bool is_meta(char* string, int j)
{

    for (int i = 0; i < strlen(metacharacters); i++)
    {
        if (string[j] == metacharacters[i])
        {
            if (is_deref(string, j))
                return 0;
            else
                return 1;
        }
    }

    return 0;
}
bool is_deref(char* string, int upper)
{
    if (upper == 0)
        return false;

    int lower = upper-1;


    while (lower >= 0 && string[lower] == '\\')
        lower --;

    return ((upper-lower) % 2 == 0);

}
char** tokens_get(char* input, int* length, tokenchar_pair** var_indices, int* var_index)
{  
    int index = 0;
    int j = 0;
    char** tokens;
    int max_length;
    *length = 0;
    *var_indices = NULL;
    tokenchar_pair* var_indices2;
    char current_token[TOKEN_SIZE];
    bool in_quotes = false;
    bool meta = false;
    int type, prev_type = NONE;

    if (!(max_length = tokens_init(input)))
    {
        error = PARSE_ERROR;
        return NULL;
    }
    
    //Using calloc and an extra 1 so I can pass these tokens as a null terminated array of pointers to execvp.
    if (!(tokens = (char**) calloc(max_length+1, sizeof(char*))))
    {
        error = TOKENS_MEMORY_ERROR;
        return tokens;
    }
 
    //Lets assume the worst case scenario. All the tokens provided are variables that need to be expanded.
    if(!(var_indices2 = (tokenchar_pair*) malloc(max_length * sizeof(tokenchar_pair))))
    {
        error = VARINDICES_MEMORY_ERROR;
        return tokens;
    }
 
    for (int i = 0; i < strlen(input); i++)
    {
        meta = is_meta(input, i);
        prev_type = type;
        type = char_type(input, i);

        if (j == TOKEN_SIZE-1)
        {
            error =  BUFFER_OVERFLOW_ERROR;
            return tokens;
        }


        //Eg: $ followed by a META, or $""
        if (prev_type == VARIABLE && type != NORMAL && type != QUOTE)
        {
            error = VARIABLE_DECLARATION_ERROR;
            return tokens;
        }
        
        //Lets store the index to the variable for later use (when we perform expansion)

        if (type == VARIABLE)
        {
            var_indices2[*var_index].token_index = index;
            var_indices2[(*var_index)++].char_index = j;
        }

        if (type == QUOTE)
        {       
            in_quotes ^= true;
            continue;   
        }            

        //Anything in quotes except for special shell characters should be treated as is, devoid of any special meaning.
        if (in_quotes && type != ESCAPE && type != VARIABLE)
        {
            type = NORMAL;
            meta = false;
        }

        //Just like the token_len function, if a metacharacter terminated a string, then this is the end of a token
        //So lets save it in into tokens.
        if (meta && (prev_type == NORMAL || prev_type == QUOTE))
        {
            if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    error = MEMORY_ERROR;
                    return tokens;
                }
            
            current_token[j++] = '\0';
            strncpy(tokens[index++], current_token, j);

            //If the function returns due to an error mid-way, then that means not all tokens have
            //dynamic memory allocated to them. Length keeps track of the current token_length so 
            //only the tokens with dynamic memory are freed afterwards.
            *length = index; 

            j = 0;
        }

        else if (meta || (type == ESCAPE))
            continue;
        else
            current_token[j++] = input[i];
                
    }

    //If j is greater than 0 , that means there is still data in the current_token vector, which has not been added to tokens.
    if (j > 0) 
    {
        if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
        {   
            error = MEMORY_ERROR;
            return tokens;
        }
        current_token[j++] = '\0';
        strncpy(tokens[index++], current_token, j);
        *length = index;
    }
    
    *var_indices = var_indices2;

    return tokens;

}

// Error handling and variable resetting.
int handle_error()
{
    if (error == SYSTEM_CALL_ERROR)
    {
        perror("Error");
        return 0;
    }
    
    if (error)
        fprintf(stderr,"%s",errors[error]);
   

    return 0;
    
}
int tokens_free(char** tokens, int* length)
{
    if (!tokens)
        return 0;

    for (int i = 0; i < (*length)+1; i++)
        free(tokens[i]);

    if (*length > 0)
        free(tokens);

    *length = -1;
    tokens = NULL;
    return 0;
}
int var_indices_free(tokenchar_pair* var_indices, int* var_indices_len)
{
    if (!var_indices)
        return 0;
    if (*var_indices_len > 0)
        free(var_indices);
        
    var_indices = NULL;
    *var_indices_len = -1;

    return 0;
}
void reset_streams()
{
    if (stdin_fd > 0 && !read_from_file)
    {
        dup2(stdin_fd, STDIN_FILENO);
        close(stdin_fd);
    }
    if (stdout_fd > 0 && !read_from_file)
    {
        dup2(stdout_fd, STDOUT_FILENO);
        close(stdout_fd);
    }
}
void reset_ex()
{
    ex.section[0]->input = 0;
    ex.section[0]->output = 0;
    ex.section[0]->redirect_count = 0;
    ex.pipe_count = 0;
    ex.pipe_start = -1;
    ex.pipe_end = -1;
    ex.redirect_end = 0;
    new_start = 0;
}
void reset_in()
{
    in.redirect_count = 0;
    in.input_filename[0] = 0;
    in.output_filename[0] = 0;
    in.output_cat_filename[0] = 0;
    in.redirect_start = 0;
}

//Shell variables.
int init_vars(void)
{
    // Set every enviroment variable as a shell variable, with the bool env set to true.
    for (int i = 0; environ[i]; i++) 
    {
        char var[VALUE_SIZE];
        strncpy(var,environ[i],VALUE_SIZE);

        char* key = strtok(var, "=");
        if (error = node_insert(key, getenv(key), true))
            return error;
    }

    //Checking if PATH, USER and HOME were set

    int uid = geteuid(); // Get effective user id
    struct passwd *pass = getpwuid(uid); // Get password file entry structure

    if (!node_search("PATH") && (error = node_insert("PATH","/bin",true)))
        return error;
    
    if (!node_search("HOME") && (error = node_insert("HOME",pass->pw_dir,true)))
        return error;
    
    if (!node_search("USER") && (error = node_insert("USER",pass->pw_name,true)))
        return error;
    
    if (!node_search("PROMPT") && (error = node_insert("PROMPT", "init>", true)))
        return error;

    if (!node_search("TERMINAL") && (error = node_insert("TERMINAL", getenv("TERM"), true)))
        return error;
   
    if (!node_search("EXITCODE") && (error = node_insert("EXITCODE", "NONE", true)))
        return error;

    //Renew the location of the shell executable
    char shell[BUFSIZE];
    int num_bytes;
    if ((num_bytes = readlink("/proc/self/exe", shell, BUFSIZE-1)) == -1)
        return NODE_ASSIGNMENT_ERROR;
    
    shell[num_bytes] = '\0';

    if (error = node_insert("SHELL", shell ,true))
        return error;

    //Any changes to PWD (env), should be mirrored to CWD (env), which is then mirrored to CWD (shell).
    // On initialisation, home points to the current working directory

    char cwd[VALUE_SIZE];
    strncpy(cwd,getenv("HOME"),VALUE_SIZE);

    if (error = node_insert("CWD", cwd, true)) //Create shell variable CWD.
        return error;
    
    if (setenv("PWD", cwd, true)) //Create PWD/Edit PWD(env), and mirror it with CWD(shell) and CWD(env)
        return ENV_VARIABLE_ASSIGNMENT_ERROR;

    //Will delete the node if it exists since we will be working with CWD(shell) not PWD(shell)
    node* current_node;
    if (current_node = node_search("PWD"))
        node_delete(current_node); 
    
    if (error = push(cwd)) //Pushing cwd onto the directory stack
        return error;

    chdir(cwd);
    
    //______________________________________________________________________


    return 0;

} 
bool vars_valid(char* token, int j)
{
    
    if (    (token[j] >= '0'  && token[j] <= '9') || 
            (token[j] >= 'a'  && token[j] <= 'z') || 
            (token[j] >= 'A'  && token[j] <= 'Z') || 
            (token[j] == '_'))
        return true;

    return false;        
}
int expand_vars(char* tokens[TOKEN_SIZE], tokenchar_pair* var_indices, int var_indices_len, int m)
{
    int equal = 0;
    char token[TOKEN_SIZE];
    char append[TOKEN_SIZE];

    int end = 0;
    int offset = 1;
    int token_index;
    int char_index; 
    int value_len = 0;
    int append_len = 0;
    int original_len = 0;   

    token_index = var_indices[m].token_index;
    char_index = var_indices[m].char_index;

    strcpy(token, tokens[token_index] + char_index); //Ignoring all before the $
    end = strlen(token);

    if (token[1] == '{') //Using <Any Chars> ${...} <Any Chars> notation
    {
        for (int l = 2; l < strlen(token); l++)
        {
            if (token[l] == '}')
            {
                end = l;
                break;
            }
        }
        if (end == 2)
            return VARIABLE_DECLARATION_ERROR;
        
        token[end] = '\0';
        offset = 2;
    }
    else //Using  <Any Chars> $... <Illegeal Chars> notation
    {
        //Stop until you encounter an illegal character,
        for (int j = 1; j < end; j++)
        {
            if (!vars_valid(token,j))
            {
                end =  j;
                token[end] = '\0';
                break;
            }      
        }
            
    }
    //Check if its in the environment variables

    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
    {
        if (!(equal = strcmp(token + offset, current_node->key)))
        {
            strcpy(append, tokens[token_index] + char_index + end + offset-1);
            strcpy(tokens[token_index] + char_index, current_node->value);
            append_len = strlen(append);
            value_len = strlen(current_node->value);

            //If there is not enough space to replace the variable with the textual data and also include all other subsequent characters
            if (value_len > TOKEN_SIZE || append_len > TOKEN_SIZE || char_index+value_len+append_len > TOKEN_SIZE)
                return BUFFER_OVERFLOW_ERROR;
                
            //Adding the other characters after the key was replaced with the value
            for (int k = 0; k < append_len+1; k++)
                tokens[token_index][char_index+value_len+k] = append[k];

            break;
        }
    }
    
    

    if (equal)
        return VARIABLE_EXPANSION_ERROR;

    //For the remaining variables within the token, add to char_index: - var_name(including $) + value_len
    // Since the current token will be edited, all pointers to variable characters must be shifted.

    for (int j = m+1; j < var_indices_len; j++)
    {
        if (var_indices[j].token_index != token_index)
            break;
        
        var_indices[j].char_index += -(strlen(token) + offset-1) + value_len;
    }

    
    return 0;
}
int assign_vars(char** tokens, int length, int i)
{

    node* current_node;
    char current_token[TOKEN_SIZE];
    char key_value[2][TOKEN_SIZE];
    int j = 0;
    int current_token_len = strlen(tokens[i]);

    strcpy(current_token,tokens[i]);

    for (char* string = strtok(current_token, "="); (string != NULL) && (strlen(string) != current_token_len); string = strtok(NULL, "="))
    {
        if (j == 2)
            return VARIABLE_ASSIGNMENT_ERROR;
        strcpy(key_value[j++],string);
    }

    //If j != 2 then that means the token didn't have exactly one '='
    if (j != 2)
        return VARIABLE_ASSIGNMENT_ERROR;

    if (key_value[0][0] >= '0' && key_value[0][0] <= '9')
        return VARIABLE_NAME_ERROR;
    
    
    //If: The node doesn't exist
    //Then: Create it and assign it the values given.
    //Else(if it does exists): edit the node with the new value and...
    // if the variable edited was CWD, update PWD(env) with the same value

    if (!(current_node = node_search(key_value[0])))
    {
        if (error = node_insert(key_value[0], key_value[1], false))
            return error;
    }
    else
    {
        if (error = node_edit(current_node, key_value[1]))
            return error;
        
        if (!strcmp(current_node->key,"CWD") && setenv("PWD",getenv("CWD"),1))
            return ENV_VARIABLE_NOT_FOUND_ERROR;
    }
    
    return 0;


}

// Commands.
int execute_internal(char* args[TOKEN_SIZE], int arg_num, int j)
{
    //Note that the first element of args is NOT the name of the command, its the first argument
    switch (j)
    {
        case EXIT_CMD:
        {
            int exit_value;
            int error;
            //Maximum arg size is 1
            if (arg_num > 1)
                return INVALID_ARGS_ERROR;

            //If the argument is provided
            if (arg_num == 1)
            {
                //If the argument was translated to int
                if(!(error = str_to_int(&exit_value,args[0])))
                {
                    printf("Shell terminated with exit code [%d]\n",exit_value);
                    exit(exit_value);
                }
                
                else
                    return error;
            }
            //If no argument was given
            printf("Shell terminated with exit code [0]\n");
            exit(0);
                
        }
        case ECHO_CMD:
        {
            if (arg_num < 1)
                return INVALID_ARGS_ERROR;
            
            for(int i = 0; i < arg_num; i++)
                printf("%s ",args[i]);
            printf("\n");
            return 0;
        }
        case CD_CMD:
        {
           //Check if arg_num is valid and change the environment variable
           //The only reason chdir should fail is because the supplied argument was invalid. 
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            struct stat sb;
            //Checks if argument is a existing directory
            if (stat(args[0], &sb) || !S_ISDIR(sb.st_mode))
                return NOT_A_DIR_ERROR;
            
            if (error = change_directory(args[0]))
                return error;
            return 0;
        }
        case SHOWVAR_CMD:
        {   

            if (!arg_num) //Print all shell vars
                nodes_print();
            else //Print just one shell var
            {
                for (int i = 0; i < arg_num; i++)
                {
                    node* current_node;
                    if (!(current_node = node_search(args[i])))
                        return NODE_NOT_FOUND_ERROR;

                    printf("%s=%s\n",current_node->key,current_node->value);
                }      
            }

            return 0;
        }
        case EXPORT_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            node* current_node;
            if (!(current_node = node_search(args[0])))
                return NODE_NOT_FOUND_ERROR;

            if(error = node_export(current_node))
                return error;
            
            return 0;
        }
        case UNSET_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            node* current_node;
            if (current_node = node_search(args[0]))
                node_delete(current_node);
            else
                return NODE_NOT_FOUND_ERROR;

            if (unsetenv(args[0]))
                return ENV_VARIABLE_NOT_FOUND_ERROR;

            return 0;
        }
        case SHOWENV_CMD:
        {
            if (!arg_num)
            {
                for (char** var = environ; *var; var++)
                    printf("%s\n",*var);
            }
            else
            {
                for (int i = 0; i < arg_num; i++)
                {
                    char* var;
                    if (!(var = getenv(args[i])))
                        return ENV_VARIABLE_NOT_FOUND_ERROR;

                    printf("%s=%s\n", args[i], var);
                }
                
            }

            return 0;
        }
        case PUSHD_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            struct stat sb;

            //Checks if argument is a existing directory
            if (stat(args[0], &sb) || !S_ISDIR(sb.st_mode))
                return NOT_A_DIR_ERROR;
            
            //Pushing directory into stack and changing current working directory to new value
            if (error = push(args[0]))
                return error;
            
            //Prints the stack to view the new changes
            if (error = print_stack()) 
                return error;

            return 0;
        }
        case POPD_CMD:
        {
            if (arg_num)
                return INVALID_ARGS_ERROR;

            char* popped_value;

            //Pops the value from the stack and updates current working directory.
            if (error = pop(&popped_value))
                return error;

            if (error = print_stack()) //Prints the stack to view the new changes
                return error;

            return 0;
        }
        case DIRS_CMD:
        {
            if (arg_num)
                return INVALID_ARGS_ERROR;
            
            if (error = print_stack())
                return error;
            
            return 0;
        }
        case SOURCE_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            if (!(fp = fopen(args[0],"r")))
                return SYSTEM_CALL_ERROR;
            
            read_from_file = true;
    
            return 0;
        }
      
        default:
            return 0;
    }
}
int execute_external(char** tokens, int token_num)
{
    int fd[ex.pipe_count*2];
    int* current_fd = fd;
    int* previous_fd;
    pid_t pid;
    int output_file;
    int fd_output;
    int input_file;
    int fd_input;
    int status;
    int exitcode;
    char str[10];
    
    for (int i = 0; i < ex.pipe_count+1; i++, previous_fd = current_fd, current_fd += 2)
    {
        int argc = 0;
        char* args[BUFSIZE];
        pid_t pid;

        // This for loops iterates over the a set of arguments seperated by pipes.
        // If the set of arguments has redirection files specified, they are not iterated over.

        for (int j = new_start; j < ex.pipe_indices[i]-ex.section[i]->redirect_count; j++)
            args[argc++] = tokens[j];
        args[argc] = NULL;
        
        // The next iterations starts from  the end of the previous.
        new_start = ex.pipe_indices[i];
        
        if (i < ex.pipe_count)
            pipe(current_fd);
         
        if ((pid = fork()) < 0)
            return SYSTEM_CALL_ERROR;
        else if (!pid) // Child Process
        {

            // Hook output based on pipeline
            if (i < ex.pipe_count)
            {
                close(current_fd[0]);
                dup2(current_fd[1], STDOUT_FILENO);
                close(current_fd[1]);
                
            }
            //Hook output if the user specified a file
            if (output_file = ex.section[i]->output)
            {
                //Opens file in rw or a+
                if ((fd_output = open(tokens[output_file], ex.section[i]->cat? (O_CREAT | O_APPEND | O_RDWR):(O_CREAT | O_RDWR | O_TRUNC), S_IRWXU)) < 0)
                {
                    perror("File Error");
                    exit(1);
                }

                dup2(fd_output,STDOUT_FILENO);
                close(fd_output); 
            }
            
            // Hooks input based on pipeline
            if (i)
            {
                close(previous_fd[1]);
                dup2(previous_fd[0],STDIN_FILENO);
                close(previous_fd[0]);
            }
            // Hooks input if the user specified a file
            if (input_file = ex.section[i]->input)
            {
                if ((fd_input = open(tokens[input_file], O_RDWR)) < 0)
                {
                    perror("File Error");
                    exit(1);
                }
                dup2(fd_input, STDIN_FILENO);
                close(fd_input);
            }
            
            execvp(args[0], args);
            perror("Exec Error");
            exit(1);
        }

        if (i) // The first parent process
        {
            close(previous_fd[0]);
            close(previous_fd[1]);
        }

        if (wait(&status) > 0 && WIFEXITED(status))
        { 
            exitcode = WEXITSTATUS(status);
            //Converts int to string
            sprintf(str, "%d", exitcode);

            //Stores exitcode in shell variable EXITCODE
            if (error = node_edit(node_search("EXITCODE"), str)) 
                fprintf(stderr,"Could not save exitcode");
        } 

    }

    return 0;
}

//Redirects for internal commands.
int handle_redirect(char** tokens, int state, int j)
{

    //What redirect is it?
    if (state == INPUT)
        strcpy(in.input_filename, tokens[in.redirect_start+j]);

    if (state == OUTPUT)
        strcpy(in.output_filename, tokens[in.redirect_start+j]);

    if (state == OUTPUT_CAT)
        strcpy(in.output_cat_filename, tokens[in.redirect_start+j]);
    
    return 0;

}
int hook_streams()
{
    int match = 1;
    int fd_input;
    int fd_output;
    int fd_output_cat;
    error = 0;

    // Do the input and output have to be redirected?
    // Opens files, links them with respective stream.

    
    // if (r.input[0] && !read_from_file)
    if (in.input_filename[0] && !read_from_file)
    {
        // if ((fd_input = open(r.input, O_RDWR)) == -1)
        if ((fd_input = open(in.input_filename, O_RDWR)) == -1)
            error = SYSTEM_CALL_ERROR;
        else
        {
            stdin_fd = dup(STDIN_FILENO);
            fprintf(stderr,"%d\n",fd_input);

            dup2(fd_input,STDIN_FILENO);
            close(fd_input);
        }
    } 
    
    // if (r.output[0] && !read_from_file)
    if (in.output_filename[0] && !read_from_file)
    {
        // if ((fd_output = open(r.output, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU)) == -1)
        if ((fd_output = open(in.output_filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU)) == -1)
            error = SYSTEM_CALL_ERROR;
        else
        {
            stdout_fd = dup(STDOUT_FILENO);

            dup2(fd_output,STDOUT_FILENO);
            close(fd_output);
        }
    }
    
    // if (r.output_cat[0] && !read_from_file)
    if (in.output_cat_filename[0] && !read_from_file)
    {
        // if ((fd_output = open(r.output_cat, O_CREAT | O_APPEND | O_RDWR, S_IRWXU)) == -1)
        if ((fd_output = open(in.output_cat_filename, O_CREAT | O_APPEND | O_RDWR, S_IRWXU)) == -1)
            error = SYSTEM_CALL_ERROR;
        else
        {
        stdout_fd = dup(STDOUT_FILENO);

        dup2(fd_output,STDOUT_FILENO); 
        close(fd_output);
        }

    }

    return error;
}

//Miscellanous.
int contains_char(char* string, char a)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == a)
            return i;
    }

    return -1;
}
int str_to_int(int* value, char* string)
{
    char* end;

    long num = strtol(string, &end, 10);

    //Checks if the string was all numbers, and if this number is small enough to be stored in an int
    if ((string == end) || (num > INT_MAX) || (num < INT_MIN) || (*end != '\0')) 
        return INVALID_ARGS_ERROR;

    *value = (int) num;
    return 0;
}
char* get_input_from_file(FILE* fp)
{
    char line [BUFSIZE];
    error = 0;

    if (!(fgets(line,BUFSIZ,fp)))
        return NULL;

    int length = strlen(line);

    char* input;
    if (!(input = (char*) malloc(length)))
    {
        error = MEMORY_ERROR;
        return NULL;
    }
    
    //Change the newline character with a null terminator

    if (line[length-2] == '\r')
        line[length-2] = '\0';
    else
        line[length-1] = '\0';


    strcpy(input,line);


    return input;

}
int contains_word(char* input, char* key)
{
    int input_len = strlen(input);
    int key_len = strlen(key);
    int j = 0;

    if (input_len < key_len)
        return 0;

    for (int i = 0; i <= input_len-key_len; i++)
    {
        for (j = 0; j < key_len; j++)
        {
            if (input[i+j] != key[j])
                break;
        }    

        if (j < key_len) // Loop breaked
            continue;
        else //Loop finished
            return 1;
    }

    return 0;


}
