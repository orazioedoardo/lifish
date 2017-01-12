#include "HauntingSpiritBoss.hpp"
#include "HauntedStatue.hpp"
#include "Collider.hpp"
#include "Animated.hpp"
#include "Drawable.hpp"
#include "Clock.hpp"
#include "FreeSighted.hpp"
#include "CircleShootingPattern.hpp"
#include "ScatterVsPlayerPattern.hpp"
#include "Killable.hpp"
#include "Angle.hpp"
#include "core.hpp"
#include <algorithm>

using lif::HauntingSpiritBoss;

HauntingSpiritBoss::HauntingSpiritBoss(const sf::Vector2f& pos)
	: lif::Boss(pos)
	, state(State::START)
{
	// This boss has no Lifed component: it dies when there are no HauntedStatues left in the level.
	addComponent(new lif::FreeSighted(*this))->setActive(false);
	animated = addComponent(new lif::Animated(*this, lif::getAsset("graphics", "haunting_spirit.png")));
	const auto size = 4 * lif::TILE_SIZE;
	// This is needed by parent class
	collider = addComponent(new lif::Collider(*this, lif::c_layers::DEFAULT,
				sf::Vector2f(size, size),
				sf::Vector2f(-size/2, -size/2), true));
	collider->setActive(false);
	animated->addAnimation("idle", {
		sf::IntRect(0 * size, 0, size, size),
		sf::IntRect(1 * size, 0, size, size),
		sf::IntRect(2 * size, 0, size, size),
		sf::IntRect(3 * size, 0, size, size),
		sf::IntRect(4 * size, 0, size, size),
		sf::IntRect(5 * size, 0, size, size)
	}, true);
	animated->addAnimation("start", {
		sf::IntRect(0 * size, size, size, size),
		sf::IntRect(1 * size, size, size, size),
		sf::IntRect(2 * size, size, size, size),
		sf::IntRect(3 * size, size, size, size),
		sf::IntRect(4 * size, size, size, size),
		sf::IntRect(5 * size, size, size, size)
	});
	animated->addAnimation("transition", {
		sf::IntRect(0 * size, 2 * size, size, size),
		sf::IntRect(1 * size, 2 * size, size, size),
		sf::IntRect(2 * size, 2 * size, size, size),
	});
	animated->addAnimation("death", {
		sf::IntRect(0 * size, 3 * size, size, size),
		sf::IntRect(1 * size, 3 * size, size, size),
		sf::IntRect(2 * size, 3 * size, size, size),
		sf::IntRect(3 * size, 3 * size, size, size),
		sf::IntRect(4 * size, 3 * size, size, size),
		sf::IntRect(5 * size, 3 * size, size, size),
	});
	animated->getSprite().setOrigin(size/2, size/2);
	animClock = addComponent(new lif::Clock(*this));
	hauntClock = addComponent(new lif::Clock(*this));
	atkClock = addComponent(new lif::Clock(*this));

	lif::BulletInfo bullet;
	bullet.id = 102;
	bullet.speed = 1;
	bullet.damage = 4;
	auto circle = addComponent(new lif::CircleShootingPattern(*this, bullet));
	circle->consecutiveShots = 6;
	circle->timeBetweenShots = sf::seconds(0.5);
	circle->bulletsPerShot = 6;
	circle->rotationPerShot = lif::radians(lif::PI / 5.);
	shootPatterns[0] = circle;
	auto spiral = addComponent(new lif::CircleShootingPattern(*this, bullet));
	spiral->consecutiveShots = 50;
	spiral->timeBetweenShots = sf::seconds(0.1);
	spiral->bulletsPerShot = 1;
	spiral->rotationPerShot = lif::radians(0.4);
	spiral->randomizeShootAngle = true;
	shootPatterns[1] = spiral;
	auto scatter = addComponent(new lif::ScatterVsPlayerPattern(*this, bullet));
	scatter->consecutiveShots = 18;
	scatter->timeBetweenShots = sf::seconds(0.1);
	scatter->scatterAngle = lif::degrees(30);
	shootPatterns[2] = scatter;
}

void HauntingSpiritBoss::update() {
	lif::Entity::update();

	switch (state) {
	case State::START:
		_updateStart();
		break;
	case State::SEARCHING:
		_updateSearching();
		break;
	case State::SELECT_NEW_STATUE:
		_updateSelectNewStatue();
		break;
	case State::TRANSITIONING_BEGIN:
		_updateTransitioningBegin();
		break;
	case State::TRANSITIONING_END:
		_updateTransitioningEnd();
		break;
	case State::HAUNTING:
		_updateHaunting();
		break;
	case State::DYING:
		_updateDying();
		break;
	}
}

void HauntingSpiritBoss::_updateStart() {
	// Task: play initial animation after a delay
	if (animClock->getElapsedTime() < sf::seconds(2)) return;
	if (!animated->getSprite().isPlaying()) {
		animated->getSprite().setLooped(true);
		animated->setAnimation("idle");
		animated->getSprite().play();
		get<lif::FreeSighted>()->setActive(true);
		state = State::SEARCHING;
	} else if (animated->getAnimationName() == "idle") {
		animated->getSprite().setLooped(false, false);	
		animated->setAnimation("start");
		animated->getSprite().play();
	}
}

void HauntingSpiritBoss::_updateSearching() {
	// Task: find all the HauntedStatues in the level. Do this once and keep track of them internally thereafter.
	auto sighted = get<lif::FreeSighted>();
	const auto seen = sighted->entitiesSeen();
	for (auto& pair : seen) {
		if (auto statue = std::dynamic_pointer_cast<lif::HauntedStatue>(pair.first.lock()))
			if (!statue->isPossessed())
				statues.push_back(statue);
	}
	sighted->setActive(false); // no need for this anymore
	state = State::SELECT_NEW_STATUE;
}

void HauntingSpiritBoss::_updateSelectNewStatue() {
	// Task: select a statue to possess
	for (auto it = statues.begin(); it != statues.end(); ) {
		if (it->expired())
			it = statues.erase(it);
		else
			++it;
	}
	if (statues.size() == 0) {
		state = State::DYING;
		return;
	}
	std::uniform_int_distribution<> dist(0, statues.size() - 1);
	targetStatue = statues[dist(lif::rng)];
	state = State::TRANSITIONING_BEGIN;
}

void HauntingSpiritBoss::_updateTransitioningBegin() {
	// Task: play the transitioning animation and set the next haunted statue as `possessed`
	if (animated->getAnimationName() != "transition") {
		animated->setAnimation("transition");
		animated->getSprite().setLooped(false, false);
		animated->getSprite().play();
		animClock->restart();
	}
	if (position.y < 0) {
		// Center on the target statue
		const auto statue = targetStatue.lock();
		if (statue == nullptr) {
			state = State::DYING;
			return;
		}
		position.x = statue->getPosition().x + lif::TILE_SIZE / 2;
		animated->getSprite().rotate(180);
		state = State::TRANSITIONING_END;
	} else {
		position.y -= animClock->restart().asSeconds() * 200;
	}
}

void HauntingSpiritBoss::_updateTransitioningEnd() {
	const auto statue = targetStatue.lock();
	if (statue == nullptr) {
		state = State::SELECT_NEW_STATUE;
		return;
	}
	if (position.y >= statue->getPosition().y) {
		animated->getSprite().rotate(180);
		animated->getSprite().setLooped(true);
		animated->setAnimation("idle");
		animated->getSprite().play();
		hauntClock->restart();
		atkClock->restart();
		targetStatue.lock()->setPossessed(true);
		get<lif::Drawable>()->setActive(false);
		state = State::HAUNTING;
		return;
	}
	position.y += animClock->restart().asSeconds() * 200;
}

void HauntingSpiritBoss::_updateHaunting() {
	// Task: attack the player; leave after some delay
	if (targetStatue.expired()) {
		get<lif::Drawable>()->setActive(true);
		state = State::SELECT_NEW_STATUE;
		if (curShootPattern != nullptr)
			curShootPattern->setActive(false);
		return;
	}
	if (_isShooting()) {
		atkClock->restart();
		return;
	}
	if (hauntClock->getElapsedTime() > sf::seconds(25)) {
		get<lif::Drawable>()->setActive(true);
		targetStatue.lock()->setPossessed(false);
		state = State::SELECT_NEW_STATUE;
		return;
	}
	if (atkClock->getElapsedTime() > sf::seconds(3)) {
		std::uniform_int_distribution<unsigned short> dist(0, shootPatterns.size() - 1);
		curShootPattern = shootPatterns[dist(lif::rng)];
		curShootPattern->resetAndPlay();
	}
}

void HauntingSpiritBoss::_updateDying() {
	// Task: play death animation
	if (animated->getAnimationName() != "death") {
		killable->kill();
		animated->setAnimation("death");
		animated->getSprite().setLooped(false);
		animated->getSprite().play();
	}
}

bool HauntingSpiritBoss::_isShooting() const {
	return curShootPattern != nullptr && curShootPattern->isActive();
}
