#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>



int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, int* error);
int is_escape(char* string, int upper);
