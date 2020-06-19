//
// Created by Gegel85 on 19/06/2020.
//

#include "OsuParticle.hpp"

namespace OsuReplayPlayer
{
	OsuParticle::OsuParticle(const OsuSkin &skin, unsigned int lifeTime, const std::string &basePath, sf::Vector2f pos, bool loop) :
		_skin(skin),
		_pos(pos),
		_lifeTime(lifeTime),
		_basePath(basePath),
		_loop(loop)
	{
		while (skin.hasImage(basePath + std::to_string(this->_animationSize)))
			this->_animationSize++;
	}

	void OsuParticle::draw(RenderTarget &target)
	{
		if (this->_animationSize) {
			target.drawImage(
				sf::Vector2i(this->_pos.x, this->_pos.y),
				this->_skin.get().getImage(this->_basePath + std::to_string(this->_currentAnimation)),
				{-1, -1},
				{255, 255, 255, this->_alpha},
				true,
				this->_rotation
			);

			if (this->_currentAnimation < this->_animationSize - 1)
				this->_currentAnimation++;
			else if (this->_loop)
				this->_currentAnimation = 0;
		} else
			target.drawImage(
				sf::Vector2i(this->_pos.x, this->_pos.y),
				this->_skin.get().getImage(this->_basePath),
				{-1, -1},
				{255, 255, 255, this->_alpha},
				true,
				this->_rotation
			);
	}

	void OsuParticle::update(float timeElapsed)
	{
		this->_lifeTime -= timeElapsed;
	}

	bool OsuParticle::hasExpired() const
	{
		return this->_lifeTime <= 0;
	}
}
