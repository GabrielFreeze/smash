#pragma once
#define TOKEN_SIZE 100
#define KEY_SIZE 50
#define VALUE_SIZE 50
#define VARIABLE_LENGTH 100
#define INIT_VARS_LEN 9

#define MEMORY_ERROR_MSG "A problem occured while dynamically allocating memory\n"
#define BUFFER_ERROR_MSG "Did a token exceed its maximum buffer size of %d characters?\n"
#define PARSE_ERROR_MSG "Error while parsing.\n"
#define VARIABLE_DECLARATION_MSG "Do you have a variable expansion character in quotes or referencing illegal characters?\n"
#define VARIABLE_EXPANSION_MSG "Variable does not exsist.\n"


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
#define NODE_NOT_FOUND_ERROR 57
#define NODE_ASSIGNMENT_ERROR 58

typedef struct node_ {
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
    bool env;
    struct node_ *next;
    struct node_ *prev;
} node;


typedef unsigned char byte;
typedef unsigned short two_bytes;


typedef struct tokenchar_pair_struct
{
    byte token_index;
    byte char_index;
} tokenchar_pair;

char* prompt = {"init> "};
char* exit_keyword = {"exit"};
char* metacharacters = {" |;<>\t"};
char* quotes = {"\"\'"};
two_bytes vars_len = 0;


// char* env_key_startup[KEY_SIZE] = {"PATH", "PROMPT", "CWD", "USER", "HOME", "SHELL", "TERMINAL", "EXITCODE"};
// char* env_val_startup[VALUE_SIZE] = {"","","","","","","","","",};
typedef struct var
{
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
    bool is_valid;
    bool env;
} vars[VARIABLE_LENGTH];



vars variables;
node* head;
