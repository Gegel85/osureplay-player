#ifndef OSUREPLAY_PLAYER_UTILS_H
#define OSUREPLAY_PLAYER_UTILS_H

#include <osu_map_parser.h>
#include <SFML/System.h>
#include "dict.h"
#include "frame_buffer.h"

sfVector2f	getTextSize(const char *str, unsigned charSize);
unsigned char	calcAlpha(OsuMap_hitObject obj, unsigned long totalTicks);
unsigned	getNbrLen(unsigned long nbr, unsigned base);
void		displayNumber(FrameBuffer *frameBuffer, unsigned number, sfVector2i pos, Dict *images, unsigned char alpha, unsigned textSize, const char *font);

#endif //OSUREPLAY_PLAYER_UTILS_H
