#include <SFML/Graphics.h>
#include <osu_map_parser.h>
#include <string.h>
#include "dict.h"
#include "frame_buffer.h"
#include "defines.h"
#include "utils.h"

sfVector2f	getTextSize(const char *str, unsigned charSize)
{
	sfVector2f	size = {0, charSize};
	float		buffer = 0;

	for (int i = 0; str[i]; i++) {
		if (str[i] == '\n') {
			size.y += charSize;
			size.x = size.x >= buffer ? size.x : buffer;
			buffer = 0;
		} else
			buffer += charSize;
	};
	size.x = size.x >= buffer ? size.x : buffer;
	return size;
}

unsigned char	calcAlpha(OsuMap_hitObject obj, unsigned long totalTicks)
{
	if ((long)(obj.timeToAppear - totalTicks) <= 400)
		return BASE_OBJ_ALPHA;
	return (unsigned char)((long)(obj.timeToAppear - totalTicks - 400) * -BASE_OBJ_ALPHA / 400 + BASE_OBJ_ALPHA);
}

unsigned getNbrLen(unsigned long nbr, unsigned base)
{
	unsigned len = 0;

	do {
		len++;
		nbr /= base;
	} while (nbr);
	return len;
}

void	displayNumber(FrameBuffer *frameBuffer, unsigned number, sfVector2i pos, Dict *images, unsigned char alpha, unsigned textSize, const char *font)
{
	char		buffer[11];
	char		buff[11];
	char		image[10];
	sfVector2f	size;

	memset(buff, 0, sizeof(buff));
	sprintf(buffer, "%u", number);
	size = getTextSize(buffer, textSize);
	for (int i = 0; buffer[i]; i++) {
		buff[i] = buffer[i];
		sprintf(image, "%s-%c", font, buffer[i]);
		FrameBuffer_drawImage(
			frameBuffer,
			(sfVector2i){
				pos.x - size.x / 2 + getTextSize(buff, textSize).x - 7.5,
				pos.y - size.y / 2 + getTextSize(buff, textSize).y - 7.5,
			},
			Dict_getElement(
				images,
				image
			),
			(sfVector2i){15, 15},
			(sfColor){255, 255, 255, alpha},
			true,
			0
		);
	}
}