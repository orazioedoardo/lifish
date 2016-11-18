#include "BaseEventHandler.hpp"
#include "core.hpp"
#include "Options.hpp"
#include <SFML/Graphics.hpp>

using Game::BaseEventHandler;

bool BaseEventHandler::handleEvent(sf::Window& window, sf::Event event) {
	switch (event.type) {
	case sf::Event::Closed:
		window.close();
		return true;
#ifndef MULTITHREADED
	case sf::Event::Resized:
		static_cast<sf::RenderWindow&>(window).setView(_keepRatio(event.size, Game::options.windowSize));
		return true;
#endif
	case sf::Event::KeyPressed:
		switch (event.key.code) {
#ifndef MULTITHREADED
		case sf::Keyboard::V:
			Game::options.vsync = !Game::options.vsync;
			window.setFramerateLimit(Game::options.vsync ? Game::options.framerateLimit : 0);
			return true;
		default:
			break;
#endif
		}
	default: 
		break;
	}
	return false;
}

sf::View BaseEventHandler::_keepRatio(const sf::Event::SizeEvent& size, const sf::Vector2u& designedsize) {
	sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);

	const float screenwidth = size.width / static_cast<float>(designedsize.x),
	            screenheight = size.height / static_cast<float>(designedsize.y);

	if (screenwidth > screenheight) {
		viewport.width = screenheight / screenwidth;
		viewport.left = (1.f - viewport.width) / 2.f;
	} else if (screenwidth < screenheight) {
		viewport.height = screenwidth / screenheight;
		viewport.top = (1.f - viewport.height) / 2.f;
	}

	sf::View view(sf::FloatRect(0, 0, designedsize.x , designedsize.y));
	view.setViewport(viewport);

	return view;
}
