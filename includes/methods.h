#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>



int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, int* error);
bool is_deref(char* string, int upper);
void handle_error(int error);
