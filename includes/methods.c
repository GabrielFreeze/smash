#include "methods.h"
#include "linenoise-master/linenoise.h"
extern char* metacharacters;

int tokenlen(char* string)
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

int get_tokens(char** args, char* input)
{

    return 0;
}
