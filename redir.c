#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


int
redir_outfile(const char* filename)
{
    // moves stdout to the specified filename
    //
    // open a file with filename, create if needed, writeonly
    int fd_out = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    // check fd
    if(fd_out < 0) {
        printf("Unable to open file: %s\n", filename);
        return -1;
    }

    // redirect stdout to the specified file for writing
    close(1);
    if(1 != dup(fd_out)) {
        puts("unable to redirect stdout\n");
        return -1;
    }
    // need to make sure this process doesn't read from stdin
    close(0);
    return 0;
}

int
redir_infile(const char* filename)
{
    // moves stdin to read the specified filename
    //
    // open the filename, readonly
    int fd_in = open(filename, O_RDONLY, 0444);

    // check file opened
    if(fd_in < 0) {
        printf("Unable to open file: %s\n", filename);
        return -1;
    }

    // try to redirect stdin to filename
    close(0);
    if(0 != dup(fd_in)) {
        puts("unable to redirect stdin\n");
        return -1;
    }

    return 0;
}

int
replace_fd(int fd_1, int fd_2)
{
    // just call dup2 for now
    if(fd_2 != dup2(fd_1, fd_2)) {
        puts("unable to redirect fd1 to fd2\n");
        return -1;
    }

    return 0;
}
