// SFML Emscripten backend (BOOM Remake web port)
//
// The browser gives us one canvas and one WebGL context. Both are created on first use (SFML
// creates its shared GL context before any window, e.g. when loading textures) and live for the
// rest of the program. Every sf::Window and every sf::Context use them.

#ifndef SFML_EMSCRIPTEN_SDLSTATE_HPP
#define SFML_EMSCRIPTEN_SDLSTATE_HPP

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <SDL2/SDL.h>

namespace sf
{
namespace priv
{
namespace sdl
{
    // Creates the SDL window (the canvas) and the WebGL context if needed
    void ensureInitialized();

    SDL_Window* window();

    Keyboard::Key toSfKey(SDL_Keycode code);
    Keyboard::Scancode toSfScancode(SDL_Scancode code);

    // Input state, updated from SDL events by WindowImplEmscripten::processEvents()
    extern bool keyDown[Keyboard::KeyCount];
    extern bool scanDown[Keyboard::Scan::ScancodeCount];
    extern bool mouseDown[Mouse::ButtonCount];
    extern Vector2i mousePosition;
} // namespace sdl
} // namespace priv
} // namespace sf

#endif
