//
// Created by Gegel85 on 02/02/2019.
//

#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>

#include "frame_buffer.h"

void	encodeVideoFrame(AVFormatContext *fmtContext, AVStream *stream, AVFrame *frame)
{
	int ret;
	AVPacket *pkt = av_packet_alloc();

	if (!pkt)
		display_error("Cannot alloc packet");
	/* send the frame to the encoder */
	ret = avcodec_send_frame(stream->codec, frame);
	if (ret < 0)
		display_error("Error sending a frame for encoding\n");

	while (ret >= 0) {
		ret = avcodec_receive_packet(stream->codec, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0)
			display_error("Error during encoding\n");
		if (av_interleaved_write_frame(fmtContext, pkt))
			display_error("Cannot write in file\n");
	}
	av_packet_free(&pkt);
}