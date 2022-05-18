#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <bsd/string.h>
#include <string.h>

#include "ast.h"

// here are the currently defined operators
static char* operators[] = {"&&", "||", "&", "|", "<", ">", ";", "(", ")"};


// if the string is an operator, returns pointer to that operator string
static
char*
is_op(char* string)
{
    // go through all of the commands checking for match
    for(int ii = 0; ii < 9; ++ii) {
        if(!strcmp(operators[ii], string)) {
            return operators[ii];
        }
    }
    // no match here!
    return 0;
}

// prints the given tree
void
print_ast(ast_node* root, int depth)
{
    static const int scale = 4;
    if(root == 0) {
        return;
    }
    char spaces[(depth*scale) + 1];
    memset(spaces, ' ', depth * scale);
    spaces[(depth*scale)] = '\0';
    print_ast(root->right, depth+1);
    printf("%s", spaces);
    if(root->cmd) {
        printf("%s {",root->cmd);
        for(int ii = 0; root->args && root->args[ii]; ++ii) {
            printf("%s, ", root->args[ii]);
        }
        printf("0}\n");
    } 
    else {
        printf("%s\n", root->op);
    }

    print_ast(root->left, depth+1);
}



// creates a command/file node with owned strings
// that need to be freed using node_free()
ast_node*
ast_cmd_create(char* cmd_str, char** args, int num_args)
{
    // allocate space for new command
    ast_node* cmd_node = malloc(sizeof(ast_node));
    cmd_node->op = 0;
    cmd_node->group = 0;
    // set children nodes
    cmd_node->left = 0;
    cmd_node->right = 0;

    if(cmd_str) {
        cmd_node->cmd = strdup(cmd_str);
    } else {
        puts("No command string provided");
        abort();
    }

    // allocate space for string pointers
    cmd_node->args = malloc( (num_args+1) * sizeof(char*));
    memset(cmd_node->args, 0, (num_args+1) * sizeof(char*));

    // do this only if we have args to add
    if(args) {
        for( int ii = 0; ii < num_args && args[ii]; ++ii) {
            // copy strings
            cmd_node->args[ii] = strdup(args[ii]);
        }
    }
    
    // ensure that argument list is null terminated
    cmd_node->args[num_args] = 0;
    return cmd_node;
}

// creates a new operation node that owns it's own string
// needs to be freed using node_free()
ast_node*
ast_op_create(char* op, ast_node* left, ast_node* right)
{
    ast_node* op_node = malloc(sizeof(ast_node));
    op_node->op = strdup(op);
    op_node->cmd = 0;
    op_node->args = 0;
    op_node->group = 0;
    op_node->left = left;
    op_node->right = right;
    return op_node;
}

// creates a new grouping node from the command given
ast_node*
ast_group_create(ast_node* tree)
{
    // groups will act like commands in ast creation
    ast_node* group = malloc(sizeof(ast_node));
    group->group = strdup("(");
    group->cmd = strdup("(");
    group->args = 0;
    group->op = 0;
    group->left = tree;
    group->right = 0;
    return group;
}

void
node_free(ast_node* root)
{
    if(!root) {
        return;
    }

    // free every part of the node
    node_free(root->left);
    node_free(root->right);
    free(root->op);
    free(root->cmd);
    free(root->group);

    // if there are arguments left
    if(root->args) {
        // don't need to know the length since arguments
        // array is terminated by null pointer
        for(int ii = 0; root->args[ii]; ++ii) {
            free(root->args[ii]);
        }
    }

    // now the argument array can be freed
    free(root->args);
    
    // new this root can be free'd
    free(root);
    return;
}

// creates an abstract syntax tree from the given svec
ast_node*
ast_create(svec* tokens, int* tok_index)
{   
    // create an ast using the tokens starting at tok_index
    // declaring the working tree and the node to be appended
    ast_node* tree_so_far = 0;
    ast_node* node = 0;

    // collect these nodes from the tokens
    while( *tok_index < tokens->size ) {
        // get the current token
        char* tok = svec_get(tokens, *tok_index);

        // if we are at the end of a grouping return the generated tree
        if(*tok == ')') {
            return tree_so_far;
        }

        // if this is a grouping, get the tree for the grouping
        if(*tok == '(') {
            ++(*tok_index);
            node = ast_group_create(ast_create(tokens, tok_index));
            ++(*tok_index);
        }
        // just handle like a normal command
        else {
            node = get_next_node(tokens, tok_index);
        }

        // add the new node (oblivious to groups)
        tree_so_far = add_node(tree_so_far, node);
        
    }

    // nothing left
    return tree_so_far;
    
}

ast_node*
get_next_node(svec* tokens, int* index)
{
    // Check index within bounds
    if(*index >= tokens->size) {
        return 0;
    }

    // if this starts with an op, return that
    char* tok = svec_get(tokens, *index);
    char* op = is_op(tok);
    if(op != 0) {
        if(!strcmp(op, "(") || !strcmp(op, ")")) {
            // the case where the paranthesis are
            // empty
            char* temp[] = {"", 0};
            return ast_cmd_create("", temp, 1);
        }  
        // need to move the index forward
        ++*index;
        // need to make sure that the function that 
        // constructs the tree handles the children
        return ast_op_create(op, 0, 0);
    }

    char* cmd = 0;
    char* args[tokens->size + 1]; // safe bet
    memset(args, 0, sizeof(char*) * (tokens->size+1));

    // index into the args array
    int num_args = 0;

    // loop through tokens, either getting an op
    // or a cmd/file updating index while doing it
    for(; *index < tokens->size; ++(*index)) {
        
        tok = svec_get(tokens, *index);
        op = is_op(tok);
        if(op != 0) {
            // this is the end of the command/file
            break;
        }
        // this must be either a command or file
        if(cmd == 0) {
            // this is the first part of the command
            cmd = tok;
        }
        // these are part of the args
        args[num_args++] = tok;
    }

    // make the resulting command
    return ast_cmd_create(cmd, args, num_args);

}

// determines if operator ends a command sequence
static
int
is_special_op(ast_node* op)
{
    return !strcmp(op->op, ";") || !strcmp(op->op, "&");
}

// attempt to add a given node to the tree with the following rules
// if ast's root is an operator:
//  - adding operators (not & or ;) will make the ast's root the left
//    child of the operator, returning the resulting tree
//  - adding operators ; or & will add them to the right child to 
//    enforce they are executed at the end of the sequence
//  - adding command will make the command the right child of the 
//    ast's root operator
// if ast's root is a command:
//  - adding an operator will make the ast's root command the
//    left child of the command 
//  - adding a command is INVALID
// Adding a command to a tree where the roots right child is
// a operator signals that a new sequence is being started
// since only & and ; are ever added as the right child operand
// of a node. This requires that a new sequence be started by
// making a new operator node (;) that seperates the two 
// sequences
// NULL pointer returned if node cannot be added
// a ; or & will end what I call a sequence 
// (an expression of commands and operators)
// allowing the background command to only fork() when it is supposed to
ast_node*
add_node(ast_node* ast, ast_node* node)
{
    // if there is nothing in the ast, return node
    if(ast == 0) {
        return node;
    }
    
    // if this is an operator, make the root node
    // the left operand of the operator
    if(node->op) {
        // adding op to cmd makes cmd left child of op
        if(ast->cmd){
            node->left = ast;
            return node; 
        }
        // adding to op
        if(ast->op){
            // if special op (; or &) make right child
            // result of adding op to the right child
            if(is_special_op(node)) {
                ast->right = add_node(ast->right, node);
                return ast;
            }
            // otherwise make the op's left child the root op
            node->left = ast;
            return node;
        }
    }

    if(node->cmd) {
        if(ast->cmd) {
            // error, this is not allowed
            node_free(ast);
            node_free(node);
            return 0;
        }
        // if there is an op to the right of the root
        // node this is a & or ;, start a new sequence
        if(ast->right && ast->right->op) {
            return ast_op_create(";", ast, node);   
        }
        // adding cmd to op makes cmd right child of op
        if(ast->op) {
            ast->right = node;
            return ast;
        }
    }

    // somehow you got here, bad command
    node_free(ast);
    node_free(node);

    // this is an error
    return 0;
}
