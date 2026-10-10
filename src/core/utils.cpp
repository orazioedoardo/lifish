#include "utils.hpp"
#include "Component.hpp"
#include "LoopingMusic.hpp"
#include "Options.hpp"
#include "ShadedText.hpp"
#include <fstream>
#include <cassert>
#include <iostream>
#include <random>
#ifdef __EMSCRIPTEN__
#	include <emscripten.h>
#endif
#if defined(_WIN32) || defined(__MINGW32__)
#	include <windows.h>
#else
#	include <sys/stat.h>
#endif

void lif::testMusic() {
	sf::Music sample;
	sample.openFromFile(lif::getAsset("music", "music1.ogg"));
	sample.play();
	sample.setVolume(lif::options.musicVolume);
	sf::sleep(sf::milliseconds(200));
	sample.stop();
}

std::string lif::toRelativePath(const std::string& path) {
	auto idx = lif::assetDir.length();

	if (idx >= path.length()) return path;

	while (idx < path.length() && path[idx] == lif::DIRSEP)
		++idx;

	return idx < path.length() ? path.substr(idx) : "";
}

sf::View lif::keepRatio(const sf::Vector2f& size, const sf::Vector2u& designedsize) {
	sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);

	const float screenwidth = size.x / static_cast<float>(designedsize.x),
	            screenheight = size.y / static_cast<float>(designedsize.y);

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

bool lif::createDirIfNotExisting(const std::filesystem::path& path) {
	std::error_code err_created, err_exists;

	if (!std::filesystem::create_directories(path, err_created)) {
		// Directory not created
		if (std::filesystem::exists(path, err_exists)) {
			// Directory already existing
			return true;
		}
		std::cerr << "[ WARNING ] could not create directory " << path << ". " << err_created.message() << std::endl;
		return false;
	}

	return true;
}

std::string lif::readEntireFile(const std::string& path) {
	std::ifstream f(path);
	if (!f) return "";
	std::ostringstream sstr;
	sstr << f.rdbuf();
	return sstr.str();
}

#ifdef __EMSCRIPTEN__
static bool persistentFilesDirty = false;

// Waits for the write to finish: the game may quit right after saving, and exiting
// the runtime closes the database, failing any write still in flight.
EM_ASYNC_JS(void, syncFSToIndexedDB, (), {
	await new Promise((resolve) => FS.syncfs(false, (err) => {
		if (err) console.warn("Saving to IndexedDB failed:", err);
		resolve();
	}));
});
#endif

void lif::persistentFilesChanged() {
#ifdef __EMSCRIPTEN__
	persistentFilesDirty = true;
#endif
}

void lif::flushPersistentFiles() {
#ifdef __EMSCRIPTEN__
	if (!persistentFilesDirty)
		return;
	persistentFilesDirty = false;
	syncFSToIndexedDB();
#endif
}
