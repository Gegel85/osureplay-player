#include <SFML/Audio.h>
#include <stdbool.h>
#include "replay_player.h"
#include "defines.h"

void	playSound(replayPlayerState *state, char *index, float pitch)
{
	void	*elem = Dict_getElement(state->sounds, index);

	if (!elem)
		return;
	if (!state->stream) {
		sfSoundBuffer *buffer = elem;
		static sfSound *sounds[nbOfSound];
		static int current = 0;
		static bool first = true;

		if (first) {
			for (int i = 0; i < nbOfSound; i++)
				sounds[i] = sfSound_create();
			first = false;
		}
		sfSound_setBuffer(sounds[current], buffer);
		sfSound_setPitch(sounds[current], pitch);
		sfSound_play(sounds[current++]);
		current %= nbOfSound;
	}
}