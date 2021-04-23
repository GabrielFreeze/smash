#include "methods.h"
#include "linenoise-master/linenoise.h"
#include "config.h"
int tokens_len(char* string)
{
    int count = is_delim(string[0])? -1 : 0;
    char prev_char = '\0';
    int len = strlen(string);

    for (int i = 0; i < len; i++)
    {
        if (is_delim(string[i]) && !is_delim(prev_char))
            count ++;
            
        prev_char = string[i];
    }
    return is_delim(string[len-1])? count : count+1;
}

bool is_delim(char a)
{
    for (int i = 0; i < strlen(metacharacters); i++)
    {
        if (a == metacharacters[i])
            return true;           
    }
    return false;
}

char** tokens_get(char* input, int* length)
{  
    char** tokens;
    char* value;
    *length = tokens_len(input);
    
    int index = 0;

    if ((tokens = (char**) malloc(*length * sizeof(char*))) == NULL)
        return NULL;


    for (char* current_token = strtok(input, metacharacters); current_token != NULL; current_token = strtok(NULL, metacharacters))
    {
        tokens[index] = (char*) malloc(ARG_SIZE);
        strcpy(tokens[index], current_token);
        index++;
    }

    return tokens;
}
