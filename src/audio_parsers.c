//
// Created by Gegel85 on 24/03/2019.
//

#include <stdbool.h>
#include <stdio.h>
#include "frame_buffer.h"
#include "sound.h"
#include "wav_parser.h"

bool (* const audioChecker[])(FILE *stream) = {
	checkWavFile
};

Sound *(* const audioParsers[])(FILE *stream) = {
	parseWavFile
};
