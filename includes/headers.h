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
void handle_error(int error);
void tokens_free(char** tokens, int length);
bool is_var(char* token);
int init_vars(void);
bool vars_valid(char* token, byte j);
int expand_vars(char** tokens, tokenchar_pair* var_indices, int var_indices_len);
byte node_insert(char* key, char* value, bool env);
byte node_delete(char* key);
node* node_search(char* key);
byte assign_vars(char** tokens, byte length);
