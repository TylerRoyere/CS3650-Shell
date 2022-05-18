#include <bsd/string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "svec.h"
#include "tokenize.h"

static void get_tokens(svec*, char*);
// it is important that the longest operators come first, 
// otherwise the shorter one will match instead of the longer one
static char* operators[] = {"&&", "||", "&", "|", ";", "<", ">", "(", ")"};

// makes a svec with each entry being a token from the input string
svec* 
tokenize(char* string)
{
    // make a vector, get the tokens in the string 
    // and put them in the vector 
    svec* sv = make_svec();
    get_tokens(sv, string);
    return sv;
}

// if the string starts with one of the operators defined above
// returns the operator string that matched, NULL otherwise
static
char*
is_operator(char* string)
{
    
    // check if the start of the string is an operator
    for(int ii = 0; ii < 9; ++ii){
        int op_len = strlen(operators[ii]);
        // this is ok since the longer operators are checked first
        if(strncmp(string, operators[ii], op_len) == 0){
            return operators[ii];
        }
    }
    return 0;
} 

static 
void 
get_tokens(svec* sv, char* string)
{
    // if the string starts with a null, return
    if(!(*string)){
        return;
    }
    // if string starts with whitespace, tokenize
    // the string starting at the next character
    if(isspace(*string)){
        get_tokens(sv, string + 1);
        return;
    }

    // if the string starts with " get the string that ends at the other "
    if(*string == '"') {
        // get start and end of quoted string
        char* start = string + 1;
        char* end = start;
        // move end down until the ending quotes
        while(*end && *end != '"') {
            ++end;
        }
        // the end of the string is here
        char tok_buf[end-start + 2 ];
        strlcpy(tok_buf, start, end-start+1);
        svec_push_back(sv, tok_buf);
        // get the tokens in the rest of the string
        if(*end) {
            get_tokens(sv, end+1);
        }
        return;
    }
    
    // if the string starts with an operator append to 
    // token vector and get tokens starting after operator
    char* op = is_operator(string);
    if(op){
        svec_push_back(sv, op);
        // need to move start of string forward by
        // the length of the operator found
        get_tokens(sv, string + strlen(op));
        return;
    }

    // otherwise find the end of what must be a command or argument
    char* end = string + 1;
    while(*end) {
        // if end is whitespace or an operator, we found the
        // end of the command/argument
        if(isspace(*end) || is_operator(end)){
            // if string is the start of the word
            // and end is the character after it ends
            // (end-string) is 1 less than the length
            // of the token
            int tok_len = end-string + 1;
            char tok_buf[tok_len + 1]; // for null terminator
            strlcpy(tok_buf, string, tok_len);
            svec_push_back(sv, tok_buf);
            // get the tokens in the rest of the string
            get_tokens(sv, end);
            return;
        }
        // move end forward 
        end++;
    }
}
