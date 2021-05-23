#include "headers.h"
int vars_len = 0;

int node_insert(char* key, char* value, bool env)
{
    node* new_node;
    node* current_node;

    if (key == NULL || value == NULL)
        return NODE_ASSIGNMENT_ERROR;

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

    if (current_node->prev)
        current_node->prev->next = current_node->next;
    else
        head = current_node->next;

    free(current_node);
    vars_len--;

    return 0;

}
int node_edit(char* key, char* value)
{
    node* current_node;

    if (!(current_node = node_search(key)))
        return NODE_NOT_FOUND_ERROR;
    
    current_node = (node*) realloc(current_node, strlen(value));
    strcpy(current_node->value,value);

    return 0;
}
void nodes_print(){
    for (node* current_node = head; current_node != NULL; current_node = current_node->next)
        printf("%s=%s\n",current_node->key, current_node->value);  
}


