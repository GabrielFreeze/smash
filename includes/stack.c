#include "headers.h"
#define STACK_SIZE 100
       
char* stack[STACK_SIZE];     
int top = -1;  //Will always point to the last element of stack. -1 if stack is empty.


bool is_empty()
{
    return top == -1;
}
bool is_full()
{
    return top+1 == STACK_SIZE;
}
int push(char* value)
{
    if (is_full())
        return STACK_FULL_ERROR;
    
    if(!(stack[++top] = (char*) malloc(strlen(value))))
        return MEMORY_ERROR;

    strcpy(stack[top],value);

    return 0;
}   
int pop(char** value)
{
    if (is_empty())
        return STACK_EMPTY_ERROR;

    strcpy(*value,stack[top]);
    free(stack[top--]);
    return 0;
}
int peek(char** value)
{
    if(is_empty())
        return STACK_EMPTY_ERROR;

    strcpy(*value, stack[top]);

    return 0;
}