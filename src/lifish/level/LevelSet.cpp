#include "LevelSet.hpp"
#include "json.hpp"
#include "utils.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <typeinfo>

using json = nlohmann::json;
using lif::LevelSet;
using lif::Level;

/** Optional additional metadata; all values must be strings */
static constexpr const char* AVAIL_METADATA[] = {
	"author", "created", "difficulty", "comment", "name"
};

LevelSet::LevelSet(const std::string& path) {
	loadFromFile(path);
}

void LevelSet::loadFromFile(const std::string& path) {
	json levelJSON = json::parse(std::ifstream(path));

	// load metadata
	for (const auto& key : AVAIL_METADATA) {
		if (levelJSON.find(key) != levelJSON.end())
			metadata[key] = levelJSON[(std::string)key].get<std::string>();
	}
	metadata["path"] = path;

	// load tracks data
	const auto tracksdata = levelJSON["tracks"];
	unsigned short tracknum = 1;
	/* trackinfo = {
	 *	"loop": {
	 *		"start": float,
	 *		"end": float
	 *	}
	 * }
	 */
	for (const auto& trackinfo : tracksdata) {
		auto loop = trackinfo["loop"];
		float loopstart = loop["start"];
		float looplength = -1;
		auto len = loop.find("length");
		if (len != loop.end()) {
			looplength = *len;
		} else {
			float loopend = loop["end"];
			looplength = loopend - loopstart;
		}
		tracks.emplace_back(getNthTrack(tracknum++, loopstart, looplength));
	}

	// load enemies data
	const auto enemydata = levelJSON["enemies"];
	unsigned short enemynum = 0;
	/* enemyinfo = {
	 *	"ai": ushort,
	 *	"speed": ushort,
	 *	"attack": {
	 *		"type": string,
	 *		"damage": short,
	 *		"id": ushort         [opt]
	 *		"speed": float,      [opt]
	 *		"fireRate": float,   [opt]
	 *		"blockTime": float,  [opt]
	 *		"range": float,      [opt, default=-1]
	 *		"tileRange": short,  [opt, only valid if `range` was not specified]
	 *	}
	 *
	 * }
	 */
	for (const auto& enemyinfo : enemydata) {
		enemies[enemynum].ai = enemyinfo["ai"];
		enemies[enemynum].speed = enemyinfo["speed"];
		
		auto atk = enemyinfo["attack"];
		auto atktype = atk["type"];
		
		enemies[enemynum].attack.type = lif::AttackType::AXIS_BOUND;
		for (unsigned i = 0; i < atktype.size(); ++i) {
			AttackType type;
			const auto at = atktype[i].get<std::string>();
			if (!lif::stringToAttackType(at, type))
				throw std::invalid_argument(at.c_str());

			enemies[enemynum].attack.type = AttackType(
					static_cast<unsigned int>(enemies[enemynum].attack.type)
					| static_cast<unsigned int>(type));
		}

		// Mandatory fields
		enemies[enemynum].attack.bullet.damage = atk["damage"].get<short>();
		// Optional fields
		auto it = atk.find("speed");
		if (it != atk.end())
			enemies[enemynum].attack.bullet.speed = it->get<float>();
		
		it = atk.find("id");
		if (it != atk.end())
			enemies[enemynum].attack.bullet.id = it->get<unsigned short>();

		it = atk.find("fireRate");
		if (it != atk.end())
			enemies[enemynum].attack.fireRate = it->get<float>();

		it = atk.find("blockTime");
		if (it != atk.end())
			enemies[enemynum].attack.blockTime = sf::milliseconds(it->get<float>());

		// Find range: first search for `range` (in pixels); if not found, search `tileRange`.
		// If neither is found, set range to -1 (infinite).
		enemies[enemynum].attack.bullet.range = -1;
		bool range_found = false;
		it = atk.find("range");
		if (it != atk.end()) {
			enemies[enemynum].attack.bullet.range = it->get<float>();
			range_found = true;
		}
		if (!range_found) {
			it = atk.find("tileRange");
			if (it != atk.end()) {
				enemies[enemynum].attack.bullet.range =
					static_cast<float>(it->get<short>() * lif::TILE_SIZE);
			}
		}
		++enemynum;
	}


	// load levels data
	const auto levelsdata = levelJSON["levels"];

	unsigned short lvnum = 1;
	/* lvinfo = {
	 *	"time": uint,
	 *	"tilemap": string,
	 *	"music": ushort,
	 *	"width": ushort,
	 *	"height": ushort,
	 *	"tileIDs": {
	 *		"border": ushort,
	 *		"bg": ushort,
	 *		"fixed": ushort,
	 *		"breakable": ushort
	 *	},
	 *	"effects": [string] [opt]
	 * }
	 */
	for (const auto& lvinfo : levelsdata) {
		LevelInfo info;
		info.levelnum          = lvnum++;
		info.time              = lvinfo["time"];
		info.track             = tracks[lvinfo["music"].get<unsigned short>()-1];
		info.tilemap           = lvinfo["tilemap"];
		info.width             = lvinfo["width"];
		info.height            = lvinfo["height"];
		info.tileIDs.border    = lvinfo["tileIDs"]["border"];
		info.tileIDs.bg        = lvinfo["tileIDs"]["bg"];
		info.tileIDs.fixed     = lvinfo["tileIDs"]["fixed"];
		info.tileIDs.breakable = lvinfo["tileIDs"]["breakable"];
		auto it = lvinfo.find("effects");
		if (it != lvinfo.end()) {
			for (const auto& s : *it) {
				info.effects.insert(s.get<std::string>());
			}
		}
		levels.emplace_back(info);
	}
}

std::unique_ptr<Level> LevelSet::getLevel(unsigned short num) const {
	std::unique_ptr<Level> level;
	if (num > 0 && num <= levels.size()) {
		level = std::make_unique<Level>(*this);
		level->info = levels[num - 1];
		if (!level->init()) {
			level.reset();
		}
	}
	return level;
}

std::string LevelSet::toString() const {
	std::stringstream ss;
	ss << "Level Set: " << metadata.find("name")->second << "\r\n"
	   << "Tracks: " << tracks.size() << "\r\n"
	   << "Levels: " << levels.size() << "\r\n";
	for (const auto& pair : metadata) {
		if (pair.first == "name") continue;
		ss << pair.first << ": " << pair.second << "\r\n";
	}
	return ss.str();
}

std::string LevelSet::getMeta(const std::string& key) const {
	auto val = metadata.find(key);
	if (val == metadata.end()) return "";
	return val->second;
}
