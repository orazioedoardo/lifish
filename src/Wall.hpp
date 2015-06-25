#pragma once
/**
 * Base virtual class for walls.
 */
#include "FixedEntity.hpp"

namespace Game {

class Wall : public Game::FixedEntity {
public:
	Wall(sf::Vector2f pos, const std::string& texture_name)
		: FixedEntity(pos, texture_name) {}
};

}
