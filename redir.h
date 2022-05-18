#ifndef REDIR_H
#define REDIR_H

// redirects stdout to specified file, non-zero return for failure
int redir_outfile(const char* filename);

// redirects stdin to specified file, non-zero return for failure
int redir_infile(const char* filename);

// places fd_2 at fd_1, non-zero return for failure
int replace_fd(int fd_1, int fd_2);

#endif
