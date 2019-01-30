#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "frame_buffer.h"

void	display_error(char *msg)
{
	write(2, msg, strlen(msg));
	abort();
}

void	mergeColors(sfColor *col1, sfColor col2)
{

}

void	FrameBuffer_clear(FrameBuffer *buffer, sfColor color)
{
	for (unsigned x = 0; x < buffer->size.x; x++)
		for (unsigned y = 0; y < buffer->size.y; y++)
			buffer->content[y][x] = color;
}

void	FrameBuffer_init(FrameBuffer *buffer, sfVector2u size)
{
	if (!buffer)
		return;
	buffer->size = size;
	buffer->content = malloc(size.y * sizeof(*buffer->content));
	if (!buffer->content)
		display_error("Memory allocation error\n");
	*buffer->content = malloc(size.y * size.x * sizeof(**buffer->content));
	if (!*buffer->content)
		display_error("Memory allocation error\n");
	memset(*buffer->content, 0, size.y * size.x * sizeof(**buffer->content));
	for (unsigned i = 0; i < size.y; i++)
		buffer->content[i] = &(*buffer->content)[i * size.x];
}

void	FrameBuffer_destroy(FrameBuffer *buffer)
{
	free(*buffer->content);
	free(buffer->content);
}

void	FrameBuffer_drawPoint(FrameBuffer *buffer, sfVector2i pos, sfColor color)
{
	double	a = 0;

	if (pos.x < 0 || pos.y < 0 || pos.x >= (int)buffer->size.x || pos.y >= (int)buffer->size.y)
		return;
	a = color.a / 255.;
	buffer->content[pos.y][pos.x].r += (color.r - buffer->content[pos.y][pos.x].r) * a;
	buffer->content[pos.y][pos.x].g += (color.g - buffer->content[pos.y][pos.x].g) * a;
	buffer->content[pos.y][pos.x].b += (color.b - buffer->content[pos.y][pos.x].b) * a;
}

void	FrameBuffer_drawRectangle(FrameBuffer *buffer, sfVector2i pos, sfVector2u size, sfColor color)
{
	for (unsigned x = 0; x < size.x; x++)
		FrameBuffer_drawPoint(buffer, (sfVector2i){x + pos.x, pos.y}, color);
	for (unsigned x = 0; x < size.x; x++)
		FrameBuffer_drawPoint(buffer, (sfVector2i){x + pos.x, pos.y + size.y - 1}, color);
	for (unsigned y = 0; y < size.x; y++)
		FrameBuffer_drawPoint(buffer, (sfVector2i){pos.x, pos.y + y}, color);
	for (unsigned y = 0; y < size.x; y++)
		FrameBuffer_drawPoint(buffer, (sfVector2i){pos.x + size.x - 1, pos.y + y}, color);
}

void	FrameBuffer_drawFilledRectangle(FrameBuffer *buffer, sfVector2i pos, sfVector2u size, sfColor color)
{
	for (unsigned x = 0; x < size.x; x++)
		for (unsigned y = 0; y < size.y; y++)
			FrameBuffer_drawPoint(buffer, (sfVector2i){x + pos.x, y + pos.y}, color);
}

void	FrameBuffer_drawImage(FrameBuffer *buffer, sfVector2i pos, sfImage *image, sfVector2i newSize, sfColor tint)
{
	if (!image)
		return;

	const sfColor	*array = (const sfColor *)sfImage_getPixelsPtr(image);
	sfVector2u	size = sfImage_getSize(image);
	sfVector2f	scale = {
		newSize.x < 0 ? 1 : ((float)newSize.x / size.x),
		newSize.y < 0 ? 1 : ((float)newSize.y / size.y)
	};
	sfColor col;

	for (unsigned x = 0; x < size.x * scale.x; x++)
		for (unsigned y = 0; y < size.y * scale.y; y++) {
			col = (sfColor) {
				array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].r * (tint.r / 255.),
				array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].g * (tint.g / 255.),
				array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].b * (tint.b / 255.),
				array[(int)(y / scale.y) * size.x + (int)(x / scale.x)].a * (tint.a / 255.),
			};
			FrameBuffer_drawPoint(buffer, (sfVector2i) {x + pos.x, y + pos.y}, col);
		}
}

void	FrameBuffer_drawFilledCircle(FrameBuffer *buffer, sfVector2i pos, int radius, sfColor color)
{
	for (int x = 0; x < radius * 2; x++)
		for (int y = 0; y < radius * 2; y++)
			if (sqrt(pow(radius - x, 2) + pow(radius - y, 2)) <= radius)
				FrameBuffer_drawPoint(buffer, (sfVector2i) {x + pos.x, y + pos.y}, color);
}

void	FrameBuffer_drawCircle(FrameBuffer *buffer, unsigned thickness, sfVector2i pos, int radius, sfColor color)
{
	double		distance = 0;

	for (int x = -thickness; x < radius * 2 + (int)thickness; x++)
		for (int y = -thickness; y < radius * 2 + (int)thickness; y++) {
			distance = sqrt(pow(x - radius, 2) + pow(y - radius, 2));
			if (distance >= radius && distance <= radius + thickness) {
				FrameBuffer_drawPoint(buffer, (sfVector2i) {x + pos.x, y + pos.y}, color);
			}
		}
}

void	FrameBuffer_draw(FrameBuffer *buffer, sfRenderWindow *window)
{
	sfImage		*image = sfImage_createFromPixels(buffer->size.x, buffer->size.y, *(sfUint8 **)buffer->content);
	sfTexture	*texture = sfTexture_createFromImage(image, NULL);
	sfSprite	*sprite = sfSprite_create();

	sfSprite_setTexture(sprite, texture, true);
	sfRenderWindow_drawSprite(window, sprite, NULL);
	sfImage_destroy(image);
	sfTexture_destroy(texture);
	sfSprite_destroy(sprite);
}