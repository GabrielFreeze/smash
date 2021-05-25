#include "headers.h"
#define STACK_SIZE 100
       
char* stack[STACK_SIZE];     
int top = -1;  //Will always point to the last element of stack. -1 if stack is empty.



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
    if (!top) //Checks whether the stack has more than 1 value. The stack can never be empty.
        return STACK_EMPTY_ERROR;
    
    if (error = peek(value))
        return error;

    free(stack[top--]);

    return 0;
}
int peek(char** value)
{
    if (!top)
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
    for (int i = 0; i < top+1; i++)
        printf("%s  ",stack[i]);
    printf("\n");

    return 0;
}
int change_topmost(char* value)
{
    // if ((stack[top] = (char*) realloc(stack[top],strlen(value)+1)));
    //     return MEMORY_ERROR;
    if (!(stack[top] = (char*) realloc(stack[top], strlen(value)+1)))
        return MEMORY_ERROR;

    strcpy(stack[top],value);
    printf("%s\n",stack[top]);
    return 0;
}
int change_directory(char* cwd)
{
    if (!cwd)
        return CWD_NOT_FOUND;

    if (chdir(cwd))
        return CWD_NOT_FOUND; //Change this to perror maybe

    char* new_cwd;
    if (!(new_cwd = getcwd(NULL,0)))
        return CWD_NOT_FOUND; //perror?

    if (setenv("PWD",new_cwd,1))
        return ENV_VARIABLE_NOT_FOUND_ERROR;

    node* current_node;

    if (!(current_node = node_search("CWD")))
        return CWD_NOT_FOUND;

    //Will mirror CWD(shell) and CWD(env) to the newly updated value in PWD(env)
    if (error = node_edit(current_node, new_cwd))
        return error;

    //Update directory stack by changing top most element.
    if (error = change_topmost(new_cwd))
        return error;
    
    return 0;

}
