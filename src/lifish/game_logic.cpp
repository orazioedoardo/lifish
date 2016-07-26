#include "game_logic.hpp"
#include "Bomb.hpp"
#include "Explosion.hpp"
#include "Controllable.hpp"
#include "Scored.hpp"
#include "AI.hpp"
#include "Points.hpp"
#include "Player.hpp"
#include "Bonus.hpp"
#include "BreakableWall.hpp"

using EntityList = std::list<Game::Entity*>;

void Game::Logic::bombExplosionLogic(Game::Entity *e, Game::LevelManager& lm,
		EntityList& tbspawned, EntityList& tbkilled)
{
	auto bomb = dynamic_cast<Game::Bomb*>(e);
	if (bomb == nullptr) return;

	if (bomb->isFuseOver()) {
		auto killable = bomb->get<Game::Killable>();
		if (killable->isKilled()) return;
		killable->kill();
		tbspawned.push_back((new Game::Explosion(bomb->getPosition(),
				bomb->getRadius(), &bomb->getSourcePlayer()))->propagate(lm));
		lm.rmBomb(bomb);
		tbkilled.push_back(bomb);
	}
}

void Game::Logic::bombDeployLogic(Game::Entity *e, Game::LevelManager& lm,
		EntityList& tbspawned, EntityList&)
{
	if (!lm.isPlayer(e)) return;
	auto player = static_cast<Game::Player*>(e);

	const auto pinfo = player->getInfo();
	if (player->get<Game::Controllable>()->hasFocus() 
		&& player->isAligned() 
		&& sf::Keyboard::isKeyPressed(
			Game::Controls::players[player->getInfo().id-1][Game::Controls::CTRL_BOMB])
		&& lm.bombsDeployedBy(pinfo.id) < pinfo.powers.maxBombs
		&& !lm.isBombAt(Game::tile(player->getPosition())))
	{
		tbspawned.push_back(new Game::Bomb(Game::aligned(player->getPosition()), 
					*player, pinfo.powers.bombFuseTime, pinfo.powers.bombRadius));
	}
}

void Game::Logic::bonusDropLogic(Game::Entity *e, Game::LevelManager&,
		EntityList& tbspawned, EntityList&)
{
	auto wall = dynamic_cast<Game::BreakableWall*>(e);
	if (wall == nullptr) return;

	auto klb = wall->get<Game::Killable>();
	if (klb->isKilled() && !klb->isKillInProgress()) {
		const auto bonus_type = Game::bonusTypeDistribution(Game::rng);
		if (bonus_type < Game::Bonus::N_BONUS_TYPES) {
			tbspawned.push_back(new Game::Bonus(e->getPosition(),
						static_cast<Game::Bonus::Type>(bonus_type)));
		}
	}
}

void Game::Logic::scoredKillablesLogic(Game::Entity *e, Game::LevelManager&,
		EntityList& tbspawned, EntityList&)
{
	auto scored = e->get<Game::Scored>();
	if (scored == nullptr || scored->hasGivenPoints()) return;
	
	auto klb = e->get<Game::Killable>();
	if (klb != nullptr && klb->isKilled()) {
		// Give and spawn points
		auto target = scored->getTarget();
		if (target < 0) 
			std::cerr << "[ WARNING ] target of scored hasn't been set" << std::endl;
		else {
			Game::score[target - 1] += scored->givePoints();
			tbspawned.push_back(new Game::Points(e->getPosition(), scored->getPointsGiven()));
		}
	}
}

//void Game::Logic::explosionDamageLogic(Game::Entity *e, Game::LevelManager &lm,
		//EntityList& tbspawned, EntityList& tbkilled)
//{
	//auto expl = dynamic_cast<Game::Explosion*>(e);
	//if (expl == nullptr) return;

	//const auto& tiles = expl->getInvolvedTiles();
	//for (const auto& tile : tiles) {
		//// Check if someone is at that tile
		
	//}
//}

std::vector<Game::Logic::GameLogicFunc> Game::Logic::functions = {
	bombDeployLogic,
	bombExplosionLogic,
	bonusDropLogic,
	scoredKillablesLogic
};