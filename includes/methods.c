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
    bool check = false;
    char** tokens;
    char current_token[ARG_SIZE] = {'\0'};

    *length = tokens_len(input);
    
    if (*length == 0)
        return NULL;

    if ((tokens = (char**) malloc(*length * sizeof(char*))) == NULL)
        return NULL;

    for (int i = 0; i < strlen(input); i++)
    {
        check = true;

        if (i == 0)
        {
            if(is_delim(input, i))
                continue;
            else
            {
            printf("!");
            current_token[j++] = input[i];
            }

        }

        else if(is_delim(input, i) && !is_delim(input, i-1))
        {
            tokens[index] = (char*) malloc(ARG_SIZE);
            strncpy(tokens[index++], current_token, j);
            
            j = 0;

            printf("Token [%s] added.\n",tokens[index-1]);
            
        }else
        {
            printf("!");

            current_token[j++] = input[i];
        }
    }

    if (j > 0 && check)
    {
        tokens[index] = (char*) malloc(ARG_SIZE);
        strncpy(tokens[index], current_token, j); 
        printf("Token [%s] added.\n",tokens[index]);

    }
       


    return tokens;
}

    // for (char* current_token = strtok(input, metacharacters); current_token != NULL; current_token = strtok(NULL, metacharacters))
    // {
    //     tokens[index] = (char*) malloc(ARG_SIZE);
    //     strcpy(tokens[index], current_token);
    //     index++;
    // }