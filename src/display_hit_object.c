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

unsigned	getLastObjToDisplay(unsigned currentGameHitObject, OsuMap *beatmap, unsigned long totalTicks)
{
	unsigned	end = currentGameHitObject;

	while (true) {
		if (end >= beatmap->hitObjects.length)
			break;
		if (beatmap->hitObjects.content[end].timeToAppear >= 800 && beatmap->hitObjects.content[end].timeToAppear - 800 > totalTicks)
			break;
		end++;
	}
	return end;
}

void	displayApproachCircle(FrameBuffer *frameBuffer, sfColor color, OsuMap_hitObject *object, double circleSize, unsigned long ticks, Dict *images)
{
	if (object->timeToAppear < ticks)
		return;
	FrameBuffer_drawImage(
		frameBuffer,
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

void	displaySpinner(FrameBuffer *frameBuffer, OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images)
{
	long	duration = *(long *)object->additionalInfos - object->timeToAppear;
	long	remaining = *(long *)object->additionalInfos - totalTicks;
	float	radius = object->timeToAppear >= totalTicks ? 400 : 4 * (100 - (duration - remaining) * 100.f / duration);

	FrameBuffer_drawImage(
		frameBuffer,
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

void	displaySlider(FrameBuffer *frameBuffer, OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images, unsigned combo, OsuMap_color color, double circleSize, OsuMap_timingPointEvent *timeingpt, OsuMap *beatmap)
{
	OsuIntegerVectorArray	*points = &sliderInfos(object->additionalInfos)->curvePoints;
	size_t length = points->length;

	if (totalTicks < object->timeToAppear - 400)
		length *= (400 - (object->timeToAppear - totalTicks - 400)) / 400.;

	//Display the body of the slider
	for (unsigned j = 0; j < length; j += 1) {
		FrameBuffer_drawFilledCircle(
			frameBuffer,
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
		frameBuffer,
		(sfVector2i){
			points->content[length - 1].x + padding.x,
			points->content[length - 1].y + padding.y
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

	//Display the start of the slider
	FrameBuffer_drawImage(
		frameBuffer,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		Dict_getElement(images, "sliderstartcircle") ?: Dict_getElement(images, "hitcircle"),
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
		frameBuffer,
		(sfVector2i){
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		Dict_getElement(images, "sliderstartcircleoverlay") ?: Dict_getElement(images, "hitcircleoverlay"),
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
			frameBuffer,
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
			frameBuffer,
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
			frameBuffer,
			(sfColor){color.red, color.green, color.blue, alpha},
			object,
			circleSize,
			totalTicks,
			images
		);
	//Display the combo number on top of the start circle
	displayNumber(
		frameBuffer,
		combo,
		(sfVector2i) {
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		images,
		alpha,
		15,
		"default"
	);
}

void	displayHitCircle(FrameBuffer *frameBuffer, OsuMap_hitObject *object, unsigned long totalTicks, unsigned char alpha, Dict *images, unsigned combo, OsuMap_color color, double circleSize)
{
	FrameBuffer_drawImage(
		frameBuffer,
		(sfVector2i){object->position.x + padding.x, object->position.y + padding.y},
		Dict_getElement(images, "hitcircle"),
		(sfVector2i){(54.4f - 4.48f * circleSize) * 2, (54.4f - 4.48f * circleSize) * 2},
		(sfColor){color.red * 0.5, color.green * 0.5, color.blue * 0.5, alpha},
		true,
		0
	);
	FrameBuffer_drawImage(
		frameBuffer,
		(sfVector2i){object->position.x + padding.x, object->position.y + padding.y},
		Dict_getElement(images, "hitcircleoverlay"),
		(sfVector2i){(54.4f - 4.48f * circleSize) * 2, (54.4f - 4.48f * circleSize) * 2},
		(sfColor){255, 255, 255, alpha},
		true,
		0
	);
	displayApproachCircle(
		frameBuffer,
		(sfColor){color.red, color.green, color.blue, alpha},
		object,
		circleSize,
		totalTicks,
		images
	);
	displayNumber(
		frameBuffer,
		combo,
		(sfVector2i) {
			object->position.x + padding.x,
			object->position.y + padding.y
		},
		images,
		alpha,
		15,
		"default"
	);
}

void	displayHitObjects(ReplayPlayerState *state, OsuMap *beatmap)
{
	unsigned	end;
	unsigned	combo;
	unsigned	color;

	end = getLastObjToDisplay(state->currentGameHitObject, beatmap, state->totalTicks);
	for (int i = end - 1; (unsigned)i >= state->currentGameHitObject && i >= 0; i--) {
		unsigned char	alpha = calcAlpha(beatmap->hitObjects.content[i], state->totalTicks);

		combo = state->beginCombo;
		color = state->currentComboColor;
		for (int j = state->currentGameHitObject; j < i; j++) {
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
