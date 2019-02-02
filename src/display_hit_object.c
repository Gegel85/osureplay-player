//
// Created by Gegel85 on 02/02/2019.
//

#include <stdbool.h>
#include <osu_map_parser.h>
#include <SFML/Graphics.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "dict.h"
#include "frame_buffer.h"

unsigned	getLastObjToDisplay(unsigned currentGameHitObject, unsigned currentTimingPoint, OsuMap *beatmap, unsigned long totalTicks)
{
	unsigned	end = currentGameHitObject;

	while (true) {
		if (end >= beatmap->hitObjects.length)
			break;
		if (beatmap->hitObjects.content[end].timeToAppear - 800 > totalTicks)
			break;
		end++;
	}
	return end;
}

void	displayApproachCircle(sfColor color, OsuMap_hitObject *object, double circleSize, unsigned long ticks, Dict *images)
{
	if (object->timeToAppear < ticks)
		return;
	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		Dict_getElement(
			images,
			"approachcircle"
		),
		(sfVector2i){
			(54.4f - 4.48f * (float)circleSize + (object->timeToAppear - ticks) / 8.f) * 2,
			(54.4f - 4.48f * (float)circleSize + (object->timeToAppear - ticks) / 8.f) * 2
		},
		color,
		true,
		0
	);
}

void	displayCombo(unsigned combo, sfVector2i pos, Dict *images, unsigned char alpha)
{
	char		buffer[11];
	char		buff[11];
	char		image[10];
	sfVector2f	size;

	memset(buff, 0, sizeof(buff));
	sprintf(buffer, "%u", combo);
	size = getTextSize(buffer, 15);
	for (int i = 0; buffer[i]; i++) {
		buff[i] = buffer[i];
		sprintf(image, "default-%c", buffer[i]);
		FrameBuffer_drawImage(
			&frame_buffer,
			(sfVector2i){
				pos.x - size.x / 2 + getTextSize(buff, 15).x - 7.5,
				pos.y - size.y / 2 + getTextSize(buff, 15).y - 7.5,
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

void	displaySpinner(OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images)
{
	long	duration = *(long *)object->additionalInfos - object->timeToAppear;
	long	remaining = *(long *)object->additionalInfos - totalTicks;
	float	radius = object->timeToAppear >= totalTicks ? 400 : 4 * (100 - (duration - remaining) * 100.f / duration);

	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){
			320,
			240
		},
		Dict_getElement(
			images,
			"spinner-approachcircle"
		),
		(sfVector2i){
			radius,
			radius
		},
		(sfColor){255, 255, 255, alpha},
		true,
		0
	);
}

void	displaySlider(OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images, unsigned combo, OsuMap_color color, double circleSize)
{
	OsuIntegerVectorArray	*points = &sliderInfos(object->additionalInfos)->curvePoints;

	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){
			points->content[points->length - 1].x + padding.x,
			points->content[points->length - 1].y + padding.y
		},
		Dict_getElement(
			images,
			"sliderendcircle"
		),
		(sfVector2i){
			(54.4f - 4.48f * circleSize) * 2,
			(54.4f - 4.48f * circleSize) * 2
		},
		(sfColor){
			color.red * 0.5,
			color.green * 0.5,
			color.blue * 0.5,
			alpha
		},
		true,
		0
	);
	for (unsigned j = 0; j < points->length; j++) {
		FrameBuffer_drawFilledCircle(
			&frame_buffer,
			(sfVector2i) {
				points->content[j].x - (54.4f - 4.48f * circleSize) + padding.x,
				points->content[j].y - (54.4f - 4.48f * circleSize) + padding.y
			},
			54.4f - 4.48f * circleSize,
			(sfColor){
				color.red * 0.5,
				color.green * 0.5,
				color.blue * 0.5,
				alpha
			}
		);
	}
	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		Dict_getElement(
			images,
			"sliderstartcircle"
		),
		(sfVector2i){
			(54.4f - 4.48f * circleSize) * 2,
			(54.4f - 4.48f * circleSize) * 2
		},
		(sfColor){
			color.red * 0.5,
			color.green * 0.5,
			color.blue * 0.5,
			alpha
		},
		true,
		0
	);
	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		Dict_getElement(
			images,
			"sliderstartcircleoverlay"
		),
		(sfVector2i){
			(54.4f - 4.48f * (float)circleSize) * 2,
			(54.4f - 4.48f * (float)circleSize) * 2
		},
		(sfColor){255, 255, 255, alpha},
		true,
		0
	);
	displayApproachCircle(
		(sfColor){color.red, color.green, color.blue, alpha},
		object,
		circleSize,
		totalTicks,
		images
	);
	displayCombo(
		combo,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		images,
		alpha
	);
}

void	displayHitCircle(OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images, unsigned combo, OsuMap_color color, double circleSize)
{
	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){object->position.x + padding.x, object->position.y + padding.y},
		Dict_getElement(images, "hitcircle"),
		(sfVector2i){(54.4f - 4.48f * circleSize) * 2, (54.4f - 4.48f * circleSize) * 2},
		(sfColor){color.red * 0.5, color.green * 0.5, color.blue * 0.5, alpha},
		true,
		0
	);
	FrameBuffer_drawImage(
		&frame_buffer,
		(sfVector2i){object->position.x + padding.x, object->position.y + padding.y},
		Dict_getElement(images, "hitcircleoverlay"),
		(sfVector2i){(54.4f - 4.48f * circleSize) * 2, (54.4f - 4.48f * circleSize) * 2},
		(sfColor){255, 255, 255, alpha},
		true,
		0
	);
	displayApproachCircle(
		(sfColor){color.red, color.green, color.blue, alpha},
		object,
		circleSize,
		totalTicks,
		images
	);
	displayCombo(
		combo,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		images,
		alpha
	);
}

void	displayHitObjects(unsigned currentComboColor, unsigned currentGameHitObject, unsigned currentTimingPoint, OsuMap *beatmap, unsigned long totalTicks, unsigned beginCombo, Dict *images, Dict *sounds)
{
	unsigned	end;
	unsigned	combo;
	unsigned	color;

	end = getLastObjToDisplay(currentGameHitObject, currentTimingPoint, beatmap, totalTicks);
	for (unsigned i = end - 1; i >= currentGameHitObject && (int)i >= 0; i--) {
		unsigned char	alpha = calcAlpha(beatmap->hitObjects.content[i], totalTicks);

		combo = beginCombo;
		color = currentComboColor;
		for (unsigned j = currentGameHitObject; j < i; j++) {
			if (beatmap->hitObjects.content[j].type & HITOBJ_NEW_COMBO) {
				combo = 0;
				color = (color + 1) % beatmap->colors.length;
			}
			combo++;
		}

		if (beatmap->hitObjects.content[i].type & HITOBJ_SPINNER)
			displaySpinner(&beatmap->hitObjects.content[i], totalTicks, alpha, images);
		else if (beatmap->hitObjects.content[i].type & HITOBJ_SLIDER)
			displaySlider(&beatmap->hitObjects.content[i], totalTicks, alpha, images, combo, beatmap->colors.content[color], beatmap->difficulty.circleSize);
		else
			displayHitCircle(&beatmap->hitObjects.content[i], totalTicks, alpha, images, combo, beatmap->colors.content[color], beatmap->difficulty.circleSize);
	}
}
