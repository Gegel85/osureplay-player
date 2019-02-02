#include <SFML/Audio.h>
#include <stdbool.h>
#include "defines.h"

void	playSound(sfSoundBuffer *buffer)
{
	static sfSound	*sounds[nbOfSound];
	static int	current = 0;
	static bool	first = true;

	if (first) {
		for (int i = 0; i < nbOfSound; i++)
			sounds[i] = sfSound_create();
		first = false;
	}
	if (!buffer)
		return;
	sfSound_setBuffer(sounds[current], buffer);
	sfSound_play(sounds[current++]);
	current %= nbOfSound;
}