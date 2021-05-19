#include "headers.h"
#include "linenoise-master/linenoise.h"
#include "config.h"
int tokens_len(char* string)
{
    int count = 0;
    bool in_quotes = false;
    int type, prev_type = NONE;

    for(int i = 0; i < strlen(string); i++)
    {
        type = char_type(string,i);

        if (type == QUOTE)
            in_quotes = in_quotes? false:true;
        
        //If: The character is a meta character, and its previous characters are either normal or quote
        //Then: This means a token just ended, so increment token_count
        if (!in_quotes && type == META && (prev_type == NORMAL || prev_type == QUOTE))
            ++count;

            prev_type = type;
    }
    //Since the only way a token is considered is if a string of characters is terminated with a meta character...
    //Check if the last character was META or not. If it wasn't meta, that means there is a token that we need to consider
    return (type == NORMAL || type == QUOTE)? count+1 : count;
    
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

    for (int i = 0; i < strlen(metacharacters); i++)
    {
        if (string[j] == metacharacters[i])
        {
            if (is_deref(string, j))
                return NORMAL;
            else
                return META;
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

    return NORMAL; //If its none of the above, then its just a normal character.

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
int handle_error(int error)
{
    if (error == 0)
        return 0;
    if (error == MEMORY_ERROR)
        fprintf(stderr, MEMORY_ERROR_MSG) ;

    if (error == BUFFER_OVERFLOW_ERROR)
        fprintf(stderr, BUFFER_ERROR_MSG,TOKEN_SIZE);

    if (error == PARSE_ERROR)
        fprintf(stderr, PARSE_ERROR_MSG);
        
    if (error == VARIABLE_DECLARATION_ERROR)
        fprintf(stderr, VARIABLE_DECLARATION_MSG);

    if (error == VARIABLE_EXPANSION_ERROR)
        fprintf(stderr, VARIABLE_EXPANSION_MSG);

    if (error == VARIABLE_ASSIGNMENT_ERROR)
        fprintf(stderr, VARIABLE_ASSIGNMENT_MSG);
    
    if (error == NODE_NOT_FOUND_ERROR)
        fprintf(stderr, NODE_NOT_FOUND_MSG);

    if (error == NODE_ASSIGNMENT_ERROR)
        fprintf(stderr, NODE_ASSIGNMENT_MSG);

    return 0;
    
}
char** tokens_get(char* input, int* length, int* error, tokenchar_pair** var_indices, int* var_indices_len)
{  
    
    int index = 0;
    int j = 0;
    int var_index = 0;
    char** tokens;
    int max_length;
    tokenchar_pair* var_indices2;
    char current_token[TOKEN_SIZE];
    bool in_quotes = false;
    int type, prev_type = NONE;

    if ((max_length = tokens_len(input)) <= 0)
    {
        *error = PARSE_ERROR;
        return NULL;
    }

    if ((tokens = (char**) malloc(max_length * sizeof(char*))) == NULL)
    {
        *error = TOKENS_MEMORY_ERROR;
        return tokens;
    }
    //Lets assume the worst case scenario. All the tokens provided are variables that need to be expanded.
    if((var_indices2 = (tokenchar_pair*) malloc(max_length * sizeof(tokenchar_pair))) == NULL)
    {
        *error = VARINDICES_MEMORY_ERROR;
        return tokens;
    }
 

    for (int i = 0; i < strlen(input); i++)
    {
        prev_type = type;
        type = char_type(input, i);

        if (j == TOKEN_SIZE)
        {
            *error =  BUFFER_OVERFLOW_ERROR;
            return tokens;
        }

        //Eg: $ followed by a META, or $""
        if (prev_type == VARIABLE && type != NORMAL)
        {
            *error = VARIABLE_DECLARATION_ERROR;
            return tokens;
        }
        
        //Lets store the index to the variable for later use (when we perform expansion)
        if (type == VARIABLE)
        {
            var_indices2[var_index].token_index = index;
            var_indices2[var_index++].char_index = j;
        }

        if (type == QUOTE)
        {       
            in_quotes = in_quotes? false:true;
            continue;   
        }            

        //Anything in quotes except for special shell characters should be treated as is, devoid of any special meaning.
        if (in_quotes && type != ESCAPE && type != VARIABLE)
            type = NORMAL;

        //Just like the token_len function, if its a metacharacter terminated a string, then this is the end of a token
        //So lets save it in into tokens.
        if (type == META && (prev_type == NORMAL || prev_type == QUOTE))
        {
            if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    *error = MEMORY_ERROR;
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

        else if ((type == META) || (type == ESCAPE))
            continue;
        else
            current_token[j++] = input[i];
                
    }

    //If we traverse the whole input string and we are still in quotes, then that means the user used quotes incorrectly.
    if(in_quotes)
    {
        *error = PARSE_ERROR;
        return tokens;
    }

    //If j is greater than 0 , that means there is still data in the current_token vector, which has not been added to tokens.
    if (j > 0) 
    {
        if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
        {   
            *error = MEMORY_ERROR;
            return tokens;
        }
        current_token[j++] = '\0';
        strncpy(tokens[index++], current_token, j);
        *length = index;
    }
    
    *error = 0;
    *var_indices = var_indices2;
    *var_indices_len = var_index;

    return tokens;

}
int tokens_free(char** tokens, int length)
{
    if (tokens == NULL)
        return 0;

    for (int i = 0; i < length; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
    return 0;
}
int var_indices_free(tokenchar_pair* var_indices)
{
    if (var_indices == NULL)
        return 0;
    
    free(var_indices);

    return 0;
}
int init_vars(void)
{
    int error;

    //______________________________________________________________________
    
    if (error = node_insert("PROMPT", "init>" ,true))
        return error;

    //______________________________________________________________________

    char cwd[VALUE_SIZE];



    if (error = node_insert("CWD", getenv("HOME") ,true))
        return error;
    
    if (setenv("PWD",getenv("HOME"),1))
        return errno;

    //______________________________________________________________________
    
    if (error = node_insert("HOME", getenv("HOME") ,true))
        return error;
    //______________________________________________________________________

    if (error = node_insert("USER", getenv("USER") ,true))
        return error;

    //______________________________________________________________________

    char shell[VALUE_SIZE];

    if(readlink("/proc/self/exe", shell, VALUE_SIZE) == -1)
        return NODE_ASSIGNMENT_ERROR;
    
    if (error = node_insert("SHELL", shell ,true))
        return error;
    
    //______________________________________________________________________

    if (error = node_insert("TERMINAL", getenv("TERM") ,true))
        return error;

    //______________________________________________________________________

    if (error = node_insert("EXITCODE", "NONE" ,true))
        return error;


    if (error = node_insert("PATH", getenv("PATH") ,true))
        return error;

    return 0;


} 
bool vars_valid(char* token, int j)
{
    if (    (token[j] >= '0'  && token[j] <= '9') || 
            (token[j] >= 'a'  && token[j] <= 'z') || 
            (token[j] >= 'A'  && token[j] <= 'Z') || 
            (token[j] == '_'))
        return j;

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
int node_insert(char* key, char* value, bool env)
{
    node* new_node;

    if (key == NULL || value == NULL)
        return NODE_ASSIGNMENT_ERROR;

    if  (!((new_node = (node*) malloc(sizeof(node))) &&
        ((new_node->key = (char*) malloc(strlen(key))) && 
        (new_node->value = (char*) malloc(strlen(value))))))
        return MEMORY_ERROR;


    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->env = env;
    new_node->next = head;
    head = new_node;
    head->prev= new_node;

    vars_len++;
    

    return 0;

}
node* node_search(char* key)
{
    node* current_node;
    

    if ((current_node = head) == NULL)
        return NULL;
    
    while (current_node != NULL && strcmp(current_node->key,key))
        current_node = current_node->next;

    return current_node;
}
int node_delete(char* key)
{
    node* current_node;
    node* prev_node;

    if (!(current_node = node_search(key)))
        return NODE_NOT_FOUND_ERROR;

    //If previous node is Null, then the node to delete is the first one

    if (current_node->prev)
        current_node->prev->next = current_node->next;
    else
        head = current_node->next;


    vars_len--;
    return 0;

}
int node_edit(char* key, char* value)
{
    node* current_node;

    if (!(current_node = node_search(key)))
        return NODE_NOT_FOUND_ERROR;
    
    current_node = (node*) realloc(current_node, strlen(value));
    strcpy(current_node->value,value);

    return 0;
}
void nodes_print()
{
    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
        printf("%s=%s\n",current_node->key, current_node->value);  
}
int assign_vars(char** tokens, int length, int i)
{

    node* current_node;
    char current_token[TOKEN_SIZE];
    char key_value[2][TOKEN_SIZE];
    int j = 0;
    int current_token_len = strlen(tokens[i]);
    int error;

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

    
    //If: The variable doesn't exist
    //Then: Create it and assign it the values given.
    //Else(if it does exists): Change the value of the variable. 
    if ((current_node = node_search(key_value[0])) == NULL)
    {
        if (error = node_insert(key_value[0], key_value[1], false))  
            return error;
    }
    else
    {
        if (error = node_edit(key_value[0], key_value[1]))
            return error;
    }
    
    return 0;


}
int contains_char(char* string, char a)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == a)
            return i;
    }

    return -1;
}
int tokens_parse(char* tokens[TOKEN_SIZE], int token_num)
{
    int i;
    int match = 1;
    int error;
    for(i = 0; i < internal_commands_len && (match = strcmp(tokens[0],internal_commands[i])); i++);
    
    if(match)//It is an external command
        return 0;

    else //It is an internal command
    {
        if (error = execute_internal(tokens+1, token_num-1, i))
            return error;
        //The loop breaks upon finding a match, therefore i should point to the internal command

    }
        
    return 0;
}
int execute_internal(char* args[TOKEN_SIZE], int arg_num, int j)
{
    //Note that the first element of args is NOT the name of the command, its the first argument
    int error;
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
            
        }
        case SHOWVAR_CMD:
        {   
            if (arg_num > 1)
                return INVALID_ARGS_ERROR;

            if (!arg_num) //Print all shell vars
                nodes_print();
            else //Print just one shell var
            {
                node* current_node;
                if ((current_node = node_search(args[0])) == NULL)
                    return NODE_NOT_FOUND_ERROR;
                printf("%s=%s\n",current_node->key,current_node->value);
            }

            return 0;
        }
        case EXPORT_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            node* current_node;
            if ((current_node = node_search(args[0])) == NULL)
                return NODE_NOT_FOUND_ERROR;

            if (setenv(current_node->key,current_node->value,69))
                return SYSTEM_CALL_ERROR;
            
            return 0;
        }
        case UNSET_CMD:
        {
            if (arg_num != 1)
                return INVALID_ARGS_ERROR;

            if (error = node_delete(args[0]))
                return error;

            if (unsetenv(args[0]))
                return SYSTEM_CALL_ERROR;

            return 0;
        }
        case SHOWENV_CMD:
        {
        //  for (char** var = environ; *var; var++)
        //         printf("%s\n",*var);

        //     return 0;
        }
        case PUSHD_CMD:
        {
            
        }
        case POPD_CMD:
        {
            
        }
        case DIRS_CMD:
        {
            
        }
        case SOURCE_CMD:
        {
            
        }
      
        default:
            return INVALID_FUNCTION_USE_ERROR;
    }
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



