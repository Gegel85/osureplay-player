//
// Created by Gegel85 on 27/05/2020.
//

#ifndef OSUREPLAY_PLAYER_SOUNDMANAGER_HPP
#define OSUREPLAY_PLAYER_SOUNDMANAGER_HPP


#include "../Sound.hpp"

namespace OsuReplayPlayer
{
	class SoundManager {
	public:
		virtual void setVolume(float volume) = 0;
		virtual unsigned playSound(const Sound &sound) = 0;
		virtual void stopSound(unsigned id) = 0;
	};
}


#endif //OSUREPLAY_PLAYER_SOUNDMANAGER_HPP
