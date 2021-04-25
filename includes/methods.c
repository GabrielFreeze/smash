#include "methods.h"
#include "linenoise-master/linenoise.h"
#include "config.h"
int tokens_len(char* string)
{
    int count = 0;
    int len = strlen(string);

    for (int i = 1; i < len; i++)
    {

        if (is_delim(string, i) && !is_delim(string, i-1))
            count ++;
            
    }
    int val = is_delim(string, len-1)? count : count+1;

    printf("Number of tokens is: %d\n", val);
    return val;
}

bool is_delim(char* string, int j)
{

    for (int i = 0; i < strlen(metacharacters); i++)
    {   
        if (j == 0)
        {
            if (string[j] == metacharacters[i])
                return true;
            else
                continue;
        }

        if (string[j] == metacharacters[i] && string[j-1] != '\\')
            return true;           
    }
    return false;
}


char** tokens_get(char* input, int* length)
{  

    int index = 0;
    int j = 0;
    char** tokens;
    char current_token[ARG_SIZE];

    if ((tokens = (char**) malloc(*length * sizeof(char*))) == NULL || (*length = tokens_len(input)) == 0)
        return NULL;

    if(!is_delim(input, 0) && input[0] != '\\')
        current_token[j++] = input[0];

    for (int i = 1; i < strlen(input); i++)
    {
        if (j == ARG_SIZE)
            return NULL;

        if(is_delim(input, i) && !is_delim(input, i-1))
        {
            tokens[index] = (char*) malloc(ARG_SIZE);
            current_token[j] = '\0';
            strncpy(tokens[index++], current_token, j+1);
            
            j = 0;
            
        } else if(is_delim(input, i) || input[i] == '\\')
            continue;

        else
            current_token[j++] = input[i];
    }

    if (j > 0) //If j is greater than 0 , that means there is data in the current_token vector,
                // which we need to placce in tokens.
    {
        tokens[index] = (char*) malloc(ARG_SIZE);
        current_token[j] = '\0';
        strncpy(tokens[index], current_token, j+1); 
    }
       
    return tokens;
}

    // for (char* current_token = strtok(input, metacharacters); current_token != NULL; current_token = strtok(NULL, metacharacters))
    // {
    //     tokens[index] = (char*) malloc(ARG_SIZE);
    //     strcpy(tokens[index], current_token);
    //     index++;
    // }