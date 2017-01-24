#include "LevelEffects.hpp"
#include "Entity.hpp"
#include "LevelManager.hpp"
#include "AxisSighted.hpp"
#include "Player.hpp"
#include "Level.hpp"
#include "Fog.hpp"
#include "game.hpp"
#include "LightSource.hpp"

using lif::LevelEffects;
using lif::TILE_SIZE;
using lif::LEVEL_WIDTH;
using lif::LEVEL_HEIGHT;

LevelEffects::LevelEffects() {
	darknessRenderTex.create(LEVEL_WIDTH * TILE_SIZE, LEVEL_HEIGHT * TILE_SIZE);
}

std::set<lif::Entity*> LevelEffects::getEffectEntities(const lif::Level& level) {
	std::set<lif::Entity*> entities;

	const auto& effects = level.getInfo().effects;
	if (effects.find("fog") != effects.end()) {
		entities.insert(new lif::Fog);
	}

	return entities;
}

void LevelEffects::blendEffects(const lif::LevelManager& lm, sf::RenderTarget& window,
		const sf::Vector2f& origin) const
{
	if (darknessOn) {
		_blendDarkness(lm, window, origin);
	}
}

void LevelEffects::_blendDarkness(const lif::LevelManager& lm, sf::RenderTarget& window,
		const sf::Vector2f& origin) const
{
	darknessRenderTex.clear(sf::Color::Black);
	const auto dy = darknessRenderTex.getSize().y;
	// Calculate visibility rectangles for players
	for (unsigned i = 0; i < lif::MAX_PLAYERS; ++i) {
		const auto player = lm.getPlayer(i + 1);
		if (player == nullptr) continue;
		const auto pos = player->getPosition();
		const auto sighted = player->get<lif::AxisSighted>();
		// distance with nearest opaque entity
		std::array<float, static_cast<std::size_t>(lif::Direction::NONE)> nearest = {
			pos.y - TILE_SIZE, // up
			pos.x - TILE_SIZE, // left
			LEVEL_HEIGHT * TILE_SIZE - nearest[0] - TILE_SIZE, // down
			LEVEL_WIDTH * TILE_SIZE - nearest[1] - TILE_SIZE,  // right
		};
		// calculate vision in all directions
		for (unsigned i = 0; i < nearest.size(); ++i) {
			const float vision = sighted->getVision(static_cast<lif::Direction>(i));
			if (vision >= 0)
				nearest[i] = vision;
		}
		// vertical rectangle
		sf::RectangleShape rect(sf::Vector2f(
					TILE_SIZE,
					TILE_SIZE + nearest[lif::UP] + nearest[lif::DOWN]));
		rect.setPosition(pos.x - TILE_SIZE, dy - pos.y - nearest[lif::DOWN]);
		darknessRenderTex.draw(rect);
		// horizontal rectangle
		rect = sf::RectangleShape(sf::Vector2f(
					TILE_SIZE + nearest[lif::LEFT] + nearest[lif::RIGHT],
					TILE_SIZE));
		rect.setPosition(pos.x - (TILE_SIZE + nearest[lif::LEFT]), dy - pos.y);
		darknessRenderTex.draw(rect);
	}
	
	// Calculate visibility circles for light sources
	lm.getEntities().apply([this, dy, origin] (const lif::Entity *e) {
		const auto source = e->get<lif::LightSource>();
		if (source == nullptr) return;
		sf::CircleShape light(source->getRadius());
		const auto pos = e->getPosition();
		light.setPosition(pos.x - 1.5 * TILE_SIZE, dy - pos.y - 0.5 * TILE_SIZE);
		light.setFillColor(source->getColor());
		darknessRenderTex.draw(light);
	});

	sf::Sprite darkSprite(darknessRenderTex.getTexture());
	darkSprite.setPosition(lif::TILE_SIZE, lif::TILE_SIZE);
	darkSprite.setOrigin(origin);
	window.draw(darkSprite, sf::BlendMultiply);
}
