#include "Fire.hpp"
#include "Animated.hpp"
#include "Temporary.hpp"
#include "Drawable.hpp"
#include "core.hpp"

using lif::Fire;
using lif::TILE_SIZE;

Fire::Fire(const sf::Vector2f& pos, const sf::Vector2f& size, sf::Time duration)
	: lif::Pond(pos, size, 1, { lif::c_layers::ENEMIES, lif::c_layers::ENEMIES_IGNORE_BREAKABLES })
{
	auto animated = addComponent(new lif::Animated(*this, lif::getAsset("graphics", "fire.png")));
	animated->addAnimation("burn", {
		sf::IntRect(0, 0, size.x, size.y),
		sf::IntRect(TILE_SIZE, 0, size.x, size.y)
	}, true);
	animated->getTexture()->setRepeated(true);
	addComponent(new lif::Drawable(*this, *animated));
	if (duration > sf::Time::Zero) {
		sf::Clock clock;
		addComponent(new lif::Temporary(*this, [clock, duration] () {
			return clock.getElapsedTime() > duration;
		}));
	}
}