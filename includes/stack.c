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
    
    if(!(stack[++top] = (char*) malloc(strlen(value)+1)))
        return MEMORY_ERROR;

    strcpy(stack[top],value);

    return 0;
}   
int pop(char** value)
{
    peek(value);
    free(stack[top--]);

    return 0;
}
int peek(char** value)
{
    if (is_empty())
        return STACK_EMPTY_ERROR;

    char* value2;
    if(!(value2 = (char*) malloc(strlen(stack[top])+1)))
        return MEMORY_ERROR;

    strcpy(value2,stack[top]);
    *value = value2;
    return 0;
}
int print_stack()
{
    if (is_empty())
        return STACK_EMPTY_ERROR;
    
    for (int i = 0; i < top+1; i++)
        printf("%s  ",stack[i]);
    printf("\n");
    return 0;
}