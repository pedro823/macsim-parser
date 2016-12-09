#ifndef __TAGLIST_H__
#define __TAGLIST_H__

// Lista ligada de tags a se corrigir
typedef struct t_s {
    char* tagName;
    int linePos;
    struct t_s* next;
} Tag;

typedef Tag* _Tag;

_Tag tag_create();

void tag_insert(_Tag* head, char* jumpName, int linePos);

void tag_destroy(_Tag head);



#endif
