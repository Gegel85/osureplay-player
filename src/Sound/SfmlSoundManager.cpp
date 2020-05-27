//
// Created by Gegel85 on 27/05/2020.
//

#include "SfmlSoundManager.hpp"

namespace OsuReplayPlayer
{
	void SFMLSoundManager::setVolume(float volume)
	{
		for (auto &sound : this->_sounds)
			sound.setVolume(volume);
	}

	unsigned int SFMLSoundManager::playSound(const Sound &sound)
	{
		while (this->_sounds[this->_currentSound].getStatus() == sf::Sound::Playing) {
			this->_currentSound++;
			this->_currentSound %= this->_sounds.size();
		}

		auto &s = this->_sounds[this->_currentSound];

		s.setBuffer(sound.getBuffer());
		s.play();
		return this->_currentSound;
	}

	void SFMLSoundManager::stopSound(unsigned id)
	{
		this->_sounds[id].stop();
	}
}
