#include <malloc.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <libavutil/imgutils.h>
#include <utils.h>
#include "frame_buffer.h"
#include "replay_player.h"

void	FrameBuffer_clear(FrameBuffer *this, sfColor color)
{
	for (unsigned x = 0; x < this->size.x; x++)
		for (unsigned y = 0; y < this->size.y; y++) {
			this->content[y][x] = color;
			this->content[y][x].a = 255;
		}
}

void	FrameBuffer_init(FrameBuffer *this, sfVector2u size)
{
	if (!this)
		return;

	this->size = size;
	this->content = malloc(size.y * sizeof(*this->content));
	if (!this->content)
		display_error("Memory allocation error (%luB)\n", size.y * (unsigned long)sizeof(*this->content));
	*this->content = calloc(size.y * size.x, sizeof(**this->content));
	if (!*this->content)
		display_error("Memory allocation error (%luB)\n", size.y * size.x * (unsigned long)sizeof(**this->content));
	for (unsigned i = 0; i < size.y; i++)
		this->content[i] = &(*this->content)[i * size.x];
}

void	FrameBuffer_destroy(FrameBuffer *this)
{
	free(*this->content);
	free(this->content);
}

void	FrameBuffer_drawPixel(FrameBuffer *this, sfVector2i pos, sfColor color)
{
	double	a = 0;

	if (this->content[pos.y][pos.x].a != 255)
		return;
	a = color.a / 255.;
	this->content[pos.y][pos.x].r += (color.r - this->content[pos.y][pos.x].r) * a;
	this->content[pos.y][pos.x].g += (color.g - this->content[pos.y][pos.x].g) * a;
	this->content[pos.y][pos.x].b += (color.b - this->content[pos.y][pos.x].b) * a;
}

void	FrameBuffer_drawPoint(FrameBuffer *this, sfVector2f pos, sfColor color)
{
	if (pos.x < 0 || pos.y < 0 || pos.x >= (int)this->size.x || pos.y >= (int)this->size.y)
		return;
	FrameBuffer_drawPixel(this, (sfVector2i){pos.x, pos.y}, color);
	if ((int)pos.x != pos.x)
		FrameBuffer_drawPixel(this, (sfVector2i){pos.x + 1, pos.y}, color);
	if ((int)pos.y != pos.y)
		FrameBuffer_drawPixel(this, (sfVector2i){pos.x, pos.y + 1}, color);
	if ((int)pos.x != pos.x && (int)pos.y != pos.y)
		FrameBuffer_drawPixel(this, (sfVector2i){pos.x + 1, pos.y + 1}, color);
}

void	FrameBuffer_drawRectangle(FrameBuffer *this, sfVector2i pos, sfVector2u size, sfColor color)
{
	for (unsigned x = 0; x < size.x; x++)
		FrameBuffer_drawPoint(this, (sfVector2f){x + pos.x, pos.y}, color);
	for (unsigned x = 0; x < size.x; x++)
		FrameBuffer_drawPoint(this, (sfVector2f){x + pos.x, pos.y + size.y - 1}, color);
	for (unsigned y = 0; y < size.x; y++)
		FrameBuffer_drawPoint(this, (sfVector2f){pos.x, pos.y + y}, color);
	for (unsigned y = 0; y < size.x; y++)
		FrameBuffer_drawPoint(this, (sfVector2f){pos.x + size.x - 1, pos.y + y}, color);
}

void	FrameBuffer_drawFilledRectangle(FrameBuffer *this, sfVector2i pos, sfVector2u size, sfColor color)
{
	for (unsigned x = 0; x < size.x; x++)
		for (unsigned y = 0; y < size.y; y++)
			FrameBuffer_drawPoint(this, (sfVector2f){x + pos.x, y + pos.y}, color);
}

void	FrameBuffer_drawImage(FrameBuffer *this, sfVector2i pos, sfImage *image, sfVector2i newSize, sfColor tint, bool centered, float rotation)
{
	if (!image)
		return;

	const sfColor	*array = (const sfColor *)sfImage_getPixelsPtr(image);
	sfVector2u	size = sfImage_getSize(image);
	sfVector2f	scale = {
		newSize.x < 0 ? 1 : ((float)newSize.x / size.x),
		newSize.y < 0 ? 1 : ((float)newSize.y / size.y)
	};
	double	c;
	double	s;
	sfColor col;

	c = cos(rotation);
	s = sin(rotation);
	if (centered) {
		pos.x -= (size.x * scale.x) / 2;
		pos.y -= (size.y * scale.y) / 2;
	}

	for (unsigned x = 0; x < size.x * scale.x; x++)
		for (unsigned y = 0; y < size.y * scale.y; y++) {
			if ((int)(y / scale.y) >= size.y || (int)(x / scale.x) >= size.x)
				col = (sfColor){0, 0, 0, 0};
			else
				col = (sfColor) {
					array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].r * (tint.r / 255.),
					array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].g * (tint.g / 255.),
					array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].b * (tint.b / 255.),
					array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].a * (tint.a / 255.),
				};
			if (rotation == 0) {
				FrameBuffer_drawPoint(this, (sfVector2f) {
					pos.x + x,
					pos.y + y
				}, col);
			} else {
				FrameBuffer_drawPoint(this, (sfVector2f) {
					c * (x - newSize.x / 2.) - s * (y - newSize.y / 2.) + newSize.x / 2. + pos.x,
					s * (x - newSize.x / 2.) + c * (y - newSize.y / 2.) + newSize.y / 2. + pos.y
				}, col);
			}
		}
}

void	FrameBuffer_drawFilledCircle(FrameBuffer *this, sfVector2i pos, int radius, sfColor color)
{
	for (int x = 0; x < radius * 2; x++)
		for (int y = 0; y < radius * 2; y++)
			if (sqrt(pow(radius - x, 2) + pow(radius - y, 2)) <= radius)
				FrameBuffer_drawPoint(this, (sfVector2f) {x + pos.x, y + pos.y}, color);
}

void	FrameBuffer_drawCircle(FrameBuffer *this, unsigned thickness, sfVector2i pos, int radius, sfColor color)
{
	double	distance = 0;

	for (int x = -thickness; x < (radius * 2 + thickness); x++)
		for (int y = -thickness; y < (radius * 2 + thickness); y++) {
			distance = sqrt(pow(x - radius, 2) + pow(y - radius, 2));
			if (distance >= radius && distance <= radius + thickness) {
				FrameBuffer_drawPoint(this, (sfVector2f) {x + pos.x, y + pos.y}, color);
			}
		}
}

void	FrameBuffer_draw(FrameBuffer *this, sfRenderWindow *window)
{
	sfImage		*image = sfImage_createFromPixels(this->size.x, this->size.y, (sfUint8 *)*this->content);
	sfTexture	*texture = sfTexture_createFromImage(image, NULL);
	sfSprite	*sprite = sfSprite_create();

	sfSprite_setTexture(sprite, texture, true);
	sfRenderWindow_drawSprite(window, sprite, NULL);
	sfImage_destroy(image);
	sfTexture_destroy(texture);
	sfSprite_destroy(sprite);
}

void	FrameBuffer_encode(FrameBuffer *this, ReplayPlayerState *state)
{
	fflush(stdout);

	/* make sure the frame data is writable */
	if (av_frame_make_writable(state->videoFrame) < 0)
		display_error("The frame data is not writable\n");

	/* prepare the frame */
	/* Y */
	for (int y = 0; y < state->videoCodecContext->height; y++) {
		for (int x = 0; x < state->videoCodecContext->width; x++) {
			state->videoFrame->data[0][y * state->videoFrame->linesize[0] + x] =
				0.299 * this->content[y][x].r +
				0.587 * this->content[y][x].g +
				0.114 * this->content[y][x].b;
		}
	}

	/* Cb Cr */
	for (int y = 0; y < state->videoCodecContext->height / 2; y++) {
		for (int x = 0; x < state->videoCodecContext->width / 2; x++) {
			state->videoFrame->data[1][y * state->videoFrame->linesize[1] + x] =
				-0.1687 * this->content[y * 2][x * 2].r +
				-0.3313 * this->content[y * 2][x * 2].g +
				0.5 *     this->content[y * 2][x * 2].b + 128;
			state->videoFrame->data[2][y * state->videoFrame->linesize[2] + x] =
				0.5 *     this->content[y * 2][x * 2].r +
				-0.4187 * this->content[y * 2][x * 2].g +
				-0.0813 * this->content[y * 2][x * 2].b + 128;
		}
	}

	state->videoFrame->pts = state->frameNb;

	printf("Rendering frame% *i / %li\n", getNbrLen(state->totalFrames, 10) + 1, state->frameNb, state->totalFrames);

	/* encode the image */
	encodeVideoFrame(state->videoCodecContext, state->videoFrame, state->videoPacket, state->videoStream);
}
