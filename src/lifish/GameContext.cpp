#include "GameContext.hpp"
#include "Killable.hpp"
#include "Music.hpp"
#include "BaseEventHandler.hpp"
#include "Options.hpp"
#include "core.hpp"
#include "contexts.hpp"
#include "MusicManager.hpp"
#include "Bonusable.hpp"
#include "bonus_type.hpp"
#include "Player.hpp"
#include "Controllable.hpp"
#ifndef RELEASE
#	include <iostream>
#	include <iomanip>
#	include "DebugRenderer.hpp"
#	include "DebugEventHandler.hpp"
#endif

using Game::GameContext;

GameContext::GameContext(sf::Window& window, const std::string& levelsetName, unsigned short startLv)
	: Game::WindowContext()
	, lm()
	, sidePanel(lm)
	, wlHandler(lm, sidePanel)
{
	handlers.push_back(std::unique_ptr<Game::EventHandler>(new Game::BaseEventHandler));
#ifndef RELEASE
	handlers.push_back(std::unique_ptr<Game::EventHandler>(new Game::Debug::DebugEventHandler(*this)));
#endif

	int lvnum = startLv;
	ls.loadFromFile(levelsetName);
	if (lvnum > ls.getLevelsNum())
		lvnum %= ls.getLevelsNum();
	level = std::unique_ptr<Game::Level>(ls.getLevel(lvnum));

	// Create the players
	players = lm.createNewPlayers();
	for (auto p : players)
		p->get<Game::Controllable>()->setWindow(window);

	lm.setLevel(*level);

	// Setup the music
	Game::musicManager->set(level->get<Game::Music>()->getMusic()); 

	// Ensure lm is not paused
	lm.resume();
}

void GameContext::setActive(bool b) {
	Game::Activable::setActive(b);
	if (active)
		lm.resume();
}

void GameContext::update() {
	// Handle win / loss cases
	wlHandler.handleWinLose();
	if (wlHandler.getState() == Game::WinLoseHandler::State::ADVANCING_LEVEL) {
		newContext = Game::CTX_WINLOSE;
		return;
	} else if (wlHandler.getState() == Game::WinLoseHandler::State::ADVANCED_LEVEL) {
		_advanceLevel();	
	}
/*
		// Give bonus points/handle continues/etc
		wlHandler.advanceLevel(window, sidePanel);
		if (wlHandler.getState() == WinLoseHandler::State::GAME_WON) {
			// TODO
		}
		
		for (unsigned short i = 0; i < Game::MAX_PLAYERS; ++i)
			players[i] = lm.getPlayer(i + 1);
		level = ls.getLevel(level->getInfo().levelnum + 1);
		lm.setLevel(*level);
		Game::musicManager->set(level->get<Game::Music>()->getMusic())
			.setVolume(Game::options.musicVolume)
			.play();
		continue;
	}
*/
	// Update level
	if (!lm.isPaused())
		lm.update();

#	ifndef RELEASE
	if (cycle++ % 50 == 0 && (debug >> DBG_PRINT_CD_STATS) == 1)
		_printCDStats();
#	endif

	sidePanel.update();
}

bool GameContext::handleEvent(sf::Window&, sf::Event event) {
	auto pause_game = [this] () {
		lm.pause();
		newContext = Game::CTX_UI;
	};
	switch (event.type) {
	case sf::Event::JoystickButtonPressed:
		{
			const auto btn = event.joystickButton;
			const short pb = JoystickUtils::getPauseButton(btn.joystickId);
			if (pb >= 0 && btn.button == static_cast<unsigned int>(pb))
				pause_game();
			return true;
		}
	case sf::Event::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::P:
			pause_game();
			return true;
		case sf::Keyboard::Escape:
			for (auto player : players) {
				player->setRemainingLives(0);
				player->get<Game::Killable>()->kill();
			}
			return true;
		default: 
			break;
		}
	default: 
		break;
	}
	return false;
}

void GameContext::setOrigin(const sf::Vector2f& o) {
	Game::WindowContext::setOrigin(o);
	lm.setOrigin(o);

}

void GameContext::draw(sf::RenderTarget& window, sf::RenderStates states) const {
	window.draw(lm, states);
	window.draw(sidePanel, states);
#ifndef RELEASE
	if ((debug >> DBG_DRAW_COLLIDERS) & 1)
		Debug::DebugRenderer::drawColliders(window, lm.getEntities());
	if ((debug >> DBG_DRAW_SH_CELLS) & 1)
		Debug::DebugRenderer::drawSHCells(window,
				static_cast<const Game::SHCollisionDetector&>(
					lm.getCollisionDetector()));
#endif
}

#ifndef RELEASE
void GameContext::toggleDebug(unsigned int flag) {
	debug ^= 1 << flag;
}

void GameContext::_printCDStats() const {
	const auto& dbgStats = lm.getCollisionDetector().getStats();
	std::cerr << std::setfill(' ') << std::scientific << std::setprecision(4)
		<< "#checked: " << std::setw(5) << dbgStats.counter.safeGet("checked")
		<< " | tot: " << std::setw(8) << dbgStats.timer.safeGet("tot")
		<< " | tot_narrow: " << std::setw(8) << dbgStats.timer.safeGet("tot_narrow")
		<< " | setup: " << std::setw(8) << dbgStats.timer.safeGet("setup") 
		<< " | average: " << std::setw(8) 
			<< dbgStats.timer.safeGet("tot_narrow")/dbgStats.counter.safeGet("checked")
		<< std::resetiosflags(std::ios::showbase) << std::endl;
}
#endif

void GameContext::_advanceLevel() {
	short lvnum = level->getInfo().levelnum;
	const auto& ls = level->getLevelSet();

	if (lvnum == ls.getLevelsNum()) {
		// TODO game won
		return;
	}

	// Resurrect any dead player which has a 'continue' left and
	// remove shield and speedy effects
	for (unsigned short i = 0; i < Game::MAX_PLAYERS; ++i) {
		auto player = lm.getPlayer(i + 1);
		if ((player == nullptr || player->get<Game::Killable>()->isKilled())
				&& Game::playerContinues[i] > 0) 
		{
			//if (_displayContinue(window, panel, i + 1)) {
				--Game::playerContinues[i];
				auto player = std::make_shared<Player>(sf::Vector2f(0, 0), i + 1);
				//player->get<Game::Controllable>()->setWindow(window); // TODO
				lm.setPlayer(i + 1, player);
			//} else {
				//Game::playerContinues[i] = 0;
				//lm.removePlayer(i + 1);
			//}
		} else if (player != nullptr) {
			auto bns = player->get<Game::Bonusable>();
			bns->giveBonus(Game::BonusType::SPEEDY, sf::Time::Zero);
			bns->giveBonus(Game::BonusType::SHIELD, sf::Time::Zero);
		}
	}

	level = ls.getLevel(lvnum + 1);
	lm.setLevel(*level);
	Game::musicManager->set(level->get<Game::Music>()->getMusic())
		.setVolume(Game::options.musicVolume)
		.play();
}