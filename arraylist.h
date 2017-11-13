#ifndef ARRAYLIST_H
#define ARRAYLIST_H

// arraylist loosely adapted from svec

typedef struct arraylist {
	int amt;
	int max;
	char** data;
} arraylist;

arraylist* create_al();

void free_al(arraylist* al);

void al_put(arraylist* al, int i, char* item);

void al_put_in(arraylist* al, char* item);

char* al_get(arraylist* al, int ii);

int al_contains(arraylist* al, char* item);

#endif
