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
        {
            // if((in_quotes && prev_type != QUOTE) || (!in_quotes && prev_type == NORMAL))
            //     ++count;

            in_quotes = in_quotes? false:true;
        }

        if (in_quotes)
            goto end;

        if (type == META && (prev_type == NORMAL || prev_type == QUOTE))
            ++count;

        end:
            prev_type = type;
    }

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
        if (prev_type == VARIABLE && type != NORMAL)
        {
            *error = VARIABLE_DECLARATION_ERROR;
            return tokens;
        }
        
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

        if (in_quotes && type != ESCAPE && type != VARIABLE)
            type = NORMAL;


        if (type == META && (prev_type == NORMAL || prev_type == QUOTE))
        {
            if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    *error = MEMORY_ERROR;
                    return tokens;
                }
            

            current_token[j++] = '\0';
            strncpy(tokens[index++], current_token, j);

            *length = index;
            j = 0;
        }
        else if ((type == META) || (type == ESCAPE))
            continue;
        else
            current_token[j++] = input[i];
                
    }

    if(in_quotes)
    {
        *error = PARSE_ERROR;
        return tokens;
    }

    if (j > 0) //If j is greater than 0 , that means there is data in the current_token vector
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

    if (error = node_insert("PATH", "/usr/bin:/bin:/usr/local/bin" ,true))
        return error;
    //______________________________________________________________________
    
    if (error = node_insert("PROMPT", "init" ,true))
        return error;

    //______________________________________________________________________

    char cwd[VALUE_SIZE];

    if (error = node_insert("CWD", getcwd(cwd, sizeof(cwd)) ,true))
        return error;

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
    int equal;
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
        

    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
    {
        if ((equal = strcmp(token + offset, current_node->key)) == 0)
        {
            strcpy(append, tokens[token_index] + char_index + end + offset-1);

            strcpy(tokens[token_index] + char_index, current_node->value);
            append_len = strlen(append);
            value_len = strlen(current_node->value);

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
    if ((new_node = (node*) malloc(sizeof(node))) == NULL)
        return MEMORY_ERROR;

    if (key == NULL || value == NULL)
        return NODE_ASSIGNMENT_ERROR;


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

    if ((current_node = node_search(key)) == NULL)
        return NODE_NOT_FOUND_ERROR;

    //If previous node is Null, then the node to delete is the first one

    if (current_node->prev == NULL)
        head =  current_node->next;
    else
        current_node->prev->next = current_node->next;


    vars_len--;
    return 0;

}
void nodes_print()
{
    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
        printf("Key:[%s]\nValue:[%s]\n",current_node->key, current_node->value);  
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

    if (j == 0) //If there isnt an =, exit but do not raise an error.
        return 0;

    if (j != 2) //This means that there was more than one =
        return VARIABLE_ASSIGNMENT_ERROR;
    
    //Assign current_node to node with key == key_value[0] if it exists,
    // otherwise,  create it with the values;
    if ((current_node = node_search(key_value[0])) == NULL)
    {
        if (error = node_insert(key_value[0], key_value[1], false))  
            return error;
    }
    else
    {
        strcpy(current_node->key, key_value[0]);
        strcpy(current_node->value, key_value[1]);
    }


    
    return 0;

    //Find token with =.
    //Split LHS(key) and RHS(value)
    //Does key already exist?
    //      N = Create it
    //
    //Assign value to key.      
    //bool env is false by default since this is a shell variable.


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

