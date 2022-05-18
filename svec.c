/* This file is lecture notes from CS 3650, Fall 2018 */
/* Author: Nat Tuck */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "svec.h"

svec*
make_svec()
{
    svec* sv = malloc(sizeof(svec));
    sv->data = malloc(2 * sizeof(char*));
    sv->size = 0;
    // TODO: correctly allocate and initialize data structure
    // setting initial svec capacity to 2, since that's 
    // the size already allocated
    sv->capacity = 2;
    return sv;
}

void
free_svec(svec* sv)
{
    // TODO: free all allocated data
    // free all of the strings
    for(int ii = 0; ii < sv->size; ++ii) {
        // make sure these strings are not NULL before free
        if(sv->data[ii]) {
            free(sv->data[ii]);
        }
    }
    // free the array of strings
    free(sv->data);

    // now the struct is ready for freeing
    free(sv);
}

char*
svec_get(svec* sv, int ii)
{
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

void
svec_put(svec* sv, int ii, char* item)
{
    assert(ii >= 0 && ii < sv->size);
    //sv->data[0] = item;
    // TODO: insert item into slot ii
    // Consider ownership of string in collection.

    // if this is a brand new entry from svec_push_back(), this does nothing
    // but if the string is being replaced this will correctly free it
    free(sv->data[ii]);

    // copy string for struct, needs to be freed by caller using free_svec()
    sv->data[ii] = strdup(item);
}

void
svec_push_back(svec* sv, char* item)
{
    int ii = sv->size;

    // TODO: expand vector if backing array
    // is not big enough
    if(ii >= sv->capacity) {
        // allocate new pointer array 2-times longer
        int new_capacity = sv->capacity * 2;
        sv->data = realloc(sv->data, new_capacity * sizeof(char*));

        // update vector capacity
        sv->capacity = new_capacity;
    }
    
    // update size
    sv->size = ii + 1;

    // make data pointer null to free() in svec_put() works nicely 
    sv->data[ii] = 0;
    svec_put(sv, ii, item);
}

void
svec_swap(svec* sv, int ii, int jj)
{
    // TODO: Swap the items in slots ii and jj
    // ensure the swap is even allowed
    assert(ii < sv->size && jj < sv->size);
    if(ii != jj) {
        // use temp to swap values
        char* temp = sv->data[ii];
        sv->data[ii] = sv->data[jj];
        sv->data[jj] = temp;
    }
}

svec*
svec_reverse(svec* sv)
{
    // copy the contents of the passed svec
    // into a new svec backwards
    svec* reversed = make_svec();
    
    // for each element in the starting svec
    // go backwards and push them into the
    // reversed one
    for(int ii = sv->size-1; ii >= 0; --ii){
        svec_push_back(reversed, svec_get(sv, ii));
    }

    return reversed;

}
