// Based on code from section 6.6 of The C Programming Language, posted at:
// http://stackoverflow.com/questions/4384359/quick-way-to-implement-dictionary-in-c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtbl.h"

#define HASHSIZE 101
//static struct nlist *hashtab[HASHSIZE]; /* pointer table */

struct nlist **htinit() 
{
    TODO:
    return (struct nlist **)calloc(HASHSIZE * sizeof(struct nlist *));
//    struct nlist **rv = (struct nlist **)malloc(sizeof(struct nlist **));
//    rv = &ht;
//
//    return rv;
//    return hashtab;
}
void htfree(struct nlist *ht)
{
    // JAE TODO: free table contents
    free(ht);
}

/* hash: form hash value for string s */
unsigned hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

/* lookup: look for s in hashtab */
struct nlist *htlookup(struct nlist **hashtab, char *s)
{
    struct nlist *np;
    for (np = hashtab[hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}

char *strdup(char *);
/* put: put (name, defn) in hashtab */
struct nlist *htput(struct nlist **hashtab, char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = htlookup(hashtab, name)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL)
          return NULL;
        hashval = hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = strdup(defn)) == NULL)
       return NULL;
    return np;
}

char *strdup(char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}
