#pragma once
#define TOKEN_SIZE 5000
#define KEY_SIZE 5000
#define VALUE_SIZE 32767
#define VARIABLE_LENGTH 100
#define INIT_VARS_LEN 9

#define MEMORY_ERROR_MSG "A problem occured while dynamically allocating memory\n"
#define BUFFER_ERROR_MSG "Did a token exceed its maximum buffer size of %d characters?\n"
#define PARSE_ERROR_MSG "Error while parsing.\n"
#define VARIABLE_DECLARATION_MSG "Do you have a variable expansion character in quotes or referencing illegal characters?\n"
#define VARIABLE_EXPANSION_MSG "Variable does not exsist.\n"
#define VARIABLE_ASSIGNMENT_MSG "Invalid 'assign' operation.\n"
#define NODE_NOT_FOUND_MSG "Reference to non existent variable.\n"
#define NODE_ASSIGNMENT_MSG "Could not create new variable.\n"


#define NONE -1
#define NORMAL 0
#define META 1
#define ESCAPE 2
#define VARIABLE 3
#define QUOTE 4
#define EQUAL 5

#define MEMORY_ERROR 51
#define BUFFER_OVERFLOW_ERROR 52
#define PARSE_ERROR 53
#define VARIABLE_DECLARATION_ERROR 54
#define VARIABLE_EXPANSION_ERROR 55
#define VARIABLE_ASSIGNMENT_ERROR 56
#define VARINDICES_MEMORY_ERROR 60
#define NODE_NOT_FOUND_ERROR 57
#define NODE_ASSIGNMENT_ERROR 58
#define STACK_FULL_ERROR 64
#define STACK_EMPTY_ERROR 65
#define TOKENS_MEMORY_ERROR 59
#define INVALID_FUNCTION_USE_ERROR 61 //Intended if programmers (I) call a function with incorrect values
#define INVALID_ARGS_ERROR 62
#define ENV_VARIABLE_NOT_FOUND_ERROR 63
#define ENV_VARIABLE_ASSIGNMENT_ERROR 66


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


typedef struct node_ {
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

node* head;
node* tail;
extern char **environ;