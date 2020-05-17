//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_LIBAV_HPP
#define OSUREPLAY_PLAYER_LIBAV_HPP

#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

std::string getAvErrorCode(int num);

#endif //OSUREPLAY_PLAYER_LIBAV_HPP
