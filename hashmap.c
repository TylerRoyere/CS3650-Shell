
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// TODO: sudo apt install libbsd-dev
// This provides strlcpy
// See "man strlcpy"
#include <bsd/string.h>
#include <string.h>

#include "hashmap.h"


int
hash(char* key)
{
    // TODO: Produce an appropriate hash value.
    // my best attempt at a rudimentary hash function
    int hash = 0;
    while(*key) {
        hash = (hash * 31) ^ *(key++);
    }
    return hash;
}

hashmap*
make_hashmap_presize(int nn)
{
    hashmap* hh = calloc(1, sizeof(hashmap));
    // TODO: Allocate and initialize a hashmap with capacity 'nn'.
    // Double check "man calloc" to see what that function does.
    hh->capacity = nn;
    hh->data = calloc(nn, sizeof(hashmap_pair));

    // since we used calloc, size should already be 0
    return hh;
}

hashmap*
make_hashmap()
{
    return make_hashmap_presize(4);
}

void
free_hashmap(hashmap* hh)
{
    // free all of the copied strings
    for(int ii = 0; ii < hh->capacity; ++ii) {
        free(hh->data[ii].key);
        free(hh->data[ii].val);
    }
    
    // TODO: Free all allocated data.
    // free the pairs array
    free(hh->data);

    // free the hashmap structure
    free(hh);
}

int
hashmap_has(hashmap* hh, char* kk)
{
    return hashmap_get(hh, kk) != 0;
}

char*
hashmap_get(hashmap* hh, char* kk)
{
    // TODO: Return the value associated with the
    // key kk.
    // Note: return -1 for key not found.
    // get the hash for this key, size to hashmap
    int ii = hash(kk) & (hh->capacity - 1);
    int start = ii;

    // time to use linear probing to find a matching key
    hashmap_pair pair;
    do {
        // get the pair at the first index
        pair = hh->data[ii++];
        // if the pair is used and the string matches return val
        if(pair.used && !strcmp(pair.key, kk)) {
            return pair.val;
        }
        // use logic similar to index generation to wrap index
        ii &= (hh->capacity-1);
    } while((pair.used || pair.tomb) && ii != start);

    // so at the specified index, the pair is neither a tomb nor being used
    // the pair doesn't exist, if the index wraps back around, it isn't in
    // the hashmap either

    return 0;
}

void
hashmap_put(hashmap* hh, char* kk, char* vv)
{
    // TODO: Insert the value 'vv' into the hashmap
    // for the key 'kk', replacing any existing value
    // for that key.
    // check if the key exists in the hashmap
    if(hashmap_has(hh, kk)) {
        // the key exists in the hashmap
        int ii = hash(kk) & hh->capacity-1;
        // use linear probing to find the key-value pair
        hashmap_pair* pair;
        do{
            // need to get a pointer to the currenty pair so we can change it
            pair = &(hh->data[ii++]);
            // ensure index wraps if necessary
            ii &= hh->capacity-1;
            // continue to loop until the key is found
        } while((pair->used && strcmp(kk, pair->key)) || pair->tomb);
        // update value
        free(pair->val);
        pair->val = strdup(vv);
        return;
    }
    // there was no pair for this key, need to add it
    // first check against load factor (0.5)
    if( (hh->size + 1) > (hh->capacity/2) ) {
        // make a new hashmap that's twice as large
        
        // make copy of pair array as temporary
        int cc = hh->capacity;
        hashmap_pair aa[cc];
        memcpy(aa, hh->data, cc * sizeof(hashmap_pair));

        // free the old array, allocate a new one twice as large
        free(hh->data);
        hh->data = calloc(cc * 2, sizeof(hashmap_pair));
        hh->capacity = cc*2;

        // place all the used key:value pairs into the new hashmap 
        for(int ii = 0; ii < cc; ++ii) {
            if(aa[ii].used) {
                hashmap_put(hh, aa[ii].key, aa[ii].val);
                free(aa[ii].key);
                free(aa[ii].val);
            }
        }
    }
    // add the new key value to the table
    int ii = hash(kk) & (hh->capacity-1);

    // find a pair not being used using linear probing, tomb doesn't matter here
    hashmap_pair* pair;
    do {
        pair = &(hh->data[ii++]);
        // ensure index wraps correctly
        ii &= (hh->capacity-1);
    } while(pair->used);

    // we have found an unused pair to place the key:value into
    pair->key = strdup(kk);
    pair->val = strdup(vv);
    pair->used = 1;
    pair->tomb = 0;

    // now we need to increment size of the hashmap
    ++(hh->size);
}

void
hashmap_del(hashmap* hh, char* kk)
{
    // TODO: Remove any value associated with
    // this key in the map.
    // check if this map has this key
    if(!hashmap_has(hh, kk)) {
        return;
    }
    // so we have this key, let's find it
    int ii = hash(kk) & (hh->capacity-1);
    hashmap_pair* pair = &(hh->data[ii]);

    // lets try it not using a do-while
    // while the pair is used and the string doesn't match, continue probing
    for(; pair->used && strcmp(kk, pair->key); ++ii) {
        // make sure index wraps properly
        ii &= hh->capacity-1;

        // get the next pair data
        pair = &(hh->data[ii]);
    }
    free(pair->val);
    free(pair->key);
    // get the pointer to the pair and "delete it"
    memset(pair, 0, sizeof(*pair));
    pair->tomb = 1;
}

hashmap_pair
hashmap_get_pair(hashmap* hh, int ii)
{
    // TODO: Get the {k,v} pair stored in index 'ii'.
    return hh->data[ii];

}

void
hashmap_dump(hashmap* hh)
{
    printf("== hashmap dump ==\n");
    // TODO: Print out all the keys and values currently
    // in the map, in storage order. Useful for debugging.
    hashmap_pair pair;
    for(int ii = 0; ii < hh->capacity; ++ii) {
        pair = hashmap_get_pair(hh, ii);
        printf("%s\t%s\t%s\t%s\n", pair.key, pair.val, 
                pair.tomb?"tomb":"live", pair.used?"used":"unused");
    }
}
