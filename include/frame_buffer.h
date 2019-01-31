#ifndef OSUREPLAY_PLAYER_FRAME_BUFFER_H
#define OSUREPLAY_PLAYER_FRAME_BUFFER_H

#include <SFML/Graphics.h>
#include <stdbool.h>

typedef struct FrameBuffer {
	sfColor		**content;
	sfVector2u	size;
} FrameBuffer;

void	display_error(char *msg);
void	FrameBuffer_clear(FrameBuffer *buffer, sfColor color);
void	FrameBuffer_draw(FrameBuffer *buffer, sfRenderWindow *window);
void	FrameBuffer_init(FrameBuffer *buffer, sfVector2u size);
void	FrameBuffer_destroy(FrameBuffer *buffer);
void	FrameBuffer_drawPoint(FrameBuffer *buffer, sfVector2f pos, sfColor color);
void	FrameBuffer_drawRectangle(FrameBuffer *buffer, sfVector2i pos, sfVector2u size, sfColor color);
void	FrameBuffer_drawFilledRectangle(FrameBuffer *buffer, sfVector2i pos, sfVector2u size, sfColor color);
void	FrameBuffer_drawImage(FrameBuffer *buffer, sfVector2i pos, sfImage *image, sfVector2i newSize, sfColor tint, bool centered, float rotation);
void	FrameBuffer_drawCircle(FrameBuffer *buffer, unsigned thinkness, sfVector2i pos, int radius, sfColor color);
void	FrameBuffer_drawFilledCircle(FrameBuffer *buffer, sfVector2i pos, int radius, sfColor color);

#endif //OSUREPLAY_PLAYER_FRAME_BUFFER_H
