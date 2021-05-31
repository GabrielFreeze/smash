#include "headers.h"
int vars_len = 0;

int node_insert(char* key, char* value, bool env)
{
    node* new_node;
    node* current_node;

    //If there already exists a node with that key, remove it.
    if (current_node = node_search(key))
        node_delete(current_node);

    if  (!((new_node = (node*) malloc(sizeof(node))) &&
        ((new_node->key = (char*) malloc(strlen(key)+1)) && 
        (new_node->value = (char*) malloc(strlen(value)+1)))))
        return MEMORY_ERROR;

    strcpy(new_node->key, key);
    strcpy(new_node->value, value);
    new_node->env = env;

    if (!vars_len)
        tail = new_node;
    else
        head->prev = new_node;


    new_node->next = head;
    head = new_node;

    if (env && setenv(key,value,true))
        return ENV_VARIABLE_ASSIGNMENT_ERROR;
    
    vars_len++;
    
    return 0;

}
node* node_search(char* key)
{
    node* current_node;

    if (!(current_node = head))
        return NULL;

    while (current_node && strcmp(current_node->key,key))
        current_node = current_node->next;

    return current_node;
}
int node_delete(node* current_node)
{
    //If previous node is Null, then the node to delete is the first one
    if (!current_node)
        return NODE_NOT_FOUND_ERROR;

    if (current_node == head)
      head = head->next;
   else
      current_node->prev->next = current_node->next; //The previous node will skip the current_node and point to the one after it.

   if (current_node == tail)
      tail = current_node->prev;
   else
      current_node->next->prev = current_node->prev; //The next node will skip the current_node and point to the one before it.

    free(current_node);
    vars_len--;

    return 0;

}
int node_edit(node* current_node, char* value)
{
    if (!current_node)
        return NODE_NOT_FOUND_ERROR;
    
    if (!value)
        return NULL_GIVEN_ERROR;

    current_node->value = (char*) realloc(current_node->value, strlen(value)+1);
    strcpy(current_node->value,value);

    //Update the enviroment variable representing this shell variable.
    if (current_node->env && setenv(current_node->key,value,1))
        return ENV_VARIABLE_NOT_FOUND_ERROR;

    return 0;
}
void nodes_print(){
    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
        printf("%s=%s\n",current_node->key, current_node->value);  
}
int node_export(node* current_node)
{
    if (!current_node)
        return NODE_NOT_FOUND_ERROR;
    
    current_node->env = true;

    if (setenv(current_node->key,current_node->value,1))
        return ENV_VARIABLE_ASSIGNMENT_ERROR;

    return 0;
}


