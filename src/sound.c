#include <SFML/Audio.h>
#include <stdbool.h>
#include <string.h>
#include "replay_player.h"
#include "defines.h"

void	playSound(replayPlayerState *state, char *index, double pitch, double speed)
{
	void	*elem = Dict_getElement(state->sounds, index);
	int	i = 0;
	PlayingSound	*best = NULL;

	if (!elem)
		return;
	if (!state->videoStream) {
		sfSoundBuffer *buffer = elem;
		static sfSound *sounds[nbOfSound];
		static int current = 0;
		static bool first = true;

		if (first) {
			for (; i < nbOfSound; i++)
				sounds[i] = sfSound_create();
			first = false;
		}
		sfSound_setBuffer(sounds[current], buffer);
		sfSound_setPitch(sounds[current], pitch);
		sfSound_play(sounds[current++]);
		current %= nbOfSound;
		return;
	}
	for (; state->playingSounds[i].sound; i++) {
		if (state->playingSounds[i].sound->length < state->playingSounds[i].pos * 2) {
			best = &state->playingSounds[i];
			break;
		}
	}
	if (!best) {
		state->playingSounds = realloc(state->playingSounds, sizeof(*state->playingSounds) * (i + 2));
		if (!state->playingSounds)
			display_error("Memory allocation error (%lu)\n", sizeof(*state->playingSounds) * (i + 2));
		memset(&state->playingSounds[i + 1], 0, sizeof(*state->playingSounds));
		best = &state->playingSounds[i];
	}
	memset(best, 0, sizeof(*best));
	best->sound = elem;
	best->speed = speed;
	best->pitch = pitch;
}

void	encodePlayingSounds(replayPlayerState *state)
{
	PlayingSound	*sounds = state->playingSounds;
	static double	total = 0;

	//total += state->audioCodecContext->frame_size - state->audioCodecContext->sample_rate / 60.;
	//if (total )
	/* encode a single tone sound */
	if (av_frame_make_writable(state->audioFrame) < 0)
		display_error("Frame is not writable");
	memset(state->audioFrame->data[0], 0, state->audioCodecContext->frame_size * 4);
	for (int i = 0; i < state->audioCodecContext->frame_size; i++) {
		for (int j = 0; sounds[j].sound; j++) {
			if (sounds[j].sound->length < sounds[j].pos * 2) {
				((short *)state->audioFrame->data[0])[i * 2] +=
					sounds[j].sound->data[(int)sounds[j].pos * 2] * sounds[j].pitch;
				((short *)state->audioFrame->data[0])[i * 2 + 1] +=
					sounds[j].sound->data[(int)sounds[j].pos * 2 + 1] * sounds[j].pitch;
				sounds[j].pos += sounds[j].speed;
			}
		}
	}

	encodeAudioFrame(state->audioCodecContext, state->audioFrame, state->audioPacket, state->audioStream);
}
