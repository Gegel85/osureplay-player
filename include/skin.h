#ifndef OSUREPLAY_PLAYER_SKIN_H
#define OSUREPLAY_PLAYER_SKIN_H

#include "dict.h"

enum filetype {
	SOUND,
	IMAGE,
};

typedef struct LoadingPair {
	void *(*creator)(const char *path);
	void (*destroyer)(void *obj);
	enum filetype	type;
} LoadingPair;

typedef struct OsuSkin {
	int i; //So MSVC stops crying
} OsuSkin;

char	*getFileName(char *path);
char	*strToLower(char *str);
bool	loadSkin(const char *folder, Dict *images, Dict *sounds, Dict *loaders);

#endif //OSUREPLAY_PLAYER_SKIN_H
