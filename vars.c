#include <stdio.h>
#include <stdlib.h>
#include <bsd/string.h>
#include <string.h>

#include "vars.h"

// adds or updates the key's value
void
add_var(hashmap* map, ast_node* node)
{   
    int length = 0;
    char key[strlen(node->cmd)+1];
    char value[strlen(node->cmd)+1];
    // if this is a valid command
    if(node->cmd) {
        // until the null terminator
        while(node->cmd[length]) {
            // copy the portion behind the = as key
            if(node->cmd[length] == '=') {
                strlcpy(key, node->cmd, length+1);

                // copy the portion after as value
                int remaining = strlen(node->cmd) - length;
                strlcpy(value, node->cmd+length+1, remaining);
                break;
            }
            ++length;
        }
    }
    // takes the var and adds it to the hashmap
    hashmap_put(map, key, value);
}

// determines if the node is a var assignmnet
int 
is_var_assignment(ast_node* node)
{
    int has_equals = 0;
    // this node must be a command, operators wont count
    if(node->cmd) {
        // go through the string looking for an equal sign
        for(int ii = 0; node->cmd[ii]; ++ii) {
            // if any character is '=' than has_equals
            // will be non-zero
            has_equals |= (node->cmd[ii] == '=');
        }
    }

    return has_equals;
}

// determines if the node has vars in it that 
// could be replaced
int 
has_vars(ast_node* node)
{
    // if a command or argument starts with$
    // this signals a shell var usage, return 1
    if(node->cmd) {
        if(node->cmd[0] == '$') {
            return 1;
        }
        for(int ii = 0; node->args[ii]; ++ii) {
            if(node->args[ii][0] == '$') {
                return 1;
            }
        }
    } 

    // otherwise return false
    return 0;
}


// replaces instances of vars with their associated value
// in the hashmap
void
var_replace(hashmap* vars, ast_node* node)
{
    char *temp = 0;
    if(node->cmd) {
        // if the cmd starts with $ it is a var
        if(node->cmd[0] == '$') {
            // move the pointer to the string after the $
            temp = hashmap_get(vars, node->cmd+1);
            free(node->cmd);
            node->cmd = strdup(temp);
        } // same thing for args
        for(int ii = 0; node->args[ii]; ++ii) {
            if(node->args[ii][0] == '$') {
                // same thing as before
                temp = hashmap_get(vars, node->args[ii] + 1);
                free(node->args[ii]);
                node->args[ii] = strdup(temp);
            }
        }
    }
}
