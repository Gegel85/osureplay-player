/*
** EPITECH PROJECT, 2019
** osureplay_player
** File description:
** sound.h
*/

#ifndef OSUREPLAY_PLAYER_SOUND_H
#define OSUREPLAY_PLAYER_SOUND_H


#include <SFML/Audio.h>

#define SAMPLE_RATE 44100

typedef struct Sound {
	short		**data;
	size_t		*length;
	unsigned	nbChannels;
	unsigned	sampleRate;
	unsigned	bitsPerSample;
	void		(*destroyer)(struct Sound *);
} Sound;

typedef struct PlayingSound {
	Sound	*sound;
	double	pitch;
	double	speed;
	double	pos;
} PlayingSound;

struct replayPlayerState;

void	playSound(struct replayPlayerState *state, char *sound, double pitch, double speed);
void	encodePlayingSounds(struct replayPlayerState *state);
Sound	*loadSoundFile(char *path);
void	destroySound(Sound *sound);


#endif //OSUREPLAY_PLAYER_SOUND_H
