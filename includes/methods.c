#include "methods.h"
#include "linenoise-master/linenoise.h"
#include "config.h"
int tokens_len(char* string)
{
    int count = 0;
    bool in_quotes = false;
    int type, prev_type;

    for(int i = 0; i < strlen(string); i++)
    {
        type = char_type(string,i);
        prev_type = char_type(string,i-1);

        if (type == QUOTE)
        {
            if (!in_quotes && prev_type == NORMAL)
                ++count;


            if(in_quotes && prev_type != QUOTE)
                ++count;

            in_quotes = in_quotes? false:true;

        }
           
        if (in_quotes)
            continue;

        if (type == META && prev_type == NORMAL)
            ++count;  
    }

    return (type == NORMAL)? count+1 : count;
    
    
    



    // int count = 0;
    // int len = strlen(string);

    // for (int i = 1; i < len; i++)
    // {

    //     if (is_delim(string, i) && !is_delim(string, i-1))
    //         count ++;
            
    // }
    // int val = is_delim(string, len-1)? count : count+1;

    // printf("Number of tokens is: %d\n", val);
    // return val;
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


            if (string[j-1] == '\\')
                return NORMAL; //It is a metacharacter, but the one before it was an escape character, therefore it is treated as normal
            else
                return META; //It is a metacharacter, but the one before it was not an escape character
        }
    }
    

    if (string[j] == '\\')
    {
        if (is_escape(string,j))
            return ESCAPE; //It is the escape character
        else
            return NORMAL;
    }
        
    
        

    if (string[j] == '$')
        return VARIABLE; //It is the variable expansion character

    return NORMAL; //If its none of the above, then its just a normal character.





    // for (int i = 0; i < strlen(metacharacters); i++)
    // {   
    //     if (j == 0)
    //     {
    //         if (string[j] == metacharacters[i])
    //             return true;
    //         else
    //             continue;
    //     }

    //     if (string[j] == metacharacters[i] && string[j-1] != '\\')
    //         return true;           
    // }
    // return false;
}
bool is_escape(char* string, int upper)
{
    int lower = upper;
    int length = strlen(string);  
    int count = 0; 

    while (lower >= 0 && string[lower] == '\\')
    {
        lower --;
    }

    

    if (((upper-(lower+1))% 2 == 0))
        return true;
    else
        return false;


}

char** tokens_get(char* input, int* length, int* error)
{  
    
    int index = 0;
    int j = 0;
    char** tokens;
    char current_token[TOKEN_SIZE];
    bool in_quotes = false;
    int type, prev_type;

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
        prev_type = char_type(input, i-1);

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
            continue;
            
        }            

        if (in_quotes)
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
            continue;
        }
        else
            current_token[j++] = input[i];


            
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



    // int index = 0;
    // int j = 0;
    // char** tokens;
    // char current_token[ARG_SIZE];
    // bool in_quotes = false;

    // if ((*length = tokens_len(input)) == 0 || ((tokens = (char**) malloc(*length * sizeof(char*))) == NULL))
    //     return NULL;




    // if(!is_delim(input, 0) && input[0] != '\\')
    //     current_token[j++] = input[0];



    // for (int i = 1; i < strlen(input); i++)
    // {
    //     if (j == ARG_SIZE)
    //         return NULL;

    //     if(is_delim(input, i) && !is_delim(input, i-1))
    //     {
    //         tokens[index] = (char*) malloc(ARG_SIZE);
    //         current_token[j] = '\0';
    //         strncpy(tokens[index++], current_token, j+1);
            
    //         j = 0;
            
    //     } else if(is_delim(input, i) || input[i] == '\\')
    //         continue;

    //     else
    //         current_token[j++] = input[i];
    // }

    // if (j > 0) //If j is greater than 0 , that means there is data in the current_token vector,
    //             // which we need to place in tokens.
    // {
    //     tokens[index] = (char*) malloc(ARG_SIZE);
    //     current_token[j] = '\0';
    //     strncpy(tokens[index], current_token, j+1);
    // }
       
    // return tokens;
}

