#ifndef EVAL_H
#define EVAL_H

#include "ast.h"
#include "hashmap.h"

// defines functions for evaluation different operations
// these return the resuting exit code of their operation

// these require hashmap to handle shell variables

// opens input file and places it in place of stdin for cmd
int eval_redir_in(ast_node*, ast_node*, hashmap*);

// opens output file and places it in place of stdout for cmd
int eval_redir_out(ast_node*, ast_node*, hashmap*);

// pipes two commands together
int eval_pipe(ast_node*, ast_node*, hashmap*);

// runs the command in the background
int eval_background(ast_node*, ast_node*, hashmap*);

// runs both commands, one after the other
int eval_semicolon(ast_node* , ast_node*, hashmap*);

// evaluates the tree starting at the given node
int eval(ast_node*, hashmap*);

#endif
