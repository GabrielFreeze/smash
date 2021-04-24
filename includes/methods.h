#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>



int tokens_len(char* string);
bool is_delim(char* string, int j);
char** tokens_get(char* input, int* length); 