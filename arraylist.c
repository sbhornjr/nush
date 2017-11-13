#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arraylist.h"

arraylist*
create_al()
{
        arraylist* al = malloc(sizeof(arraylist));
        al->amt = 0;
        al->max = 4;
        al->data = malloc(4 * sizeof(char*));
        memset(al->data, 0, 4 * sizeof(char*));
        return al;
}

void
free_al(arraylist* al)
{
        for (int i = 0; i < al->amt; ++i) {
		if (al->data[i] != 0) {
		        free(al->data[i]);
		}
        }

        free(al->data);
        free(al);
}

void
al_put(arraylist* al, int i, char* item)
{
        assert(i >= 0 && i < al->amt);
	//free(al->data[i]);
        al->data[i] = strdup(item);
}

void
al_put_in(arraylist* al, char* item)
{
        int i = al->amt;

        if (i >= al->max) {
                al->max *= 2;
                al->data = (char**) realloc(al->data, al->max * sizeof(char*));
        }

        al->amt = i + 1;
        al_put(al, i, item);
}

char*
al_get(arraylist* al, int i)
{
	assert(i >= 0 && i < al->amt);
	return al->data[i];
}

int
al_contains(arraylist* al, char* item)
{
	for (int i = 0; i < al->amt; ++i) {
		if (strncmp(al_get(al, i), item, 4) == 0) {
			return i;
		}
	}

	return -1;
}
