#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include "limits.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "headers.h"


// Linked List
err node_insert(const char* const key, const char* const value, bool env)
{
    node* new_node;
    node* current_node;

    //If there already exists a node with that key, remove it.
    if (current_node = node_search(key))
        node_delete(current_node);

    if  (!((new_node = (node*) malloc(sizeof(node))) &&
        ((new_node->key = (char*) malloc(strlen(key)+1)) && 
        (new_node->value = (char*) malloc(strlen(value)+1)))))
        return MEMORY_ERROR;

    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->env = env;

    if (!tail)
        tail = new_node;
    else
        head->prev = new_node;


    new_node->next = head;
    head = new_node;
    head->prev = NULL;
    if (env && setenv(key,value,true))
        return ENV_VARIABLE_ASSIGNMENT_ERROR;
        
    return NONE;

}
node* node_search(const char* key)
{
    node* current_node;

    if (!(current_node = head))
        return NULL;

    while (current_node && strcmp(current_node->key,key))
        current_node = current_node->next;

    return current_node;
}
err node_delete(node* current_node)
{
    if (!current_node)
        return NODE_NOT_FOUND_ERROR;
    
    if (current_node->env && unsetenv(current_node->key)) 
        return ENV_VARIABLE_NOT_FOUND_ERROR;

    if (current_node == head)
      head = head->next;
   else
      current_node->prev->next = current_node->next; //The previous node will skip the current_node and point to the one after it.

   if (current_node == tail)
      tail = current_node->prev;
   else
      current_node->next->prev = current_node->prev; //The next node will skip the current_node and point to the one before it.

    if (current_node->value)
        free(current_node->value);

    if (current_node->key)
        free(current_node->key);

    free(current_node);

    return NONE;

}
err node_edit(node* current_node, const char* value)
{
    if (!current_node)
        return NODE_NOT_FOUND_ERROR;
    
    if (!value)
        return NULL_GIVEN_ERROR;

    if (!(current_node->value = (char*) realloc(current_node->value, strlen(value)+1)))
        return MEMORY_ERROR;
        
    strcpy(current_node->value,value);

    //Update the enviroment variable representing this shell variable.
    if (current_node->env && setenv(current_node->key,value,1))
        return ENV_VARIABLE_NOT_FOUND_ERROR;

    return NONE;
}
void nodes_print(){
    for (node* current_node = head; current_node; current_node = current_node->next)
        printf("%s=%s\n",current_node->key, current_node->value);  
}
err node_export(node* current_node)
{
    if (!current_node)
        return NODE_NOT_FOUND_ERROR;
    
    current_node->env = true;

    if (setenv(current_node->key,current_node->value,1))
        return ENV_VARIABLE_ASSIGNMENT_ERROR;

    return NONE;
}

// Directory Stack
bool is_full()
{
    return top+1 == STACK_SIZE;
}
err push(const char* value)
{
    if (is_full())
        return STACK_FULL_ERROR;
    
    if(!(stack[++top] = (char*) malloc(strlen(value)+1)))
        return MEMORY_ERROR;

    strcpy(stack[top],value);

    if (error = change_directory(value))
        return error;

    return NONE;
}   
err pop(char** value)
{
    //peek already checks if the stack only has one value or not. peek ⊆ pop    
    if (error = peek(value))
        return error;

    free(stack[top--]);

    //Updates the current working directory to the top most value, after the pop.
    if (error = change_directory(stack[top]))
        return error;

    return NONE;
}
err peek(char** value)
{
    if (!top)
        return STACK_EMPTY_ERROR;

    char* value2;
    if(!(value2 = (char*) malloc(strlen(stack[top])+1)))
        return MEMORY_ERROR;

    strcpy(value2,stack[top]);
    *value = value2;
    return NONE;
}
err print_stack()
{
    for (int i = top; i >= 0; i--)
        printf("%s  ",stack[i]);
    printf("\n");

    return NONE;
}
err change_topmost(const char* value)
{   
    stack[top][0] = 0;
    if (!(stack[top] = (char*) realloc(stack[top], strlen(value)+1)))
        return MEMORY_ERROR;

    strcpy(stack[top],value);
    return NONE;
}
err change_directory(const char* cwd)
{
    if (!cwd)
        return CWD_NOT_FOUND_ERROR;

    if (chdir(cwd)) //Changing the directory
        return SYSTEM_CALL_ERROR; //Change this to perror maybe

    char* new_cwd;
    if (!(new_cwd = getcwd(NULL,0))) //Getting the new directory
        return SYSTEM_CALL_ERROR;

    if (setenv("PWD",new_cwd,1)) //Setting PWD(env) to the new directory
        return ENV_VARIABLE_NOT_FOUND_ERROR;

    node* current_node;

    if (!(current_node = node_search("CWD"))) 
        return CWD_NOT_FOUND_ERROR;

    if (error = node_edit(current_node, new_cwd)) //Set CWD(env and shell) to the new directory
        return error;

    if (error = change_topmost(new_cwd)) //Updating dierctory stack by changing the top most element.
        return error;
    
    free(new_cwd);
    return NONE;

}

// Tokenisation.
int tokens_init(const char* string, redirect_int* in, redirect_ext* ex)
{
    int count = 0;
    bool in_quotes = false;
    charno type, prev_type = BLANK;
    bool is_output_cat = false;
    bool special_before = false;

    bool encountered_equals = false;
    bool consider_equals = true;

    int assign_indices[BUFSIZE];
    int assign_count = 0;


    for (int i = 0; i < strlen(string); i++)
    {
        type = char_type(string,i); //Get the current type

        //Is the character in quotes?
        if (in_quotes && type != ESCAPE && type != VARIABLE && type != QUOTE)
            type = NORMAL;


        if (type == NORMAL)
            special_before = false;
        
        // Since >> are two characters, the program already checks
        // if they are 2 on the first character in (A) and handles that case correctly.
        // The next character we can just ignore it because it was already handled.

        if (type == OUTPUT && is_output_cat)
            is_output_cat = false;
        else
        {
            is_output_cat = false;
             
            if (type == PIPE)
            {
                if (special_before)
                    return 0;

                if (++ex->pipe_count == BUFSIZE)
                    return 0;
                
                ex->section[ex->pipe_count].output = 0;
                ex->section[ex->pipe_count].input = 0;
                ex->section[ex->pipe_count].redirect_count = 0;

                special_before = true;
                ex->pipe_end =  (prev_type == META)? count:count+1;

                 
                if (ex->pipe_start < 0)
                    ex->pipe_start = ex->pipe_end;
                   
                if (!i || !ex->pipe_start)
                    return 0;
                
                ex->pipe_indices[ex->pipe_count-1] = ex->pipe_end;
                ex->pipe_indices[ex->pipe_count] = 0;

            }
            
            //A:    Check if there are two (>) in succession.
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

                if (++in->redirect_count == BUFSIZE)
                    return 0;

                special_before = true;

                //Update current redirect index
                ex->redirect_end = (prev_type == META)? count:count+1;

                //Set the beginning of the redirects to the current redirect index if not already set
                if (!in->redirect_start)
                    in->redirect_start = ex->redirect_end;
                
                //Is there a redirect character as the first token? Raise an error if so.
                if (!i || !in->redirect_start)
                    return 0;
                
                //Save the type of redirect character
                in->redirect_indices[in->redirect_count-1] = type;
                ex->section[ex->pipe_count].redirect_count ++;
                
                if (type == INPUT)
                    ex->section[ex->pipe_count].input = ex->redirect_end;
                if (type == OUTPUT)
                {
                    ex->section[ex->pipe_count].output = ex->redirect_end;
                    ex->section[ex->pipe_count].cat = false;
                }
                if (type == OUTPUT_CAT)
                {
                    ex->section[ex->pipe_count].output = ex->redirect_end;
                    ex->section[ex->pipe_count].cat = true;

                }
            }
            

            if (type == QUOTE)
                in_quotes ^= true;
            

            if ((type == QUOTE && prev_type == QUOTE) ||
            (prev_type == VARIABLE && type != NORMAL && type != QUOTE & type != EQUAL))
                return 0;
            

            //If: The character is a meta character, and its previous characters are either normal or quote
            //Then: This means a token just ended, so increment token_count
            if (!in_quotes && is_meta(string,i) && (prev_type == NORMAL || prev_type == QUOTE || prev_type == EQUAL))
                ++count;
        }
        prev_type = type;
    }

    //Since the only way a token is considered is if a string of characters is terminated with a meta character...
    //Check if the last character was META or not. If it wasn't meta, that means there is one final token that we need to consider
    if (type == NORMAL || type == QUOTE || type == EQUAL) 
        count++;
    
    //Last character can't be any of these
    if (type == ESCAPE || type == VARIABLE)
        return 0;

    //There mustn't be an odd amount of quotes or a redirect or a pipe character in the last token.
    if (in_quotes || ex->redirect_end == count || ex->pipe_end == count)
        return 0;

    ex->pipe_indices[ex->pipe_count] = count;
    return count;

    
}
charno char_type(const char* string, int j)
{
    /*Function basically determines the type of the character. It checks every possibility
    and if there is a match it checks wether its derferenced or not by a preceeding slash*/

    if (j < 0){
        return BLANK;
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
    if (string[j] == '=')
    {
        if (is_deref(string, j))
            return NORMAL;
        else
            return EQUAL;
    }

    return is_meta(string,j)? META:NORMAL;

}
bool is_meta(const char* string, int j)
{

    for (int i = 0; i < strlen(metacharacters); i++)
    {
        if (string[j] == metacharacters[i])
            return !is_deref(string, j); 
    }

    return 0;
}
bool is_deref(const char* string, int upper)
{
    // Counts the number of escape character behind a character.
    // This is so to determine whether the (\) character before it is an Escape Character,
    // ...or a Dereferenced Escape Character with no special meaning.
    if (!upper)
        return false;

    int lower = upper-1;

    while (lower+1 && string[lower--] == '\\');

    return (upper-lower)%2;

}
char** tokens_get(const char* input, int* length, tokenchar_pair** var_indices, int* var_length, int** assign_indices, int* assign_count, redirect_int* in, redirect_ext* ex)
{  
    *length = 0;
    *var_indices = NULL;
    *assign_indices = NULL;
    *var_length = 0;
    *assign_count = 0;
    tokenchar_pair* var_indices2;
    int* assign_indices2;

    int index = 0;
    int j = 0;
    char** tokens;
    int max_length;
    char current_token[TOKEN_SIZE];

    bool in_quotes = false;
    bool meta = false;

    bool consider_equals = true;
    bool encountered_equals = false;
    charno type = BLANK;
    charno prev_type = BLANK;

    // int assign_indices[BUFSIZE];
    // int assign_count = 0;

    if (!(max_length = tokens_init(input, in, ex)))
    {
        error = PARSE_ERROR;
        return NULL;
    }   
    if (!(tokens = (char**) malloc(max_length * sizeof(char*))))
    {
        error = TOKENS_MEMORY_ERROR;
        return tokens;
    }
    //Lets assume the worst case scenario. All the tokens provided are variables that need to be expanded/assigned.
    if (!(var_indices2 = (tokenchar_pair*) malloc(max_length * sizeof(tokenchar_pair))))
    {
        error = VARINDICES_MEMORY_ERROR;
        return tokens;
    } 
    if (!(assign_indices2 = (int*) malloc(max_length * sizeof(int))))
    {
        error = MEMORY_ERROR;
        return NULL;
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

        //Lets store the index to the variable for later use (when we perform expansion)

        if (type == VARIABLE)
        {
            var_indices2[*var_length].token_index = index;
            var_indices2[(*var_length)++].char_index = j;
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

        if (type == EQUAL)
        {
            if (consider_equals && !encountered_equals && *assign_count == index)
            {
                assign_indices2[(*assign_count)++] = j;
                encountered_equals = true;

                if (*assign_count == BUFSIZE)
                {
                    error = BUFFER_OVERFLOW_ERROR;
                    return NULL;
                }
            }
            else
                type == NORMAL;
        }

        //Just like the token_init function, if a metacharacter terminated a string, then this is the end of a token
        //So lets save it in into tokens.
        if (meta && (prev_type == NORMAL || prev_type == QUOTE || prev_type == EQUAL))
        {
            if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    error = MEMORY_ERROR;
                    return tokens;
                }
            
            current_token[j++] = '\0';
            strncpy(tokens[index++], current_token, j);

            //If the function returns due to an error mid-way, then that means not all tokens had
            //dynamic memory allocated to them. Length keeps track of the current token_length so 
            //only the tokens with dynamic memory are freed afterwards.
            *length = index; 
            j = 0;

            if (encountered_equals)
                encountered_equals = false;
            else
                consider_equals = false;

        }

        else if (meta || (type == ESCAPE))
            continue;
        else
            current_token[j++] = input[i];
                
    }

    //If j is greater than 0,
    //that means there is still data in the current_token vector, which has not been added to tokens.
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
    *assign_indices = assign_indices2;
    ex->execute_start = *assign_count;

    if ((in->redirect_start && in->redirect_start <= *assign_count) ||
    (ex->pipe_start > 0 && ex->pipe_start <= *assign_count))
    {
        error = PARSE_ERROR;
        return NULL;
    }


    return tokens;

}

// Error handling and variable resetting.
void handle_error()
{
    if (error == SYSTEM_CALL_ERROR)
        perror("Error");
    
    else if (error)
        fprintf(stderr,"%s",error_msg[error-1]); 
}
void tokens_free(char** tokens, int* length)
{
    if (tokens)
    {
        for (int i = 0; i < *length; i++)
            free(tokens[i]);

        if (*length > 0)
            free(tokens);

        *length = -1;
        tokens = NULL;
    }

}
void var_indices_free(tokenchar_pair* var_indices, int* var_indices_len)
{
    if (var_indices)
    {
        free(var_indices);
        var_indices = NULL;
        *var_indices_len = -1;
    }

}
void assign_indices_free(int* assign_indices, int* assign_count)
{
    if (assign_indices)
    {
        free(assign_indices);
        assign_indices = NULL;
        *assign_count = -1;
    }
}
err reset_streams()
{
    error = NONE;

    /*If statements basically check wether the program was redirecting to/from a file in the first place
    and if so redirects back to standard input/output.*/

    if (stdin_fd > -1)
    {
        if (dup2(stdin_fd, STDIN_FILENO) < 0)
            error = SYSTEM_CALL_ERROR;
        
        if (close(stdin_fd))
            error = SYSTEM_CALL_ERROR;
        
        stdin_fd = -1;
    }
    if (stdout_fd > -1)
    {
        if (dup2(stdout_fd, STDOUT_FILENO) < 0)
            error = SYSTEM_CALL_ERROR;
        
        if (close(stdout_fd))
            error = SYSTEM_CALL_ERROR;
        
        stdout_fd = -1;
    }

    return error;
    
}
void reset_ex(redirect_ext* ex)
{
    ex->section[0].input = 0;
    ex->section[0].output = 0;
    ex->section[0].redirect_count = 0;
    ex->pipe_count = 0;
    ex->pipe_start = -1;
    ex->pipe_end = -1;
    ex->redirect_end = 0;
    ex->execute_start = 0;
}
void reset_in(redirect_int* in)
{
    in->redirect_count = 0;
    in->input_filename[0] = 0;
    in->output_filename[0] = 0;
    in->output_cat_filename[0] = 0;
    in->redirect_start = 0;
}
void free_vars()
{
    node* current_node;
    /*Loops through every shell variable and frees its previous one.
    This means by the end you'll have just the tail left.*/
    
    for (current_node = head; current_node; current_node = current_node->next)
    {
        if (current_node->prev) 
            node_delete(current_node->prev);
    }

    node_delete(tail);
}
void free_stack()
{
    char* popped_value;

    while(!pop(&popped_value))
        free(popped_value);

    free(stack[0]);
}

//Shell variables.
err init_vars(void)
{
    // Set every enviroment variable as a shell variable, with the bool env set to true.

    for (char** env_var = environ; *env_var; env_var++) 
    {
        char var[TOKEN_SIZE];
        strncpy(var,*env_var,TOKEN_SIZE);

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

    if (!node_search("TERMINAL") && (error = node_insert("TERMINAL", getenv("TERM")? getenv("TERM"):ttyname(0), true)))
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

    char cwd[TOKEN_SIZE];
    strncpy(cwd,getenv("HOME"),TOKEN_SIZE);

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

    if (chdir(cwd))
        return SYSTEM_CALL_ERROR;
    

    return NONE;

} 
bool vars_valid(const char* token, int j)
{
    if (!j && token[j] >= '0'  && token[j] <= '9')
        return false;
        
    if (    (token[j] >= '0'  && token[j] <= '9') || 
            (token[j] >= 'a'  && token[j] <= 'z') || 
            (token[j] >= 'A'  && token[j] <= 'Z') || 
            (token[j] == '_'))
        return true;

    return false;        
}
err expand_vars(char* tokens[TOKEN_SIZE], tokenchar_pair* var_indices, int var_indices_len, int m)
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

        for (int j = 0; j < end-1; j++)
        {
            //Ignoring the first character since it is the Variable Character
            if (!vars_valid(token+1,j))
            {
                end = j+1;
                token[end] = '\0';
                break;
            }      
        }
            
    }
    //Check if its in the shell variables

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
    
    

    if (equal) //If there was no match
        return VARIABLE_EXPANSION_ERROR;

    //For the remaining variables within the token, add to char_index: - len_of_var_name(including $) + value_len
    // Since the current token will be edited, all pointers to variable characters must be shifted.

    for (int j = m+1; j < var_indices_len; j++)
    {
        if (var_indices[j].token_index != token_index)
            break;
        
        var_indices[j].char_index += -(strlen(token) + offset-1) + value_len;
    }




    return NONE;
}
err assign_vars(char** tokens, int length, int i, int k)
{

    node* current_node;
    char current_token[TOKEN_SIZE];
    char key_value[2][TOKEN_SIZE];
    int j = 0;
    int current_token_len = strlen(tokens[i]);

    strcpy(current_token,tokens[i]);



    current_token[k] = 0;
    strcpy(key_value[0],current_token);
    strcpy(key_value[1], current_token+k+1);

    if (!strlen(key_value[0]) || !strlen(key_value[1]))
        return VARIABLE_ASSIGNMENT_ERROR;


    
    for (int j = 0; j < strlen(key_value[0]); j++)
    {
        if (!vars_valid(key_value[0],j))
            return VARIABLE_NAME_ERROR;
    }

    
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
    
    return NONE;


}

// Commands.
err execute_internal(char* args[TOKEN_SIZE], int arg_num, cmdno j)
{
    //Note that the first element of args is NOT the name of the command, its the first argument
    switch (j)
    {
        case EXIT_CMD:
        {
            //Maximum arg size is 1
            if (arg_num > 1)
                return INVALID_ARGS_ERROR;

            exit_program = true;
            //If the argument is provided, save the first argument into exit_value as an integer.
            if (arg_num == 1 && (error = str_to_int(&exit_value,args[0])))
            {
                exit_program = false;        
                return error;
            }

            return NONE;
                
        }break;
        case ECHO_CMD:
        {
            if (arg_num < 1)
                return INVALID_ARGS_ERROR;

            for(int i = 0; i < arg_num; i++)
                printf("%s ",args[i]);
            printf("\n");
            return NONE;
        }break;
        case CD_CMD:
        { 
           //Check if arg_num is valid and change the environment variable
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            struct stat sb;
            //Checks if argument is a existing directory
            if (stat(args[0], &sb) || !S_ISDIR(sb.st_mode))
                return NOT_A_DIR_ERROR;
            
            if (error = change_directory(args[0]))
                return error;
            return NONE;
        }break;
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
            return NONE;
        }break;
        case EXPORT_CMD:
        {
            if (!arg_num)
                return INVALID_ARGS_ERROR;

            for (int i = 0; i < arg_num; i++)
            {
                node* current_node;
                if (!(current_node = node_search(args[i])))
                    return NODE_NOT_FOUND_ERROR;

                if(error = node_export(current_node))
                    return error;
            }
            
            return NONE;
        }break;
        case UNSET_CMD:
        {
            if (!arg_num)
                return INVALID_ARGS_ERROR;

            for (int i = 0; i < arg_num; i++)
            {
                node* current_node;
                if (current_node = node_search(args[i]))
                {
                    if (error = node_delete(current_node))
                        return error;
                }
                else
                    return NODE_NOT_FOUND_ERROR;
            }

            return NONE;
        }break;
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

            return NONE;
        }break;
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

            return NONE;
        }break;
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

            free(popped_value);

            return NONE;
        }break;
        case DIRS_CMD:
        {
            if (arg_num)
                return INVALID_ARGS_ERROR;
            
            if (error = print_stack())
                return error;
            
            return NONE;
        }break;
        case SOURCE_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            if (!(fp = fopen(args[0],"r")))
                return SYSTEM_CALL_ERROR;

            return NONE;
        }break;
      
        default:
            return BUFFER_OVERFLOW_ERROR;
    }
}
err execute_external(char** tokens, redirect_ext* ex)
{
    int fd[ex->pipe_count*2]; //Holds file descriptors for piping
    int* current_fd = fd;
    int* previous_fd;

    pid_t pid;

    int output_file; //Index of a string in tokens where the file name for user specified ouput redirection can be found
    int fd_output; //File descriptor used to open the filename specified by ouput_file

    int input_file; //Index of a string in tokens where the file name for user specified input redirection can be found
    int fd_input; //File descriptor used to open the filename specified by input_file

    int status;
    int exitcode;

    char exitcode_str[10];
    
    error = NONE;

    char* args[BUFSIZE]; // Used to hold the arguments to pass to execvp
    int argc;

    for (int i = 0; i < ex->pipe_count+1; i++, previous_fd = current_fd, current_fd += 2)
    {
        // This for loops iterates over the a set of arguments seperated by pipes.
        // If the set of arguments has redirection files specified, they are not iterated over.
        argc = 0;
        for (int j = ex->execute_start; j < ex->pipe_indices[i]-ex->section[i].redirect_count; j++)
            args[argc++] = tokens[j];
        args[argc] = NULL;
        
        // The next iterations starts from  the end of the previous.
        ex->execute_start = ex->pipe_indices[i];
        

        if (i < ex->pipe_count && pipe(current_fd))
        {
            while (child_count) //Powerful ray of sunshine to obliterate any zombies
            {
                if (kill(child_pids[--child_count], SIGTERM))
                    error = SYSTEM_CALL_ERROR;
            }
            exit_program = 1;
            exit_value = 1;
            return SYSTEM_CALL_ERROR;
        }

        if ((pid = fork()) < 0)
        {
            while (child_count) //Powerful ray of sunshine to obliterate any zombies
            {
                if (kill(child_pids[--child_count], SIGTERM))
                    error = SYSTEM_CALL_ERROR;
            }
            exit_program = 1;
            exit_value = 1;
            return SYSTEM_CALL_ERROR;
        }


        if (!pid) // Child Process
        {
            // Hook output based on pipeline
            if (i < ex->pipe_count)
            {
                close(current_fd[0]);
                dup2(current_fd[1], STDOUT_FILENO);
                close(current_fd[1]);
                
            }
            //Hook output if the user specified a file
            if (output_file = ex->section[i].output)
            {
                //Opens file in rw or a+
                if ((fd_output = open(tokens[output_file], ex->section[i].cat? (O_CREAT | O_APPEND | O_RDWR):(O_CREAT | O_RDWR | O_TRUNC), S_IRWXU)) < 0)
                {
                    exit_program = true;
                    exit_value = 1;
                
                    return SYSTEM_CALL_ERROR;
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
            if (input_file = ex->section[i].input)
            {
                if ((fd_input = open(tokens[input_file], O_RDWR)) < 0)
                {
                    perror("File Error");
                    exit_program = true;
                    exit_value = 1;
                
                    return SYSTEM_CALL_ERROR;
                }

                dup2(fd_input, STDIN_FILENO);
                close(fd_input);
            }
            
            execvp(args[0], args);

            exit_program = true;
            exit_value = 1;
                
            return SYSTEM_CALL_ERROR;
        }
        else
            child_pids[child_count++] = pid;
        
        if (i) // All except the first iteration
        {
            close(previous_fd[0]);
            close(previous_fd[1]);
        }
    }
    while (wait(&status) > 0); //Wait for all child processes to terminate.
    child_count = 0; // All children have terminated

    if (error) // If this error is set, its because a SIGINT was sent.
        return error;

    if (WIFEXITED(status))
    {
        exitcode = WEXITSTATUS(status);
        //Converts int to string
        sprintf(exitcode_str, "%d", exitcode);

        //Stores exitcode in shell variable EXITCODE
        if (error = node_edit(node_search("EXITCODE"), exitcode_str))
            return error; 
    }
    else if (error = node_edit(node_search("EXITCODE"), "Abnormal termination")) 
        return error;

    return NONE;
}

//Redirects for internal commands.
err handle_redirect(char** tokens, charno state, int j, redirect_int* in)
{
    //What redirect is it?
    switch (state)
    {
        case INPUT: strcpy(in->input_filename, tokens[in->redirect_start+j]); break;
        case OUTPUT: strcpy(in->output_filename, tokens[in->redirect_start+j]); break;
        case OUTPUT_CAT: strcpy(in->output_cat_filename, tokens[in->redirect_start+j]); break;
        default: return INVALID_ARGS_ERROR;
    }
    return NONE;

}
err hook_streams(const redirect_int* in)
{
    int match = 1;
    int fd_input;
    int fd_output;
    int fd_output_cat;
    bool cat = false;
    error = NONE;


    for (int i = 0; i < in->redirect_count; i++)
    {
        if (in->redirect_indices[i] == OUTPUT)
            cat = false;
        else if (in->redirect_indices[i] == OUTPUT_CAT)
            cat = true;
    }
    
    // Do the input and output have to be redirected?
    // Opens files, links them with respective stream.
    //If any system call fails, the function exits

    if (in->input_filename[0])
    {
        if ((fd_input = open(in->input_filename, O_RDWR)) < 0
        || (stdin_fd = dup(STDIN_FILENO)) < 0
        || dup2(fd_input, STDIN_FILENO) < 0
        || (close(fd_input)))
            return SYSTEM_CALL_ERROR;
    } 
    

    if (cat)
    {
        if ((fd_output = open(in->output_cat_filename, O_CREAT | O_APPEND | O_RDWR, S_IRWXU)) < 0
        || (stdout_fd = dup(STDOUT_FILENO)) < 0
        || dup2(fd_output, STDOUT_FILENO) < 0
        || (close(fd_output)))
            return SYSTEM_CALL_ERROR;
    }
    else
    {
        if (in->output_filename[0]  && ((fd_output = open(in->output_filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU)) < 0
                                    || (stdout_fd = dup(STDOUT_FILENO)) < 0
                                    || dup2(fd_output, STDOUT_FILENO) < 0
                                    || (close(fd_output))))
            return SYSTEM_CALL_ERROR;
    }
    
    return error;
}

//Miscellaneous.
int contains_char(const char* string, char a)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == a)
            return i;
    }

    return -1;
}
err str_to_int(int* value, const char* string)
{
    char* end;

    long num = strtol(string, &end, 10);

    //Checks if the string was all numbers, and if this number is small enough to be stored in an int
    if ((string == end) || (num > INT_MAX) || (num < INT_MIN) || (*end != '\0')) 
        return INVALID_ARGS_ERROR;

    *value = (int) num;
    return NONE;
}
char* get_input_from_file(FILE* fp)
{
    char line[BUFSIZ];
    error = NONE;

    if (!(fgets(line,BUFSIZ-1,fp)))
        return NULL;

    int length = strlen(line);

    char* input;
    if (!(input = (char*) malloc(length)))
    {
        error = MEMORY_ERROR;
        return NULL;
    }
    
    //Make sure last character is a null terminator

    if (line[length-2] == '\r')
        line[length-2] = '\0';
    else
        line[length-1] = '\0';

    strcpy(input,line);


    return input;

}
bool contains_word(const char* input, const char* key)
{
    int input_len = strlen(input);
    int key_len = strlen(key);
    int j = 0;

    if (input_len < key_len)
        return false;

    for (int i = 0; i <= input_len-key_len; i++)
    {
        for (j = 0; j < key_len; j++)
        {
            if (input[i+j] != key[j])
                break;
        }    

        if (j == key_len) // Loop finished and did not break
            return true;
    }

    return false;


}
void SIGINT_handler(int signum)
{
    while (child_count)
    {
        if (kill(child_pids[--child_count], SIGTERM))
            error = SYSTEM_CALL_ERROR;
    }
}
