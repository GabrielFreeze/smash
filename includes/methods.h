#include <string.h>
#include <stdio.h>
#include <stdbool.h>



int tokenlen(char* string);
bool is_delim(char a);
int get_tokens(char** args, char* input); //Returns -1 on error, else length of args. Args will be dynamically allocated.