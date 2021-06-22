#pragma once
#define TOKEN_SIZE 2000

#define BUFSIZE 256

typedef enum char_type{BLANK = -1,
                NORMAL,
                META,
                ESCAPE,
                VARIABLE,
                QUOTE,
                EQUAL,
                OUTPUT,
                OUTPUT_CAT,
                INPUT,
                PIPE} charno;

typedef enum error_types{ NONE,
                 MEMORY_ERROR,
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
typedef enum cmds{
          EXIT_CMD,
          ECHO_CMD,
          CD_CMD,
          SHOWVAR_CMD,
          EXPORT_CMD,
          UNSET_CMD,
          SHOWENV_CMD,
          PUSHD_CMD,
          POPD_CMD,
          DIRS_CMD,
          SOURCE_CMD} cmdno; //The different types of internal commands.

#define STACK_SIZE 100


typedef struct node_
{
    char* key;
    char* value;
    bool env;
    struct node_ *next;
    struct node_ *prev;
} node; //A single item in the shell variable doubly linked list.
typedef struct tokenchar_pair_struct
{
    int token_index;
    int char_index;
} tokenchar_pair; //Used for identifying variable expansion characters.
typedef struct token_section_
{
    int input;
    int output;
    bool cat;
    int redirect_count;
} token_section; // Used for identifying the redirects of a specific token chunk.
typedef struct redirect_ext_
{
    token_section section[BUFSIZE]; 
    int pipe_count; 
    int pipe_start; 
    int pipe_end; 
    int pipe_indices[BUFSIZE];
    int redirect_end;
    int execute_start;
} redirect_ext; // Used for executing external commands.
typedef struct redirect_int_
{
    charno redirect_indices[BUFSIZE]; 
    int redirect_count; 

    char input_filename[TOKEN_SIZE]; 
    char output_filename[TOKEN_SIZE];
    char output_cat_filename[TOKEN_SIZE];
    
    int redirect_start;
} redirect_int; // Used for executing internal commands.


volatile sig_atomic_t child_pids[BUFSIZE]; // Holds all child pids
volatile sig_atomic_t child_count; //The length of child_pids. Initally set to 0.

extern const char* const error_msg[100];
extern char* const prompt_default;
extern const char* const exit_keyword;
extern const char* const metacharacters;
extern const char* const quotes;
extern const char* const internal_commands[TOKEN_SIZE];

volatile err error;
int exit_value; // The value the program should exit on
bool exit_program; // Whether the program should exit or not

FILE* fp; //For the source command
node* head; //Head of the shell variable doubly linked list
node* tail; //Tail of the shell variable doubly linked list

char* stack[STACK_SIZE];// The underlying data structure for the directory stack
int top;  //Will always point to the last element of stack. -1 if stack is empty.

int stdin_fd; // Used to redirect input while executing an internal command
int stdout_fd;// Used to redirect output while executing an internal command

extern char **environ;

//Tokenisation
int tokens_init(const char* string, redirect_int* in, redirect_ext* ex);
charno char_type(const char* string, int j);
char** tokens_get(const char* input, int* length, tokenchar_pair** var_indices, int* var_index, redirect_int* in, redirect_ext* ex);
bool is_deref(const char* string, int upper);
bool is_meta(const char* string, int j);

// Error handling and variable resetting.
void handle_error();
void tokens_free(char** tokens, int* length);
void var_indices_free(tokenchar_pair* var_indices, int* var_indices_len);
void reset_in(redirect_int* in);
void reset_ex(redirect_ext* ex);
err reset_streams();
void free_vars();
void free_stack();

//Shell variables.
err init_vars(void);
bool vars_valid(const char* token, int j);
err assign_vars(char** tokens, int length, int i, int k);
err expand_vars(char** tokens, tokenchar_pair* var_indices, int var_indices_len, int m);

// Commands.
err execute_internal(char* args[TOKEN_SIZE], int arg_num, cmdno j);
err execute_external(char** tokens, redirect_ext* ex);

//Redirects for internal commands.
err handle_redirect(char* tokens[TOKEN_SIZE], charno redirect_state, int j, redirect_int* in);
err hook_streams(const redirect_int* in);

//Miscellaneous
char* get_input_from_file(FILE* fp);
bool contains_word(const char* input, const char* key);
int contains_char(const char* string, char a);
err str_to_int(int* value, const char* string);
void SIGINT_handler(int signum);

//Linked List
err node_insert(const char* const key, const char* const value, bool env);
err node_delete(node* current_node);
node* node_search(const char* key);
err node_edit(node* current_node, const char* value);
err node_export(node* current_node);
void nodes_print();

//Stack
err print_stack();
err peek(char** value); 
err pop(char** value);
err push(const char* value);
bool is_full();
err change_topmost(const char* value);
err change_directory(const char* cwd);
