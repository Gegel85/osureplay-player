/*
** EPITECH PROJECT, 2019
** osureplay_player
** File description:
** sound.h
*/

#ifndef OSUREPLAY_PLAYER_SOUND_H
#define OSUREPLAY_PLAYER_SOUND_H


#include <SFML/Audio.h>

typedef struct Sound {
	unsigned	*data;
	size_t		length;
} Sound;

typedef struct PlayingSound {
	Sound	*sound;
	double	pitch;
	double	pos;
} PlayingSound;

struct replayPlayerState;

void	playSound(struct replayPlayerState *state, char *sound, float pitch);
void	encodePlayingSounds(struct replayPlayerState *state);


#endif //OSUREPLAY_PLAYER_SOUND_H
