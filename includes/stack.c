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

    if (error = change_directory(value))
        return error;

    return 0;
}   
int pop(char** value)
{
    if (!top) //Checks whether the stack has more than 1 value. The stack can never be empty.
        return STACK_EMPTY_ERROR;
    
    if (error = peek(value))
        return error;

    free(stack[top--]);

    //Updates the current working directory to the top most value, after the pop.
    if (error = change_directory(stack[top]))
        return error;

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
    for (int i = top; i >= 0; i--)
        printf("%s  ",stack[i]);
    printf("\n");

    return 0;
}
int change_topmost(char* value)
{
    if (!(stack[top] = (char*) realloc(stack[top], strlen(value)+1)))
        return MEMORY_ERROR;

    strcpy(stack[top],value);
    return 0;
}
int change_directory(char* cwd)
{
    if (!cwd)
        return CWD_NOT_FOUND_ERROR;

    if (chdir(cwd)) //Changing the directory
        return CWD_NOT_FOUND_ERROR; //Change this to perror maybe

    char* new_cwd;
    if (!(new_cwd = getcwd(NULL,0))) //Getting the new directory
        return CWD_NOT_FOUND_ERROR; //perror?

    if (setenv("PWD",new_cwd,1)) //Setting PWD(env) to the new directory
        return ENV_VARIABLE_NOT_FOUND_ERROR;

    node* current_node;

    if (!(current_node = node_search("CWD"))) 
        return CWD_NOT_FOUND_ERROR;

    if (error = node_edit(current_node, new_cwd)) //Set CWD(env and shell) to the new directory
        return error;

    if (error = change_topmost(new_cwd)) //Updating dierctory stack by changing the top most element.
        return error;
    
    return 0;

}
