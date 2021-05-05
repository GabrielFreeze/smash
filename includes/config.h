#pragma once
#define TOKEN_SIZE 50
#define KEY_SIZE 50
#define VALUE_SIZE 50
#define VARIABLE_LENGTH 100

#define MEMORY_ERROR_MSG "A problem occured while dynamically allocating memory\n"
#define BUFFER_ERROR_MSG "Did a token exceed its maximum buffer size of %d characters?\n"
#define PARSE_ERROR_MSG "Error while parsing.\n"
#define VARIABLE_ASSIGNMENT_MSG "Do you have a variable expansion character in quotes or referencing illegal characters?\n"



#define NONE -1
#define NORMAL 0
#define META 1
#define ESCAPE 2
#define VARIABLE 3
#define QUOTE 4

#define MEMORY_ERROR 5
#define BUFFER_OVERFLOW_ERROR 6
#define PARSE_ERROR 7
#define VARIABLE_ASSIGNMENT_ERROR 8


char* prompt = {"init> "};
char* exit_keyword = {"exit"};
char* metacharacters = {" |;<>\t"};
char* quotes = {"\"\'"};

typedef struct variable
{
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
    bool is_valid;
} variables[VARIABLE_LENGTH];