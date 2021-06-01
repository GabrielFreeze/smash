#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include "limits.h"
#include "config.h"
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, tokenchar_pair** var_indices, int* var_indices_len);
bool is_deref(char* string, int upper);
bool is_meta(char* string, int j);
int handle_error();
int tokens_free(char** tokens, int length);
int var_indices_free(tokenchar_pair* var_indices);
bool is_var(char* token);
int init_vars(void);
bool vars_valid(char* token, int j);
int expand_vars(char** tokens, tokenchar_pair* var_indices, int var_indices_len, int m);
int node_insert(char* key, char* value, bool env);
int node_delete(node* current_node);
node* node_search(char* key);
int node_edit(node* current_node, char* value);
int node_export(node* current_node);
int assign_vars(char** tokens, int length, int i);
int contains_char(char* string, char a);
int tokens_parse(char* tokens[TOKEN_SIZE], int token_num);
int str_to_int(int* value, char* string);
int execute_internal(char* args[TOKEN_SIZE], int arg_num, int j);
int execute_external(char* args[TOKEN_SIZE], int arg_num);
int print_stack();
int peek(char** value);
int pop(char** value);
int push(char* value);
bool is_full();
int change_directory(char* value);
char* get_input_from_file(FILE* fp);
int contains_word(char* input, char* key);
int redirect(char* tokens[TOKEN_SIZE], int redirect_state, int j);