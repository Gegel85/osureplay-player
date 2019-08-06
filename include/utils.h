#ifndef OSUREPLAY_PLAYER_UTILS_H
#define OSUREPLAY_PLAYER_UTILS_H

#include <osu_map_parser.h>
#include <SFML/System.h>

sfVector2f	getTextSize(const char *str, unsigned charSize);
unsigned char	calcAlpha(OsuMap_hitObject obj, unsigned long totalTicks);
unsigned	getNbrLen(unsigned long nbr, unsigned base);

#endif //OSUREPLAY_PLAYER_UTILS_H
