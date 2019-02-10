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
	short	*data;
	size_t	length;
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
Sound	*loadWavFile(char *path);
Sound	*loadMp3File(char *path);
Sound	*loadOggFile(char *path);
void	destroySound(Sound *sound);


#endif //OSUREPLAY_PLAYER_SOUND_H
