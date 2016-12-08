#ifndef __TAGLIST_H__
#define __TAGLIST_H__

// Lista ligada de tags a se corrigir
typedef struct b_s {
    char* tagname;
    int linePos;
    struct b_s* next;
} Tag;

typedef Tag* _Tag;

_Tag tag_create();

void tag_insert(_Tag* head);

void tag_destroy(_Tag head);



#endif
