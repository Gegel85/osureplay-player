//
// Created by Gegel85 on 17/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SOUND_HPP
#define OSUREPLAY_PLAYER_SOUND_HPP


#include <SFML/Audio.hpp>
#include <vector>

#define SAMPLE_RATE 44100LU

namespace OsuReplayPlayer
{
	class Sound {
	private:
		int64_t _sampleRate = 0;
		std::vector<std::vector<short>> _data;
		sf::SoundBuffer _buffer;

	public:
		Sound() = default;
		Sound(const std::string &path, int64_t sample_rate = SAMPLE_RATE);
		unsigned int getNbChannels() const;
		int64_t getSampleRate() const;
		std::vector<short> operator[](unsigned int index) const;
		const sf::SoundBuffer &getBuffer() const;
	};
}


#endif //OSUREPLAY_PLAYER_SOUND_HPP
