#include "AxisSighted.hpp"
#include "utils.hpp"
#include "Collider.hpp"
#include "EntityGroup.hpp"
#include <array>

using lif::AxisSighted;

// Helper functions for _fillLine
static bool check_up(const sf::Vector2i& etile, const sf::Vector2i& mtile) {
	return etile.x == mtile.x && etile.y < mtile.y;
}

static bool check_down(const sf::Vector2i& etile, const sf::Vector2i& mtile) {
	return etile.x == mtile.x && etile.y > mtile.y;
}

static bool check_left(const sf::Vector2i& etile, const sf::Vector2i& mtile) {
	return etile.y == mtile.y && etile.x < mtile.x;
}

static bool check_right(const sf::Vector2i& etile, const sf::Vector2i& mtile) {
	return etile.y == mtile.y && etile.x > mtile.x;
}

static const std::array<std::function<bool(const sf::Vector2i&, const sf::Vector2i&)>, 4> helper_funcs = {{
	check_up,
	check_left,
	check_down,
	check_right
}};
// end helper functions


AxisSighted::AxisSighted(lif::Entity& owner, float visionRadius)
	: lif::Sighted(owner, visionRadius)
{
	_declComponent<AxisSighted>();
	vision.fill(visionRadius * lif::TILE_SIZE);
}

void AxisSighted::update() {
	lif::Component::update();
	if (entities == nullptr) return;

	vision.fill(visionRadius * lif::TILE_SIZE);
	for (unsigned i = 0; i < 4; ++i) {
		_fillLine(static_cast<lif::Direction>(i));	
	}
}

void AxisSighted::_fillLine(lif::Direction dir) {
	// no check for lm != nullptr as it's done beforehand by update()

	const auto mtile = lif::tile2(owner.getPosition());
	const auto same_line = helper_funcs[static_cast<std::size_t>(dir)];

	seen[dir].clear();

	entities->apply([=] (const lif::Entity *ptr) {
		if (ptr == &owner)
			return;
		const auto etile = lif::tile2(ptr->getPosition());
		if (!same_line(etile, mtile)) return;
		const auto dist = lif::manhattanDistance(etile, mtile);
		if (visionRadius < 0 || dist <= visionRadius) {
			// Only see living entities (including those who are killed but whose kill is in progress)
			const auto killable = ptr->get<lif::Killable>();
			if (killable == nullptr || !killable->isKilled() || killable->isKillInProgress())
				seen[dir].emplace_back(const_cast<lif::Entity*>(ptr), dist);
		}
	});

	std::sort(seen[dir].begin(), seen[dir].end(), [] (const SeenPair& a, const SeenPair& b) {
		return a.second < b.second;
	});

	if (opaqueMask != 0) {
		/* Discard entities farther than an opaque one.
		 * NOTE THAT at the moment only the first collider of the entity is used
		 * to determine opaqueness; this assumes that we only see entities whose
		 * first collider determines their bounding box.
		 */
		for (auto it = seen[dir].begin(); it != seen[dir].end(); ++it) {
			const auto cld = it->first->get<lif::Collider>();
			if (cld != nullptr) {
				const auto layer = cld->getLayer();
				if (_isOpaque(layer)) {
					switch (dir) {
					case lif::Direction::DOWN:
					case lif::Direction::UP:
						vision[dir] = lif::abs(owner.getPosition().y - cld->getPosition().y);
						break;
					case lif::Direction::LEFT:
					case lif::Direction::RIGHT:
						vision[dir] = lif::abs(owner.getPosition().x - cld->getPosition().x);
						break;
					default:
						break;
					}
					seen[dir].erase(it, seen[dir].end());
					break;
				}
			}
		}
	}
}
