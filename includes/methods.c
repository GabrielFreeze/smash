#include "methods.h"
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
char** tokens_get(char* input, int* length, int* error, int** var_indices, int* var_indices_len)
{  
    
    int index = 0;
    int j = 0;
    int var_count = 0;
    int var_index = 0;
    char** tokens;
    int* var_indices2;
    char current_token[TOKEN_SIZE];
    bool in_quotes = false;
    int type, prev_type = NONE;

    if ((*length = tokens_len(input)) <= 0)
    {
        *error = PARSE_ERROR;
        return NULL;
    }

    if (((tokens = (char**) malloc(*length * sizeof(char*))) == NULL) ||
    ((var_indices2 = (int*) malloc(*length * sizeof(int))) == NULL))
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
        
        if (type == VARIABLE && ++var_count > 1)
        {
            *error = VARIABLE_ASSIGNMENT_ERROR;
            tokens_free(tokens, index);
            return NULL;
        }

        if (type == VARIABLE && j > 0)
        {
            *error = VARIABLE_ASSIGNMENT_ERROR;
            tokens_free(tokens, index);
            return NULL;
        }
        
        if (in_quotes && type == VARIABLE)
        {
            *error = VARIABLE_ASSIGNMENT_ERROR;
            tokens_free(tokens, index);
            return NULL;
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
            
            if (var_count)
            {
                var_indices2[var_index++] = index;
            }


            current_token[j++] = '\0';
            strncpy(tokens[index++], current_token, j);


            var_count = 0;
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
        if (var_count)
        {
            var_indices2[var_index++] = index;
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
    vars_len = INIT_VARS_LEN;
    for (int i = 0; i < vars_len; i++)
    {
        variables[i].is_valid = true;
        variables[i].env = true;
    } 


    strcpy(variables[0].key, "PATH\0");

    char* path;

    if ((path = getenv("PATH")) == NULL)
        return EXIT_FAILURE;

    strcpy(variables[0].value, path);

    //______________________________________________________________________

    strcpy(variables[1].key, "PROMPT\0");
    strcpy(variables[1].value, "init");
    //______________________________________________________________________

    strcpy(variables[2].key, "CWD\0");

    char cwd[VALUE_SIZE];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        return EXIT_FAILURE;

    strcpy(variables[2].value, cwd);

    //______________________________________________________________________
    strcpy(variables[3].key, "HOME\0");

    char* home;

    if ((home  = getenv("HOME")) == NULL)
        return EXIT_FAILURE;

    strcpy(variables[3].value, home);
    //______________________________________________________________________

    strcpy(variables[4].key, "USER\0");

    char* user;

    if ((user  = getenv("USER")) == NULL)
        return EXIT_FAILURE;

    strcpy(variables[4].value, user);

    //______________________________________________________________________

        
    strcpy(variables[6].key, "SHELL\0");
    char shell[VALUE_SIZE];

    if(readlink("/proc/self/exe", shell, VALUE_SIZE) == -1)
        return EXIT_FAILURE;
    
    strcpy(variables[6].value, shell);

    //______________________________________________________________________

    strcpy(variables[7].key, "TERMINAL\0");
    char* term;

    if ((term = ttyname(0)) == NULL)
        return EXIT_FAILURE;

    strcpy(variables[7].value, term);

    //______________________________________________________________________

    strcpy(variables[8].key, "EXITCODE\0");
    strcpy(variables[8].value,"NULL\0");

    return 0;


}
int expand_vars(char* tokens[TOKEN_SIZE], int* var_indices, int var_indices_len)
{
    int equal;
    char token[TOKEN_SIZE];
    char append[TOKEN_SIZE];

    byte end = 0;
    byte offset = 1;

    for (byte i = 0; i < var_indices_len; i++)
    {

        offset = 1;

        strcpy(token, tokens[var_indices[i]]);
        end = strlen(token);

        if (token[1] == '{')
        {
            for (byte l = 2; l < strlen(token); l++)
            {
                if (token[l] == '}')
                {
                    end = l;
                    break;
                }
            }
            if (!end || end == 2)
                return VARIABLE_ASSIGNMENT_ERROR;
            
            token[end] = '\0';
            offset = 2;
        }
            
        for (byte j = 0; j < vars_len; j++)
        {
            if ((equal = strcmp(token + offset, variables[j].key)) == 0)
            {
                strcpy(append, tokens[var_indices[i]] + end + offset-1);

                strcpy(tokens[var_indices[i]], variables[j].value);
                byte append_len = strlen(append);
                byte value_len = strlen(variables[j].value);

                for (byte k = 0; k < append_len+1; k++)
                    tokens[var_indices[i]][value_len+k] = append[k];

                break;
            }

        }
        if (equal)
            return VARIABLE_EXPANSION_ERROR;
    }


    return 0;
}
