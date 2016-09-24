#include "collision_layers.hpp"

#define COLLIDES(a, b) \
	collide[a][b] = true; \
	collide[b][a] = true;

#define SOLID(a, b) \
	solid[a][b] = true; \
	solid[b][a] = true;

using L = Game::Layers::Layer;

Game::Matrix<bool, L::N_LAYERS, L::N_LAYERS> Game::Layers::collide,
	                                     Game::Layers::solid;

void Game::Layers::init() {
	for (auto& l : collide)
		l.fill(false);
	for (auto& l : solid)
		l.fill(false);

	COLLIDES(L::PLAYERS, L::PLAYERS)
	COLLIDES(L::PLAYERS, L::ENEMIES)
	COLLIDES(L::PLAYERS, L::ENEMY_BULLETS)
	COLLIDES(L::PLAYERS, L::BOSSES)
	COLLIDES(L::PLAYERS, L::BOSS_BULLETS)
	COLLIDES(L::PLAYERS, L::WALLS)
	COLLIDES(L::PLAYERS, L::TRANSP_WALLS)
	COLLIDES(L::PLAYERS, L::TELEPORTS)
	COLLIDES(L::PLAYERS, L::DEFAULT) 
	COLLIDES(L::PLAYERS, L::GRABBABLE) 
	COLLIDES(L::ENEMIES, L::ENEMIES)
	COLLIDES(L::ENEMIES, L::ENEMY_BULLETS)
	COLLIDES(L::ENEMIES, L::BOSS_BULLETS)
	COLLIDES(L::ENEMIES, L::BOSSES)
	COLLIDES(L::ENEMIES, L::WALLS)
	COLLIDES(L::ENEMIES, L::TRANSP_WALLS)
	COLLIDES(L::ENEMIES, L::TELEPORTS)
	COLLIDES(L::BOSSES, L::ENEMY_BULLETS)
	//COLLIDES(L::BOSSES, L::BOSS_BULLETS)
	COLLIDES(L::ENEMY_BULLETS, L::WALLS)
	//COLLIDES(L::BOSS_BULLETS, L::WALLS)
	COLLIDES(L::EXPLOSIONS, L::PLAYERS)
	COLLIDES(L::EXPLOSIONS, L::ENEMIES)
	COLLIDES(L::EXPLOSIONS, L::BOSSES)
	COLLIDES(L::EXPLOSIONS, L::WALLS)
	COLLIDES(L::EXPLOSIONS, L::TRANSP_WALLS)
	COLLIDES(L::EXPLOSIONS, L::BOMBS)

	SOLID(L::PLAYERS, L::PLAYERS)
	SOLID(L::PLAYERS, L::WALLS)
	SOLID(L::PLAYERS, L::TRANSP_WALLS)
	SOLID(L::ENEMIES, L::ENEMIES)
	SOLID(L::ENEMIES, L::BOSSES)
	SOLID(L::ENEMIES, L::WALLS)
	SOLID(L::ENEMIES, L::TRANSP_WALLS)
	SOLID(L::BOSSES, L::ENEMY_BULLETS)
	SOLID(L::ENEMY_BULLETS, L::WALLS)
	SOLID(L::EXPLOSIONS, L::WALLS)
	SOLID(L::EXPLOSIONS, L::TRANSP_WALLS)
}
