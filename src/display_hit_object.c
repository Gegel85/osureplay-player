//
// Created by Gegel85 on 02/02/2019.
//

#include <stdbool.h>
#include <osu_map_parser.h>
#include <SFML/Graphics.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "globals.h"
#include "defines.h"
#include "utils.h"
#include "dict.h"
#include "frame_buffer.h"
#include "replay_player.h"

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

void	displayApproachCircle(FrameBuffer *frame_buffer, sfColor color, OsuMap_hitObject *object, double circleSize, unsigned long ticks, Dict *images)
{
	if (object->timeToAppear < ticks)
		return;
	FrameBuffer_drawImage(
		frame_buffer,
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

void	displayCombo(FrameBuffer *frame_buffer, unsigned combo, sfVector2i pos, Dict *images, unsigned char alpha)
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
			frame_buffer,
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

void	displaySpinner(FrameBuffer *frame_buffer, OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images)
{
	long	duration = *(long *)object->additionalInfos - object->timeToAppear;
	long	remaining = *(long *)object->additionalInfos - totalTicks;
	float	radius = object->timeToAppear >= totalTicks ? 400 : 4 * (100 - (duration - remaining) * 100.f / duration);

	FrameBuffer_drawImage(
		frame_buffer,
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

void	displaySlider(FrameBuffer *frame_buffer, OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images, unsigned combo, OsuMap_color color, double circleSize, OsuMap_timingPointEvent *timeingpt, OsuMap *beatmap)
{
	OsuIntegerVectorArray	*points = &sliderInfos(object->additionalInfos)->curvePoints;

	//Display the body of the slider
	for (unsigned j = 0; j < points->length; j++) {
		FrameBuffer_drawFilledCircle(
			frame_buffer,
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

	//Display the end of the slider
	FrameBuffer_drawImage(
		frame_buffer,
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

	//Display the start od the slider
	FrameBuffer_drawImage(
		frame_buffer,
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
		frame_buffer,
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

	//Display the follow circle if needed
	if (object->timeToAppear < totalTicks) {
		double	ptId = (
			points->length * fmod(
				totalTicks - object->timeToAppear,
				sliderInfos(
					object->additionalInfos
				)->pixelLength / (
					100 * beatmap->difficulty.sliderMultiplayer
				) * timeingpt->millisecondsPerBeat
			) / (
				sliderInfos(
					object->additionalInfos
				)->pixelLength / (
					100 * beatmap->difficulty.sliderMultiplayer
				) * timeingpt->millisecondsPerBeat
			)
		);
		sfVector2i	currentPoint =  {
			points->content[(int)ptId].x + padding.x,
			points->content[(int)ptId].y + padding.y
		};

		FrameBuffer_drawImage(
			frame_buffer,
			currentPoint,
			Dict_getElement(
				images,
				"sliderb0"
			),
			(sfVector2i) {
				(54.4f - 4.48f * (float) circleSize) * 2,
				(54.4f - 4.48f * (float) circleSize) * 2
			},
			(sfColor) {255, 255, 255, 255},
			true,
			0
		);

		FrameBuffer_drawImage(
			frame_buffer,
			currentPoint,
			Dict_getElement(
				images,
				"sliderfollowcircle"
			),
			(sfVector2i) {
				(54.4f - 4.48f * (float) circleSize) * 4,
				(54.4f - 4.48f * (float) circleSize) * 4
			},
			(sfColor) {255, 255, 255, 255},
			true,
			0
		);
	//Else, display the approach circle
	} else
		displayApproachCircle(
			frame_buffer,
			(sfColor){color.red, color.green, color.blue, alpha},
			object,
			circleSize,
			totalTicks,
			images
		);
	//Display the combo number on top of the start circle
	displayCombo(
		frame_buffer,
		combo,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		images,
		alpha
	);
}

void	displayHitCircle(FrameBuffer *frame_buffer, OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images, unsigned combo, OsuMap_color color, double circleSize)
{
	FrameBuffer_drawImage(
		frame_buffer,
		(sfVector2i){object->position.x + padding.x, object->position.y + padding.y},
		Dict_getElement(images, "hitcircle"),
		(sfVector2i){(54.4f - 4.48f * circleSize) * 2, (54.4f - 4.48f * circleSize) * 2},
		(sfColor){color.red * 0.5, color.green * 0.5, color.blue * 0.5, alpha},
		true,
		0
	);
	FrameBuffer_drawImage(
		frame_buffer,
		(sfVector2i){object->position.x + padding.x, object->position.y + padding.y},
		Dict_getElement(images, "hitcircleoverlay"),
		(sfVector2i){(54.4f - 4.48f * circleSize) * 2, (54.4f - 4.48f * circleSize) * 2},
		(sfColor){255, 255, 255, alpha},
		true,
		0
	);
	displayApproachCircle(
		frame_buffer,
		(sfColor){color.red, color.green, color.blue, alpha},
		object,
		circleSize,
		totalTicks,
		images
	);
	displayCombo(
		frame_buffer,
		combo,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		images,
		alpha
	);
}

void	displayHitObjects(ReplayPlayerState *state, OsuMap *beatmap)
{
	unsigned	end;
	unsigned	combo;
	unsigned	color;

	end = getLastObjToDisplay(state->currentGameHitObject, state->currentTimingPoint, beatmap, state->totalTicks);
	for (unsigned i = end - 1; i >= state->currentGameHitObject && (int)i >= 0; i--) {
		unsigned char	alpha = calcAlpha(beatmap->hitObjects.content[i], state->totalTicks);

		combo = state->beginCombo;
		color = state->currentComboColor;
		for (unsigned j = state->currentGameHitObject; j < i; j++) {
			if (beatmap->hitObjects.content[j].type & HITOBJ_NEW_COMBO) {
				combo = 0;
				color = (color + 1) % beatmap->colors.length;
			}
			combo++;
		}

		if (beatmap->hitObjects.content[i].type & HITOBJ_SPINNER)
			displaySpinner(
				&state->frameBuffer,
				&beatmap->hitObjects.content[i],
				state->totalTicks,
				alpha,
				state->images
			);
		else if (beatmap->hitObjects.content[i].type & HITOBJ_SLIDER)
			displaySlider(
				&state->frameBuffer,
				&beatmap->hitObjects.content[i],
				state->totalTicks,
				alpha,
				state->images,
				combo,
				beatmap->colors.content[color],
				beatmap->difficulty.circleSize,
				&beatmap->timingPoints.content[state->currentTimingPoint],
				beatmap
			);
		else
			displayHitCircle(
				&state->frameBuffer,
				&beatmap->hitObjects.content[i],
				state->totalTicks,
				alpha,
				state->images,
				combo,
				beatmap->colors.content[color],
				beatmap->difficulty.circleSize
			);
	}
}
