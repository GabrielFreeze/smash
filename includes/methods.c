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
            if((in_quotes && !(prev_type == QUOTE || prev_type == ESCAPE)) || (!in_quotes && prev_type == NORMAL))
                ++count;

            in_quotes = in_quotes? false:true;
        }

        if (in_quotes){
            prev_type = type;
            continue;
        }


        if (type == META && prev_type == NORMAL)
            ++count;

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
            return QUOTE; //It is a quoting character
    }

    for (int i = 0; i < strlen(metacharacters); i++)
    {
        if (string[j] == metacharacters[i])
        {
            if (j == 0)
                return META; //Its the first character, and its meta.

            else if (string[j-1] == '\\')
                return NORMAL; //It is a metacharacter, but the one before it was an escape character, therefore it is treated as normal
            else
                return META; //It is a metacharacter, but the one before it was not an escape character
        }
    }
    
    if (string[j] == '\\')
        return is_escape(string,j);        
    
    if (string[j] == '$')
        return VARIABLE; //It is the variable expansion character

    return NORMAL; //If its none of the above, then its just a normal character.

}
int is_escape(char* string, int upper)
{
    int lower = upper;

    while (lower >= 0 && string[lower] == '\\')
        lower --;

    return ((upper-lower) % 2 == 0)? NORMAL:ESCAPE;

}
char** tokens_get(char* input, int* length, int* error)
{  
    
    int index = 0;
    int j = 0;
    char** tokens;
    char current_token[TOKEN_SIZE];
    bool in_quotes = false;
    int type, prev_type = NONE;

    if ((*length = tokens_len(input)) == 0)
    {
        *error = LENGTH_ERROR;
        return NULL;
    }

    if (((tokens = (char**) malloc(*length * sizeof(char*))) == NULL))
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
            return NULL;
        }
            
        if (type == QUOTE)
        {
            if ((in_quotes && prev_type != QUOTE) || (!in_quotes && prev_type == NORMAL))
            {
                if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    *error = MEMORY_ERROR;
                    return NULL;
                }

                current_token[j++] = '\0';
                strncpy(tokens[index++], current_token, j);
            
                j = 0;  
            }
                
            in_quotes = in_quotes? false:true;
            prev_type = type;
            continue;
            
        }            

        if (in_quotes && type != ESCAPE && type != VARIABLE)
            type = NORMAL;


        if (type == META && prev_type == NORMAL)
        {
            if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
                {   
                    *error = MEMORY_ERROR;
                    return NULL;
                }
            current_token[j++] = '\0';
            strncpy(tokens[index++], current_token, j);
            
            j = 0;
        }
        else if ((type == META) || (type == ESCAPE))
        {
            prev_type = type;
            continue;
        }
        else
            current_token[j++] = input[i];
            

        prev_type = type;
    }

    if (j > 0) //If j is greater than 0 , that means there is data in the current_token vector
    {
        if ((tokens[index] = (char*) malloc(TOKEN_SIZE)) == NULL)
        {   
            *error = MEMORY_ERROR;
            return NULL;
        }
        current_token[j++] = '\0';
        strncpy(tokens[index], current_token, j);
    }

    return tokens;

}

