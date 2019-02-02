#include <SFML/Graphics.h>
#include <osu_map_parser.h>
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
