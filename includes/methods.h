#pragma once
#define TOKEN_SIZE 2000
#define KEY_SIZE 2000
#define VALUE_SIZE 2000
#define BUFSIZE 256
#define ERRORS_LENGTH 20
#define INTERNAL_COMMANDS_LEN 11

#define NONE -1
#define NORMAL 0
#define META 1
#define ESCAPE 2
#define VARIABLE 3
#define QUOTE 4
#define EQUAL 5
#define OUTPUT 6
#define OUTPUT_CAT 7
#define INPUT 8
#define PIPE 9


#define MEMORY_ERROR 1
#define BUFFER_OVERFLOW_ERROR 2
#define PARSE_ERROR 3
#define VARIABLE_DECLARATION_ERROR 4
#define VARIABLE_EXPANSION_ERROR 5
#define VARIABLE_ASSIGNMENT_ERROR 6
#define VARIABLE_NAME_ERROR 7
#define NODE_NOT_FOUND_ERROR 8
#define NODE_ASSIGNMENT_ERROR 9
#define STACK_FULL_ERROR 10
#define STACK_EMPTY_ERROR 11
#define TOKENS_MEMORY_ERROR 12
#define VARINDICES_MEMORY_ERROR 13
#define INVALID_ARGS_ERROR 14
#define ENV_VARIABLE_NOT_FOUND_ERROR 15
#define ENV_VARIABLE_ASSIGNMENT_ERROR 16
#define CWD_NOT_FOUND_ERROR 17
#define NULL_GIVEN_ERROR 18
#define NOT_A_DIR_ERROR 19
#define SYSTEM_CALL_ERROR 20
#define FORK_ERROR 21

#define EXIT_CMD 0
#define ECHO_CMD 1
#define CD_CMD 2
#define SHOWVAR_CMD 3
#define EXPORT_CMD 4
#define UNSET_CMD 5
#define SHOWENV_CMD 6
#define PUSHD_CMD 7
#define POPD_CMD 8
#define DIRS_CMD 9
#define SOURCE_CMD 10

#define STACK_SIZE 100

#define MEMORY_ERROR_MSG "A problem occured while dynamically allocating memory\n"
#define BUFFER_ERROR_MSG "Did a token exceed its maximum buffer size?\n"
#define PARSE_ERROR_MSG "Error while parsing.\n"
#define VARIABLE_DECLARATION_MSG "Do you have a variable expansion character in quotes or referencing illegal characters?\n"
#define VARIABLE_EXPANSION_MSG "Variable does not exsist.\n"
#define VARIABLE_ASSIGNMENT_MSG "Invalid 'assign' operation.\n"
#define VARIABLE_NAME_MSG "An invalid variable name was provided.\n"
#define NODE_NOT_FOUND_MSG "Reference to non existent variable.\n"
#define NODE_ASSIGNMENT_MSG "Could not create new variable.\n"
#define STACK_FULL_MSG "Could not add to stack. Stack is full.\n"
#define STACK_EMPTY_MSG "Could not pop from stack. Stack has only one item.\n"
#define TOKENS_MEMORY_MSG "A problem occured while dynamically allocating memory for tokens.\n"
#define VARINDICES_MEMORY_MSG "A problem occured while dynamically allocating memory for variable pointers.\n"
#define INVALID_ARGS_MSG "Invalid arguments.\n"
#define ENV_VARIABLE_NOT_FOUND_MSG "Enviroment variable could not be found.\n"
#define ENV_VARIABLE_ASSIGNMENT_MSG "A problem occured while assigning to an enviroment variable.\n"
#define CWD_NOT_FOUND_MSG "The Current Working Directory could not be found.\n"
#define NULL_GIVEN_MSG "The program encountered a NULL instead of a value.\n"
#define NOT_A_DIR_MSG "Not a directory.\n"



typedef struct node_
{
    char* key;
    char* value;
    bool env;
    struct node_ *next;
    struct node_ *prev;
} node;
typedef struct tokenchar_pair_struct
{
    int token_index;
    int char_index;
} tokenchar_pair;


typedef struct token_section_
{
    int input;
    int output;
    bool cat;
    int redirect_count;
} token_section[BUFSIZE];
typedef struct redirect_ext_
{
    token_section section[BUFSIZE]; //r.chunk_array
    int pipe_count; //p.count
    int pipe_start; //p.start
    int pipe_end; // p.end
    int pipe_indices[BUFSIZE];  //p.array
    int redirect_end; //r.end
} redirect_ext;

typedef struct redirect_int_
{
    int redirect_indices[BUFSIZE]; //r.array
    int redirect_count; //r.count

    char input_filename[TOKEN_SIZE]; //r.input
    char output_filename[TOKEN_SIZE]; //r.output
    char output_cat_filename[TOKEN_SIZE]; //r.output_cat

    int redirect_start;//r.start
} redirect_int;


redirect_ext ex;
redirect_int in;

int errors_length;
char errors[ERRORS_LENGTH][100];
char prompt_default [20];
char  exit_keyword[20];
char metacharacters [20];
char quotes[20];
char internal_commands[INTERNAL_COMMANDS_LEN][TOKEN_SIZE];

int internal_commands_len;
int error;

FILE* fp;
node* head;
node* tail;
int vars_len;

char* stack[STACK_SIZE];     
int top;  //Will always point to the last element of stack. -1 if stack is empty.

int new_start;
int stdin_fd;
int stdout_fd;
char filename[TOKEN_SIZE];

extern char **environ;

void init();
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

//Miscellaneous
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
void nodes_print();

//Stack
int print_stack();
int peek(char** value);
int pop(char** value);
int push(char* value);
bool is_full();
int change_directory(char* value);
