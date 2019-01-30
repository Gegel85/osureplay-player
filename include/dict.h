#ifndef OSUREPLAY_PLAYER_DICT_H
#define OSUREPLAY_PLAYER_DICT_H

typedef struct Dict {
	void		*data;
	const char	*index;
	struct Dict	*next;
	void		(*destroy)(void *data);
} Dict;

bool	Dict_addElement(Dict *dict, const char *index, void *data, void (*destroy)(void *));
void	*Dict_getElement(Dict *dict, const char *index);
void	Dict_destroy(Dict *dict, bool isStatic);

#endif //OSUREPLAY_PLAYER_DICT_H
