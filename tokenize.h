#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "svec.h"

// tokenize the given input string into a vector, and returns the tokens
// inside a list that needs to be freed using free_svec()
svec* tokenize(char* string);

#endif
