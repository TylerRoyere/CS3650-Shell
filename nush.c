#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <bsd/string.h>

#include "tokenize.h"
#include "svec.h"
#include "redir.h"
#include "eval.h"
#include "ast.h"
#include "vars.h"

// returns pointer to the character before the '\'
// at the end of a line ignoring whitespace.
// if no \ exists at the end, return NULL
char*
has_line_continue(char* str)
{
    char* end = str + strlen(str)-1;
    while(end >= str) {
        if(*end == '\\') {
            return end-1;
        }
        if(!isspace(*end)) {
            break;
        }
        --end;
    }
    return 0;
}

// appends the src string to the dest string
// needs to be freed using free();
char* 
line_append(char* dest, char* src, int length)
{
    // dest is empty return copy of src
    if(!dest) {
        char* duplicate = strndup(src, length+1);
        duplicate[length] = '\0';
        return duplicate;
    }
    
    // dest has stuff too
    // allocate enough character for both lines
    int new_length = strlen(dest) + length + 1;
    dest = realloc(dest, new_length * sizeof(char));
    strlcat(dest, src, new_length);
    return dest;
}

void
execute(char* cmd, hashmap* hh)
{
    // tokenize the command
    svec* tokens = tokenize(cmd);

    // if there is no command, return
    if(tokens->size == 0) {
        return;
    }

    // create an abstract syntax tree
    int start = 0;
    ast_node* ast = ast_create(tokens, &start);

//     print_ast(ast, 0);
    // evaluate the ast
    eval(ast, hh);

    // free the ast
    node_free(ast);

    // free the vector of tokens used by the ast
    free_svec(tokens);

    // line is done
    return;
}

int
main(int argc, char* argv[])
{
    char line[256];
    hashmap* hh = make_hashmap();
    // check if we are running in interactive or script mode
    int prompt = 0;
    if (argc == 1) {
        fflush(stdout);
        prompt = 1;
    }
    else {
        // we are running in script mode
        // use file as script, re-route to stdin
        // Open script file as read only
        int fd = open(argv[1], O_RDONLY, 0444);
        if(fd < 0) {
            printf("Unable to open script File %s\n", argv[1]);
        }
        // close stdin, and dup script file to become new "stdin"
        close(0);
        if(0 != dup(fd)) {
            printf("Unable to redirect script to stdin\n");
        }

        // now we can execute the file just like any other seriers of command
    }
    char* line_end = 0;
    char* multi_line = 0;
    // we are going to execute until we reach the EOF
    while(1) {
        // print the prompt if necessary
        if(prompt) {
            printf("nush$ ");
        }
        if(!fgets(line, sizeof(line), stdin)) {
            // this means execution is done
            exit(0);
        }
        
        // if this line has a continuation in it, get the part
        // of the line you would like to save
        line_end = has_line_continue(line);
        int length = line_end==0 ? strlen(line) : line_end-(&line[0])+1;

        // append this line to the multi_line string
        multi_line = line_append(multi_line, line, length);

        // if the current input had a line continuation in it
        // get new input
        if(line_end) {
            continue;
        }

        // we can only get here if there have been no '\'
        // time to execute all the things
        execute(multi_line, hh);
    
        // free heap resources and reset state variables
        free(multi_line);
        multi_line = 0;
        line_end = 0;
    }

    return 0;
}
