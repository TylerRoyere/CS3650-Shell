#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include "eval.h"
#include "ast.h"
#include "redir.h"
#include "vars.h"

int eval(ast_node* ast, hashmap* hh)
{
    int status = 0;
    int chpid;
    // is this a command
    if(ast->cmd) {
        // handle the group case first
        if(ast->group){
            // groups have a tree on the left, eval that
            return eval(ast->left, hh);
        }
        // if this command has a variable assignment use that
        if(is_var_assignment(ast)){
            add_var(hh, ast);
            // don't execute anything
            return 0;
        }
        // if this command has variables, replace them
        if(has_vars(ast)){
            var_replace(hh, ast);
        }
        // handle builtin commands
        if(strcmp(ast->cmd, "cd") == 0) {
            // change director
            chdir(ast->args[1]);
            return 0;
        }
        else if (strcmp(ast->cmd, "exit") == 0) {
            // exit the shell
            exit(0);
            return 0;
        }

        // otherwise, just execute like normal
        int chpid = 0;          
        if((chpid = fork())) {
            waitpid(chpid, &status, 0);
            return status;
        }
        else {
            execvp(ast->cmd, ast->args);
            // this exec failed
            exit(1);
        }
            
        
    } // determine what to do based on operator
    else if(ast->op) {
        char* op = ast->op;
        if(strcmp(op, "<") == 0 ) {
            return eval_redir_in(ast->left, ast->right, hh);
        }
        else if(strcmp(op, ">") == 0 ) {
            return eval_redir_out(ast->left, ast->right, hh);
        }
        else if(strcmp(op, "|") == 0) {
            return eval_pipe(ast->left, ast->right, hh);
        }
        else if(strcmp(op, "&") == 0) {
            return eval_background(ast->left, ast->right, hh);
        }
        else if(strcmp(op, ";") == 0) {
            return eval_semicolon(ast->left, ast->right, hh);
        }
        else if(strcmp(op, "&&") == 0) {
            return eval(ast->left, hh) || eval(ast->right, hh);
        }
        else if(strcmp(op, "||") == 0) {
            return eval(ast->left, hh) && eval(ast->right, hh);
        }
        else {
            printf("shouldn't be here \n");
        }

    }
    return 1;
}


// returns the exit code of the command
int
eval_redir_in(ast_node* cmd, ast_node* file, hashmap* hh)
{
    // runs cmd with file redirected to stdin
    // check nodes
    if(!(cmd && file && (file->cmd || file->op))) {
        printf("cannot complete redir in\n");
        abort();
    }

    // fork to isolate file descriptor changes
    int chpid;
    if((chpid = fork())) {
        // parent process
        int status = 0;
        waitpid(chpid, &status, 0);
        return status;
    }
    else {
        // child process
        // redirect file to stdin and exec the command
        // fix to allow for redirection to be terminated by ;
        char* filename = file->op ? file->left->cmd : file->cmd;
        if(redir_infile(filename)) {
            printf("error trying to redirect infile%s\n", filename);
            exit(1);
        }
        
        // exec the cmd
        exit(eval(cmd,hh));

    }
}

int
eval_redir_out(ast_node* cmd, ast_node* file, hashmap* hh)
{
    // runs the cmd with file redirected to stdout
    // ensure that we have something to open, file should
    // always be the "cmd" on the right
    if(!(cmd && file && (file->cmd || file->op))) {
        printf("cannot complete redir out\n");
        abort();
    }
    // forking to isolate file descriptor changes
    int chpid;
    if((chpid = fork())) {
        // wait for the process to return to get exit status
        int status = 0;
        waitpid(chpid, &status, 0);
        return status;
    }
    else {
        // redirect stdout to file with filename from cmd
        // fix in the case file redirection was terminated
        // with ;
        char* filename = file->op ? file->left->cmd : file->cmd;
        if(redir_outfile(filename)) {
            printf("error reying to redirect outfile %s\n", filename);
            exit(1);
        }
        exit(eval(cmd, hh));
    }
} 

int eval_pipe(ast_node* left, ast_node* right, hashmap* hh)
{
    // need to fork so as to not pollute the fd space
    int chpid;
    int status;
    if((chpid = fork())) {
        waitpid(chpid, &status, 0);
        return status;
    }
    else {
        // now we can create the pipe
        int pipe_fds[2];
        if(pipe(pipe_fds) < 0) {
            puts("unable to create pipe\n");
            exit(1);
        }
        // im assigning to variables for clarity
        int pipe_read = pipe_fds[0];
        int pipe_write = pipe_fds[1];

        // unless stdin has already been closed an remapped
        if(pipe_read != 0 && pipe_write !=0) {
            // close stdin
            // I don't like it when subprocesses change file pos
            close(0);
        }

        // create the two processes moving stdout of the 
        // first to the in pipe fd and moving the stdin of
        // the second to the out pipe fd
        int chpid1;
        int chpid2;
        int status1;
        int status2;
        if((chpid1 = fork())) {
            // do nothing here
        }
        else {
            // make sure to close the pipe side we aren't using
            close(pipe_read);
            // replace stdout with write end of pipe
            if(dup2(pipe_write,1) != 1){
                puts("cannot properly dup pipe\n");
                exit(1);
            }
            // cmd should be ready now
            exit(eval(left, hh));
        }

        // in our case the return value is the second process 
        if((chpid2 = fork())) {
            // close the write end of the pipe
            close(pipe_write);
            close(pipe_read);
            // wait for processes to finish
            waitpid(chpid1, &status1, 0);
            waitpid(chpid2, &status2, 0);
        }
        else {
            // replace stdin with read end of pipe
            close(pipe_write);
            int file = dup2(pipe_read, 0);
            if(file != 0) {
                printf("unable to dupe read (%d, %d)\n", file, pipe_read);
                printf("error = %d\n", errno);
                exit(1);
            }
            // program should be ready for execution
            exit(eval(right, hh));

        }
        exit(status2);
    }
}

// evaluates semicolon operation
int
eval_semicolon(ast_node* left, ast_node* right, hashmap* hh)
{
    if(left) {
        eval(left, hh);
    }
    if(right){
        return eval(right, hh);
    }
}

// evaluates background operation
int
eval_background(ast_node* left, ast_node* right, hashmap* hh)
{
    // executes the command on the left, no waiting
    // if there is a command on the right, evaluate that
    // immediatly as well
    if(left) {
        if(fork()) {
            if(right) {
                return eval(right, hh);
            }
            return 0;
        }
        else {
            close(0);
            exit(eval(left, hh));
        }
    }
    return 0;
}
