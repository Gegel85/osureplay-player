#include <SFML/Audio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "replay_player.h"
#include "defines.h"

void	playSound(replayPlayerState *state, char *index, double pitch, double speed)
{
	void	*elem = Dict_getElement(state->sounds, index);
	int	i = 0;
	PlayingSound	*best = NULL;

	if (!elem)
		return;
	if (!state->videoAvStream) {
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
		size_t len = 0;

		for (unsigned k = 0; k < state->playingSounds[i].sound->nbChannels; len = len < state->playingSounds[i].sound->length[k++] ? state->playingSounds[i].sound->length[k - 1] : len);
		if (len < state->playingSounds[i].pos * 2) {
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

#define data1b ((char **)sounds[j].sound->data)
#define data2b ((short **)sounds[j].sound->data)
#define data4b ((int **)sounds[j].sound->data)
#define cap(value, max, min) (value > max ? max : (value < min ? min : value))

void	encodePlayingSounds(replayPlayerState *state)
{
/*	PlayingSound	*sounds = state->playingSounds;
	short		*frameSampleBuffer = (short *)state->audioFrame->data[0];

	if (av_frame_make_writable(state->audioFrame) < 0)
		display_error("Frame is not writable");
	memset(frameSampleBuffer, 0, state->audioCodecContext->frame_size * sizeof(*frameSampleBuffer));
	for (int i = 0; i < state->audioCodecContext->frame_size; i++) {
		for (int j = 0; sounds[j].sound; j++) {
			if (sounds[j].sound->length[0] > sounds[j].pos) {
				switch (sounds[j].sound->bitsPerSample) {
				case 8:
					frameSampleBuffer[i] = cap(frameSampleBuffer[i] + data1b[0][(int)sounds[j].pos] * sounds[j].pitch, INT16_MIN, INT16_MAX);
					break;
				case 16:
					frameSampleBuffer[i] = cap(frameSampleBuffer[i] + data2b[0][(int)sounds[j].pos] * sounds[j].pitch, INT16_MIN, INT16_MAX);
					break;
				case 32:
					frameSampleBuffer[i] = cap(frameSampleBuffer[i] + data4b[0][(int)sounds[j].pos] * sounds[j].pitch, INT16_MIN, INT16_MAX);
				}
				sounds[j].pos += sounds[j].speed;
			}
		}
	}

	encodeAudioFrame(state->audioCodecContext, state->audioFrame, state->audioPacket, state->audioStream);*/
}
