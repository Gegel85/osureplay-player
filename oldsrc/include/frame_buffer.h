#ifndef OSUREPLAY_PLAYER_FRAME_BUFFER_H
#define OSUREPLAY_PLAYER_FRAME_BUFFER_H

#include <SFML/Graphics.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __PRETTY_FUNCTION__
#define display_warning(msg, ...) fprintf(stderr, "%s() %s:%u: "msg, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define display_warning(msg, ...) fprintf(stderr, "%s() %s:%u: "msg, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define display_error(msg, ...) display_warning(msg, ##__VA_ARGS__), exit(2)

typedef struct FrameBuffer {
	sfColor		**content;
	sfVector2u	size;
} FrameBuffer;

struct ReplayPlayerState;

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
void	FrameBuffer_encode(FrameBuffer *buffer, struct ReplayPlayerState *state);

#endif //OSUREPLAY_PLAYER_FRAME_BUFFER_H