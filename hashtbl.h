#ifndef __HASHTBL_H__
#define __HASHTBL_H__

struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name; /* defined name */
    void *defn; /* replacement text */
};

typedef struct nlist *hashtbl;

struct nlist **htinit();
void htfree(struct nlist **ht);

/* hash: form hash value for string s */
unsigned hash(char *s);

/* lookup: look for s in hashtab */
struct nlist *htlookup(struct nlist **hashtab, char *s);

/* put: put (name, defn) in hashtab */
struct nlist *htput(struct nlist **hashtab, char *name, void *defn);
#endif
