#pragma once
#define TOKEN_SIZE 2000
#define KEY_SIZE 2000
#define VALUE_SIZE 2000
#define BUFSIZE 256

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


int errors_length = 20;
char* errors[100] = {"_",
                    MEMORY_ERROR_MSG,
                    BUFFER_ERROR_MSG,
                    PARSE_ERROR_MSG,
                    VARIABLE_DECLARATION_MSG,
                    VARIABLE_EXPANSION_MSG,
                    VARIABLE_ASSIGNMENT_MSG,
                    VARIABLE_NAME_MSG,
                    NODE_NOT_FOUND_MSG,
                    NODE_ASSIGNMENT_MSG,
                    STACK_FULL_MSG,
                    STACK_EMPTY_MSG,
                    TOKENS_MEMORY_MSG,
                    VARINDICES_MEMORY_MSG,
                    INVALID_ARGS_MSG,
                    ENV_VARIABLE_NOT_FOUND_MSG,
                    ENV_VARIABLE_ASSIGNMENT_MSG,
                    CWD_NOT_FOUND_MSG,
                    NULL_GIVEN_MSG,
                    NOT_A_DIR_MSG
                    };


typedef struct node_
{
    char* key;
    char* value;
    bool env;
    struct node_ *next;
    struct node_ *prev;
} node;
typedef unsigned char byte;
typedef unsigned short two_bytes;
typedef struct tokenchar_pair_struct
{
    int token_index;
    int char_index;
} tokenchar_pair;

char* prompt_default = {"init> "};
char* exit_keyword = {"exit"};
char* metacharacters = {" |;<>\t"};
char* quotes = {"\"\'"};
char* internal_commands[TOKEN_SIZE] = {"exit","echo","cd","showvar","export","unset","showenv","pushd","popd","dirs","source"};
char** envp;

int internal_commands_len = 11;
int error = 0;

FILE* fp;
node* head;
node* tail;
bool read_from_file = false;

typedef struct red_
{
    int input;
    int output;
    bool cat;
    int start;
} red[BUFSIZE];

typedef struct redirect_
{
    char input[TOKEN_SIZE];
    char output[TOKEN_SIZE];
    char output_cat[TOKEN_SIZE];
    int count;
    int array[BUFSIZE];
    red chunk_array[BUFSIZ];
    int chunk_array_counter;
    int start;
    int end;
} redirect;



red k_array;



redirect r;
typedef struct pipe_struct
{   
    int count;
    int start;
    int end;
    int array[BUFSIZE];
} pipey;

pipey p;
int new_start;
int stdin_fd = -1;
int stdout_fd = -1;
char filename[TOKEN_SIZE];

extern char **environ;