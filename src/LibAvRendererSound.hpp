//
// Created by Gegel85 on 21/05/2020.
//

#ifndef OSUREPLAY_PLAYER_LIBAVRENDERERSOUND_HPP
#define OSUREPLAY_PLAYER_LIBAVRENDERERSOUND_HPP

#include <map>
#include "libav.hpp"
#include "Rendering/RenderTarget.hpp"
#include "Sound/SoundManager.hpp"

namespace OsuReplayPlayer
{
	struct PlayingSound {
		std::reference_wrapper<const Sound> sound;
		double pitch;
		double pos;
	};

	struct OutputStream {
		AVCodecContext *enc;
		AVStream *stream;
		AVPacket *packet;
		AVFrame *frame;
		AVFrame *tmpFrame;
		unsigned nextPts = 0;

		unsigned sampleCount;

		struct SwsContext *swsCtx;
		struct SwrContext *swrCtx;
	};

	struct VideoConfig {
		const std::map<std::string, std::string> &opts;
		sf::Vector2u resolution;
		size_t bitRate;
		unsigned frameRate;
	};

	struct AudioConfig {
		const std::map<std::string, std::string> &opts;
		size_t bitRate;
		size_t sampleRate;
	};

	class LibAvRendererSound : public RenderTarget, public SoundManager {
	private:
		AVFormatContext *_fmtContext;

		OutputStream _videoStream;
		sf::Vector2u _size;
		sf::Color **_pixelArray;
		sf::Color *_buffer;
		sf::Vector2i _padding;

		OutputStream _audioStream;
		std::vector<PlayingSound> _sounds;
		float _volume = 1;
		int _index = 0;
		unsigned _i = 0;

		void _initVideoPart(const VideoConfig &vidConf);
		void _initVideoStream(sf::Vector2u size, unsigned fps, size_t bitRate, const std::map<std::string, std::string> &opts = {});
		void _initVideoFrame();
		void _prepareVideoFrame();
		void _flushVideo(bool sendFrame);

		void _initAudioPart(const AudioConfig &audioConf);
		void _initAudioStream(size_t bitRate, size_t sampleRate, const std::map<std::string, std::string> &opts = {});
		void _initAudioFrame(size_t sampleRate);
		void _flushAudio(bool sendFrame);

	public:
		LibAvRendererSound(const std::string &path, const VideoConfig &vidConf, const AudioConfig &audioConf);
		~LibAvRendererSound() override;
		sf::Vector2u getSize() const override;
		void clear(sf::Color color) override;
		void clear(sf::Vector2i pos, const sf::Image &image, sf::Vector2f newSize, unsigned char dimPercent) override;
		void drawPixel(sf::Vector2i pos, sf::Color color) override;
		void drawPoint(sf::Vector2f pos, sf::Color color) override;
		void drawRectangle(sf::Vector2i pos, sf::Vector2u size, unsigned thickness, sf::Color color) override;
		void drawFilledRectangle(sf::Vector2i pos, sf::Vector2u size, sf::Color color) override;
		void drawImage(sf::Vector2i pos, const sf::Image &image, sf::Vector2f newSize, sf::Color tint, bool centered, float rotation) override;
		void drawCircle(unsigned thickness, sf::Vector2i pos, float radius, sf::Color color) override;
		void drawFilledCircle(sf::Vector2i pos, float radius, sf::Color color) override;
		void renderFrame() override;
		void setGlobalPadding(sf::Vector2i padding) override;
		sf::Vector2i getGlobalPadding() override;

		void setVolume(float volume) override;
		void tick(unsigned currentFrame, unsigned framePerSeconds) override;
		unsigned int playSound(const Sound &sound, double pitch) override;
		void stopSound(unsigned int id) override;
	};
}


#endif //OSUREPLAY_PLAYER_LIBAVRENDERERSOUND_HPP
