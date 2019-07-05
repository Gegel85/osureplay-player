#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include "dict.h"

bool	Dict_addElement(Dict *dict, const char *index, void *data, void (*destroy)(void *))
{
	if (!index || !dict)
		return false;
	for (; dict->next && dict->index && strcmp(index, dict->index) != 0; dict = dict->next);
	if (dict->index && strcmp(index, dict->index) == 0) {
		if (dict->destroy)
			dict->destroy(dict->data);
	} else {
		if (dict->index) {
			dict->next = calloc(1, sizeof(*dict->next));
			if (!dict->next)
				return false;
			dict = dict->next;
		}
		dict->index = strdup(index);
		if (!dict->index)
			return false;
	}
	dict->data = data;
	dict->destroy = destroy;
	return true;
}

void	*Dict_getElement(Dict *dict, const char *index)
{
	if (!index)
		return NULL;
	for (; dict && dict->index; dict = dict->next)
		if (strcmp(dict->index, index) == 0)
			return dict->data;
	return NULL;
}

void	Dict_destroy(Dict *dict, bool isStatic)
{
	if (dict->next)
		Dict_destroy(dict->next, false);
	if (dict->destroy)
		dict->destroy(dict->data);
	if (!isStatic)
		free(dict);
}