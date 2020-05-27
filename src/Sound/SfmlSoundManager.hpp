//
// Created by Gegel85 on 27/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SFMLSOUNDMANAGER_HPP
#define OSUREPLAY_PLAYER_SFMLSOUNDMANAGER_HPP


#include "SoundManager.hpp"

namespace OsuReplayPlayer
{
	class SFMLSoundManager : public SoundManager {
	private:
		unsigned _currentSound = 0;
		std::vector<sf::Sound> _sounds{64};

	public:
		void setVolume(float volume) override;
		unsigned int playSound(const Sound &sound) override;
		void stopSound(unsigned id) override;
	};
}


#endif //OSUREPLAY_PLAYER_SFMLSOUNDMANAGER_HPP
