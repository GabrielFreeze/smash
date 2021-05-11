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
            if((in_quotes && prev_type != QUOTE) || (!in_quotes && prev_type == NORMAL))
                ++count;

            in_quotes = in_quotes? false:true;
        }

        if (in_quotes)
            goto end;

        if (type == META && prev_type == NORMAL)
            ++count;

        end:
            prev_type = type;
    }

    return (type == NORMAL)? count+1 : count;
    
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
void handle_error(int error)
{
    if (error == MEMORY_ERROR)
        fprintf(stderr, MEMORY_ERROR_MSG) ;

    if (error == BUFFER_OVERFLOW_ERROR)
        fprintf(stderr, BUFFER_ERROR_MSG,TOKEN_SIZE);

    if (error == PARSE_ERROR)
        fprintf(stderr, PARSE_ERROR_MSG);
        
    if (error == VARIABLE_ASSIGNMENT_ERROR)
        fprintf(stderr, VARIABLE_ASSIGNMENT_MSG);

    if (error == VARIABLE_EXPANSION_ERROR)
        fprintf(stderr, VARIABLE_EXPANSION_MSG);
}
char** tokens_get(char* input, int* length, int* error, tokenchar_pair** var_indices, int* var_indices_len)
{  
    
    int index = 0;
    int j = 0;
    int var_index = 0;
    char** tokens;
    tokenchar_pair* var_indices2;
    char current_token[TOKEN_SIZE];
    bool in_quotes = false;
    int type, prev_type = NONE;

    if ((*length = tokens_len(input)) <= 0)
    {
        *error = PARSE_ERROR;
        return NULL;
    }

    if (((tokens = (char**) malloc(*length * sizeof(char*))) == NULL) ||
    ((var_indices2 = (tokenchar_pair*) malloc(*length * sizeof(tokenchar_pair))) == NULL))
    {
        *error = MEMORY_ERROR;
        return NULL;
    }
 

    for (int i = 0; i < strlen(input); i++)
    {
        type = char_type(input, i);

        if (j == TOKEN_SIZE)
        {
            *error =  BUFFER_OVERFLOW_ERROR;
            tokens_free(tokens, index);
            return NULL;
        }
        if (prev_type == VARIABLE && type != NORMAL)
        {
            *error = VARIABLE_ASSIGNMENT_ERROR;
            tokens_free(tokens, index);
            return NULL;
        }
        
        if (type == VARIABLE)
        {
            var_indices2[var_index].token_index = index;
            var_indices2[var_index++].char_index = j;
        }

        if (type == QUOTE)
        {
            if ((in_quotes && prev_type != QUOTE) || (!in_quotes && prev_type == NORMAL))
            {

                if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    *error = MEMORY_ERROR;
                    tokens_free(tokens, index);
                    return NULL;
                }

                current_token[j++] = '\0';
                strncpy(tokens[index++], current_token, j);


                j = 0;  
            }
                
            in_quotes = in_quotes? false:true;
            goto end;
            
        }            

        if (in_quotes && type != ESCAPE && type != VARIABLE)
            type = NORMAL;


        if (type == META && prev_type == NORMAL)
        {
            if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    *error = MEMORY_ERROR;
                    tokens_free(tokens, index);
                    return NULL;
                }
            

            current_token[j++] = '\0';
            strncpy(tokens[index++], current_token, j);

            j = 0;
        }
        else if ((type == META) || (type == ESCAPE))
            goto end;
        else
            current_token[j++] = input[i];
            
        end:
            prev_type = type;     
    }

    if(in_quotes)
    {
        *error = PARSE_ERROR;
        tokens_free(tokens, index);
        return NULL;
    }

    if (j > 0) //If j is greater than 0 , that means there is data in the current_token vector
    {
        if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
        {   
            *error = MEMORY_ERROR;
            tokens_free(tokens, index);
            return NULL;
        }

        current_token[j++] = '\0';
        strncpy(tokens[index], current_token, j);
    }

    *var_indices = var_indices2;
    *var_indices_len = var_index;
    return tokens;

}
void tokens_free(char** tokens, int length)
{
    for (int i = 0; i < length; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}
int init_vars(void)
{
    byte error;

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

    if (error = node_insert("TERMINAL", ttyname(0) ,true))
        return error;

    //______________________________________________________________________

    if (error = node_insert("EXITCODE", "NONE" ,true))
        return error;


    return 0;


}
bool vars_valid(char* token, byte j)
{
    if (    (token[j] >= '0'  && token[j] <= '9') || 
            (token[j] >= 'a'  && token[j] <= 'z') || 
            (token[j] >= 'A'  && token[j] <= 'Z') || 
            (token[j] == '_'))
        return j;

    return false;        
}
int expand_vars(char* tokens[TOKEN_SIZE], tokenchar_pair* var_indices, int var_indices_len)
{
    int equal;
    char token[TOKEN_SIZE];
    char append[TOKEN_SIZE];

    byte end = 0;
    byte offset = 1;
    byte token_index;
    byte char_index; 
    byte value_len = 0;
    byte append_len = 0;
    byte original_len = 0;   

    for (byte i = 0; i < var_indices_len; i++)
    {

        offset = 1;
        token_index = var_indices[i].token_index;
        char_index = var_indices[i].char_index;

        strcpy(token, tokens[token_index] + char_index); //Ignoring all before the $
        end = strlen(token);

        if (token[1] == '{') //Using <Any Chars> ${...} <Any Chars> notation
        {
            for (byte l = 2; l < strlen(token); l++)
            {
                if (token[l] == '}')
                {
                    end = l;
                    break;
                }
            }
            if (end == 2)
                return VARIABLE_ASSIGNMENT_ERROR;
            
            token[end] = '\0';
            offset = 2;
        }
        else //Using  <Any Chars> $... <Illegeal Chars> notation
        {
            //Stop until you encounter an illegal character,
            for (byte j = 1; j < end; j++)
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

                for (byte k = 0; k < append_len+1; k++)
                    tokens[token_index][char_index+value_len+k] = append[k];

                break;
            }

        }

        if (equal)
            return VARIABLE_EXPANSION_ERROR;

        //For the remaining variables within the token, add to char_index: - var_name(including $) + value_len
        // Since the current token will be edited and all pointers to variable characters must be shifted.

        for (byte j = i+1; j < vars_len; j++)
        {
            if (var_indices[j].token_index != token_index)
                break;
            
            var_indices[j].char_index += -(strlen(token) + offset-1) + value_len;
        }

    }


    return 0;
}
byte node_insert(char* key, char* value, bool env)
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
    vars_len++;
    return 0;

}
node* node_search(char* key, node** prev_node)
{
    node* current_node;
    *prev_node = NULL;
    

    if ((current_node = head) == NULL)
        return NULL;
    
    while (strcmp(current_node->key,key))
        {
            if (current_node == NULL)
                return NULL;

            *prev_node = current_node;
            current_node = current_node->next;
        }
    return current_node;
}
byte node_delete(char* key)
{
    node* current_node;
    node* prev_node;

    if ((current_node = node_search(key, &prev_node)) == NULL)
        return NODE_NOT_FOUND_ERROR;

    //If previous node is Null, then the node to delete is the first one

    if (prev_node == NULL)
        head =  current_node->next;
    else
        prev_node->next = current_node->next;


    vars_len--;
    return 0;

}
void nodes_print()
{
    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
        printf("Key:[%s]\nValue:[%s]\n",current_node->key, current_node->value);  
}


