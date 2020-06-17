//
// Created by Gegel85 on 16/06/2020.
//

#ifndef OSUREPLAY_PLAYER_LIBAVSOUNDMANAGER_HPP
#define OSUREPLAY_PLAYER_LIBAVSOUNDMANAGER_HPP


#include "../libav.hpp"
#include "SoundManager.hpp"

namespace OsuReplayPlayer
{
	struct PlayingSound {
		std::reference_wrapper<const Sound> sound;
		double pitch;
		double pos;
	};

	class LibAvSoundManager : public SoundManager {
	private:
		std::vector<PlayingSound> _sounds;
		AVCodecContext *_codecContext;
		AVPacket *_packet;
		AVFrame *_frame;
		FILE *_stream;
		float _volume = 1;
		int _index = 0;
		unsigned _i = 0;

		void _initCodec();
		void _initFrame();
		void _flush(bool sendFrame);

	public:
		LibAvSoundManager(const std::string &path);
		~LibAvSoundManager();
		void setVolume(float volume) override;
		void tick(unsigned currentFrame, unsigned framePerSeconds) override;
		unsigned int playSound(const Sound &sound, double pitch) override;
		void stopSound(unsigned int id) override;
	};
}


#endif //OSUREPLAY_PLAYER_LIBAVSOUNDMANAGER_HPP
