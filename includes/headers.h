#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "config.h"


int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, int* error, tokenchar_pair** var_indices, int* var_indices_len);
bool is_deref(char* string, int upper);
int handle_error(int error);
int tokens_free(char** tokens, int length);
int var_indices_free(tokenchar_pair* var_indices);
bool is_var(char* token);
int init_vars(void);
bool vars_valid(char* token, int j);
int expand_vars(char** tokens, tokenchar_pair* var_indices, int var_indices_len, int m);
int node_insert(char* key, char* value, bool env);
int node_delete(char* key);
node* node_search(char* key);
int assign_vars(char** tokens, int length, int i);
int contains_char(char* string, char a);