#ifndef AST_H
#define AST_H

#include "svec.h"

// here is the structure representing an ast node
typedef struct ast_node{
    char* op;
    char* cmd;
    char** args;
    char* group;
    struct ast_node* left;
    struct ast_node* right;
} ast_node;

// creates an ast from an svec of tokens starting at index
ast_node* ast_create(svec* tokens, int*);

// gets the node starting at a specific index
// returns the created node or NULL if there
// aren't any nodes left, changes int to the 
// start of the next token
ast_node* get_next_node(svec* tokens, /* inout */ int*);

// attempts to add a node to a given ast
ast_node* add_node(ast_node*, ast_node*);

// create nodes
ast_node* ast_cmd_create(char*, char**, int);
ast_node* ast_op_create(char*, ast_node*, ast_node*);
ast_node* ast_group_create(ast_node*);

// frees nodes recursively downwards
void node_free(ast_node*);

void print_ast(ast_node*, int);
#endif
