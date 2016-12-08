#include "taglist.h"

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
