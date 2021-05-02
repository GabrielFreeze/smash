#pragma once
#define TOKEN_SIZE 50

#define NONE -1
#define NORMAL 0
#define META 1
#define ESCAPE 2
#define VARIABLE 3
#define QUOTE 4

#define MEMORY_ERROR 5
#define BUFFER_OVERFLOW_ERROR 6
#define LENGTH_ERROR 7


char* prompt = {"init> "};
char* exit_keyword = {"exit"};
char* metacharacters = {" |;<>\t"};
char* quotes = {"\"\'"};
