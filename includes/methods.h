#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>



int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, int* error, int** var_indices, int* var_indices_length);
bool is_deref(char* string, int upper);
void handle_error(int error);
void tokens_free(char** tokens, int length);
bool is_var(char* token);
int init_variables(void);
int test(int** arr);
