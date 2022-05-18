#ifndef VARS_H
#define VARS_H

#include "hashmap.h"
#include "ast.h"

// adds a var as key with value to hashmap
// If the key is already in the hashmap
// update the value
void add_var(hashmap*, ast_node*);

// determines if the node is an assignment
int is_var_assignment(ast_node*);

// determines if the node has vars in it
int has_vars(ast_node*);

// replaces the instances of vars key with value
void var_replace(hashmap*, ast_node*);

#endif
