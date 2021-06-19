#pragma once
#define TOKEN_SIZE 2000
#define KEY_SIZE 2000
#define VALUE_SIZE 2000
#define BUFSIZE 256

enum char_types{NONE = -1,
                NORMAL,
                META,
                ESCAPE,
                VARIABLE,
                QUOTE,
                EQUAL,
                OUTPUT,
                OUTPUT_CAT,
                INPUT,
                PIPE};

typedef enum error_types{MEMORY_ERROR = 1,
                 BUFFER_OVERFLOW_ERROR,
                 PARSE_ERROR,
                 VARIABLE_DECLARATION_ERROR,
                 VARIABLE_EXPANSION_ERROR,
                 VARIABLE_ASSIGNMENT_ERROR,
                 VARIABLE_NAME_ERROR,
                 NODE_NOT_FOUND_ERROR,
                 NODE_ASSIGNMENT_ERROR,
                 STACK_FULL_ERROR,
                 STACK_EMPTY_ERROR,
                 TOKENS_MEMORY_ERROR,
                 VARINDICES_MEMORY_ERROR,
                 INVALID_ARGS_ERROR,
                 ENV_VARIABLE_NOT_FOUND_ERROR,
                 ENV_VARIABLE_ASSIGNMENT_ERROR,
                 CWD_NOT_FOUND_ERROR,
                 NULL_GIVEN_ERROR,
                 NOT_A_DIR_ERROR,
                 SYSTEM_CALL_ERROR} err;


#define ERRORS_LENGTH 19
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

#define INTERNAL_COMMANDS_LEN 11
enum cmds{EXIT_CMD,
          ECHO_CMD,
          CD_CMD,
          SHOWVAR_CMD,
          EXPORT_CMD,
          UNSET_CMD,
          SHOWENV_CMD,
          PUSHD_CMD,
          POPD_CMD,
          DIRS_CMD,
          SOURCE_CMD};

#define STACK_SIZE 100


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
} token_section;

typedef struct redirect_ext_
{
    token_section section[BUFSIZE]; 
    int pipe_count; 
    int pipe_start; 
    int pipe_end; 
    int pipe_indices[BUFSIZE];
    int redirect_end;
    int execute_start;
} redirect_ext;

typedef struct redirect_int_
{
    int redirect_indices[BUFSIZE]; 
    int redirect_count; 

    char input_filename[TOKEN_SIZE]; 
    char output_filename[TOKEN_SIZE];
    char output_cat_filename[TOKEN_SIZE];
    
    int redirect_start;
} redirect_int;


redirect_ext ex;
redirect_int in;

int child_pids[BUFSIZE];
int child_count;
char errors[ERRORS_LENGTH][100];
char prompt_default [20];
char metacharacters [20];
char quotes[20];
char internal_commands[INTERNAL_COMMANDS_LEN][TOKEN_SIZE];

err error;
int exit_value;
bool exit_program;

FILE* fp;
node* head;
node* tail;
int vars_len;

char* stack[STACK_SIZE];     
int top;  //Will always point to the last element of stack. -1 if stack is empty.

int stdin_fd;
int stdout_fd;

extern char **environ;

void init();
//Tokenisation
int tokens_len(char* string);
int char_type(char* string, int j);
char** tokens_get(char* input, int* length, tokenchar_pair** var_indices, int* var_index);
bool is_deref(char* string, int upper);
bool is_meta(char* string, int j);

// Error handling and variable resetting.
void handle_error();
void tokens_free(char** tokens, int* length);
void var_indices_free(tokenchar_pair* var_indices, int* var_indices_len);
void reset_in();
void reset_ex();
void reset_streams();
void free_vars();
void free_stack();

//Shell variables.
bool is_var(char* token);
int init_vars(void);
bool vars_valid(char* token, int j);
int expand_vars(char** tokens, tokenchar_pair* var_indices, int var_indices_len, int m);
int assign_vars(char** tokens, int length, int i, int k);

// Commands.
int execute_internal(char* args[TOKEN_SIZE], int arg_num, int j);
int execute_external(char** tokens);

//Redirects for internal commands.
int handle_redirect(char* tokens[TOKEN_SIZE], int redirect_state, int j);
int hook_streams();

//Miscellaneous
char* get_input_from_file(FILE* fp);
int contains_word(char* input, char* key);
int contains_char(char* string, char a);
int str_to_int(int* value, char* string);
void SIGINT_handler(int signum);

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
