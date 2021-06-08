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

//Tokenisation
int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, tokenchar_pair** var_indices, int* var_index);
bool is_deref(char* string, int upper);
bool is_meta(char* string, int j);

// Error handling and variable resetting.
int handle_error();
int tokens_free(char** tokens, int* length);
int var_indices_free(tokenchar_pair* var_indices, int* var_indices_len);
void reset_in();
void reset_ex();
void reset_streams();

//Shell variables.
bool is_var(char* token);
int init_vars(void);
bool vars_valid(char* token, int j);
int expand_vars(char** tokens, tokenchar_pair* var_indices, int var_indices_len, int m);
int assign_vars(char** tokens, int length, int i);

// Commands.
int execute_internal(char* args[TOKEN_SIZE], int arg_num, int j);
int execute_external(char** tokens, int token_num);

//Redirects for internal commands.
int handle_redirect(char* tokens[TOKEN_SIZE], int redirect_state, int j);
int hook_streams();

//Miscellanous
char* get_input_from_file(FILE* fp);
int contains_word(char* input, char* key);
int contains_char(char* string, char a);
int str_to_int(int* value, char* string);

//Linked List
int node_insert(char* key, char* value, bool env);
int node_delete(node* current_node);
node* node_search(char* key);
int node_edit(node* current_node, char* value);
int node_export(node* current_node);

//Stack
int print_stack();
int peek(char** value);
int pop(char** value);
int push(char* value);
bool is_full();
int change_directory(char* value);
