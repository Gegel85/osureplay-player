//
// Created by Gegel85 on 02/02/2019.
//

#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>

#include "frame_buffer.h"

void	encodeFrame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, FILE *outfile)
{
	int ret;

	/* send the frame to the encoder */
	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0)
		display_error("error sending a frame for encoding\n");

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0)
			display_error("error during encoding\n");
		if (fwrite(pkt->data, 1, pkt->size, outfile) != pkt->size)
			display_error("Cannot write in file\n");
		av_packet_unref(pkt);
	}
}