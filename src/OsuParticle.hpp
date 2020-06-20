//
// Created by Gegel85 on 19/06/2020.
//

#ifndef OSUREPLAY_PLAYER_OSUPARTICLE_HPP
#define OSUREPLAY_PLAYER_OSUPARTICLE_HPP


#include <string>
#include <SFML/Graphics.hpp>
#include "Rendering/RenderTarget.hpp"
#include "OsuSkin.hpp"

namespace OsuReplayPlayer
{
	class OsuParticle {
	private:
		std::reference_wrapper<const OsuSkin> _skin;
		float _rotation = 0;
		sf::Vector2f _pos;
		float _lifeTime;
		unsigned _fadeTime;
		std::string _basePath;
		unsigned _animationSize = 0;
		unsigned _currentAnimation = 0;
		unsigned char _alpha = 255;
		bool _loop;

	public:
		OsuParticle(const OsuSkin &skin, unsigned lifeTime, unsigned fadeTime, const std::string &basePath, sf::Vector2f pos, bool loop = false);

		void draw(RenderTarget &target);
		bool hasExpired() const;
		virtual void update(float timeElapsed);
	};
}


#endif //OSUREPLAY_PLAYER_OSUPARTICLE_HPP
