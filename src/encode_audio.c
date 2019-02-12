#include <libavcodec/avcodec.h>
#include <libavutil/common.h>
#include <libavutil/channel_layout.h>
#include <frame_buffer.h>

void	encodeAudioFrame(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, FILE *output)
{
	int ret;

	/* send the frame for encoding */
	ret = avcodec_send_frame(ctx, frame);
	if (ret < 0)
		display_error("error sending the frame to the encoder\n");

	/* read all the available output packets (in general there may be any
	 * number of them */
	while (ret >= 0) {
		ret = avcodec_receive_packet(ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0)
			display_error("error encoding audio frame\n");
		fwrite(pkt->data, 1, pkt->size, output);
		av_packet_unref(pkt);
	}
}
