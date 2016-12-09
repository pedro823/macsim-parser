#include "taglist.h"
#include <stdlib.h>
#include <string.h>

_Tag tag_create() {
    return NULL;
}

void tag_destroy(_Tag head) {
    _Tag aux, i;
    for(i = head; i != NULL; i = aux) {
        aux = i->next;
        free(i);
    }
}

void tag_insert(_Tag* head, char* jumpName, int linePos) {
    _Tag new = malloc(sizeof(struct t_s));
    new->tagName = malloc(strlen(jumpName));
    strcpy(new->tagName, jumpName);
    new->linePos = linePos;
    new->next = *head;
    *head = new;
}
