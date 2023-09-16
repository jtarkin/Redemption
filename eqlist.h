#ifndef EQLIST_H
#define EQLIST_H

typedef struct list_item LIST_ITEM;

struct list_item {
	LIST_ITEM *next;
	LIST_ITEM *prev;
	int  level;
	char *name;
	char area[MAX_STRING_LENGTH];
	char type[MAX_STRING_LENGTH];
	char wear[MAX_STRING_LENGTH];
	char stats[MAX_STRING_LENGTH];
};

void generate_eqlist(int format);

#endif
