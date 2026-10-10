// SFML Emscripten backend (BOOM Remake web port)

#include <SFML/Window/Emscripten/SDLState.hpp>
#include <SFML/System/Err.hpp>
#include <gl4esinit.h>
#include <cstdlib>

extern "C" void* emscripten_GetProcAddress(const char* name);

namespace sf
{
namespace priv
{
namespace sdl
{
bool keyDown[Keyboard::KeyCount] = {};
bool scanDown[Keyboard::Scan::ScancodeCount] = {};
bool mouseDown[Mouse::ButtonCount] = {};
Vector2i mousePosition;

namespace
{
    SDL_Window*   sdlWindow = NULL;
    SDL_GLContext glContext = NULL;

    void getMainFramebufferSize(int* width, int* height)
    {
        SDL_GL_GetDrawableSize(sdlWindow, width, height);
    }
}

////////////////////////////////////////////////////////////
void ensureInitialized()
{
    if (sdlWindow)
        return;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
    {
        err() << "SDL_Init failed: " << SDL_GetError() << std::endl;
        std::abort();
    }

    // WebGL 1; gl4es emulates SFML's fixed-function OpenGL on top of it
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // The size is set by the first sf::Window
    sdlWindow = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
    if (!sdlWindow)
    {
        err() << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        std::abort();
    }

    glContext = SDL_GL_CreateContext(sdlWindow);
    if (!glContext)
    {
        err() << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        std::abort();
    }

    // gl4es needs a live WebGL context. Give it the WebGL functions too: without them it skips
    // its hardware test and leaves the color attachment count at 0, which breaks every FBO
    // (render textures, texture copies)
    set_getprocaddress(&emscripten_GetProcAddress);
    set_getmainfbsize(&getMainFramebufferSize);
    initialize_gl4es();

    // Without text input SDL calls preventDefault on every key it handles, so keys like Alt,
    // Space or / don't trigger browser menus, scrolling or quick find. The game reads key
    // presses only (no TextEntered).
    SDL_StopTextInput();
}

////////////////////////////////////////////////////////////
SDL_Window* window()
{
    ensureInitialized();
    return sdlWindow;
}

////////////////////////////////////////////////////////////
Keyboard::Key toSfKey(SDL_Keycode code)
{
    if (code >= SDLK_a && code <= SDLK_z)
        return static_cast<Keyboard::Key>(Keyboard::A + (code - SDLK_a));
    if (code >= SDLK_0 && code <= SDLK_9)
        return static_cast<Keyboard::Key>(Keyboard::Num0 + (code - SDLK_0));
    if (code >= SDLK_F1 && code <= SDLK_F12)
        return static_cast<Keyboard::Key>(Keyboard::F1 + (code - SDLK_F1));
    if (code >= SDLK_F13 && code <= SDLK_F15)
        return static_cast<Keyboard::Key>(Keyboard::F13 + (code - SDLK_F13));

    switch (code)
    {
        case SDLK_ESCAPE:       return Keyboard::Escape;
        case SDLK_LCTRL:        return Keyboard::LControl;
        case SDLK_LSHIFT:       return Keyboard::LShift;
        case SDLK_LALT:         return Keyboard::LAlt;
        case SDLK_LGUI:         return Keyboard::LSystem;
        case SDLK_RCTRL:        return Keyboard::RControl;
        case SDLK_RSHIFT:       return Keyboard::RShift;
        case SDLK_RALT:         return Keyboard::RAlt;
        case SDLK_RGUI:         return Keyboard::RSystem;
        case SDLK_MENU:         return Keyboard::Menu;
        case SDLK_LEFTBRACKET:  return Keyboard::LBracket;
        case SDLK_RIGHTBRACKET: return Keyboard::RBracket;
        case SDLK_SEMICOLON:    return Keyboard::Semicolon;
        case SDLK_COMMA:        return Keyboard::Comma;
        case SDLK_PERIOD:       return Keyboard::Period;
        case SDLK_QUOTE:        return Keyboard::Apostrophe;
        case SDLK_SLASH:        return Keyboard::Slash;
        case SDLK_BACKSLASH:    return Keyboard::Backslash;
        case SDLK_BACKQUOTE:    return Keyboard::Grave;
        case SDLK_EQUALS:       return Keyboard::Equal;
        case SDLK_MINUS:        return Keyboard::Hyphen;
        case SDLK_SPACE:        return Keyboard::Space;
        case SDLK_RETURN:       return Keyboard::Enter;
        case SDLK_BACKSPACE:    return Keyboard::Backspace;
        case SDLK_TAB:          return Keyboard::Tab;
        case SDLK_PAGEUP:       return Keyboard::PageUp;
        case SDLK_PAGEDOWN:     return Keyboard::PageDown;
        case SDLK_END:          return Keyboard::End;
        case SDLK_HOME:         return Keyboard::Home;
        case SDLK_INSERT:       return Keyboard::Insert;
        case SDLK_DELETE:       return Keyboard::Delete;
        case SDLK_KP_PLUS:      return Keyboard::Add;
        case SDLK_KP_MINUS:     return Keyboard::Subtract;
        case SDLK_KP_MULTIPLY:  return Keyboard::Multiply;
        case SDLK_KP_DIVIDE:    return Keyboard::Divide;
        case SDLK_KP_ENTER:     return Keyboard::Enter;
        case SDLK_LEFT:         return Keyboard::Left;
        case SDLK_RIGHT:        return Keyboard::Right;
        case SDLK_UP:           return Keyboard::Up;
        case SDLK_DOWN:         return Keyboard::Down;
        case SDLK_KP_0:         return Keyboard::Numpad0;
        case SDLK_KP_1:         return Keyboard::Numpad1;
        case SDLK_KP_2:         return Keyboard::Numpad2;
        case SDLK_KP_3:         return Keyboard::Numpad3;
        case SDLK_KP_4:         return Keyboard::Numpad4;
        case SDLK_KP_5:         return Keyboard::Numpad5;
        case SDLK_KP_6:         return Keyboard::Numpad6;
        case SDLK_KP_7:         return Keyboard::Numpad7;
        case SDLK_KP_8:         return Keyboard::Numpad8;
        case SDLK_KP_9:         return Keyboard::Numpad9;
        case SDLK_PAUSE:        return Keyboard::Pause;
        default:                return Keyboard::Unknown;
    }
}

////////////////////////////////////////////////////////////
Keyboard::Scancode toSfScancode(SDL_Scancode code)
{
    if (code >= SDL_SCANCODE_A && code <= SDL_SCANCODE_Z)
        return static_cast<Keyboard::Scancode>(Keyboard::Scan::A + (code - SDL_SCANCODE_A));
    if (code >= SDL_SCANCODE_1 && code <= SDL_SCANCODE_0)
        return static_cast<Keyboard::Scancode>(Keyboard::Scan::Num1 + (code - SDL_SCANCODE_1));
    if (code >= SDL_SCANCODE_F1 && code <= SDL_SCANCODE_F12)
        return static_cast<Keyboard::Scancode>(Keyboard::Scan::F1 + (code - SDL_SCANCODE_F1));

    switch (code)
    {
        case SDL_SCANCODE_RETURN:    return Keyboard::Scan::Enter;
        case SDL_SCANCODE_ESCAPE:    return Keyboard::Scan::Escape;
        case SDL_SCANCODE_BACKSPACE: return Keyboard::Scan::Backspace;
        case SDL_SCANCODE_TAB:       return Keyboard::Scan::Tab;
        case SDL_SCANCODE_SPACE:     return Keyboard::Scan::Space;
        case SDL_SCANCODE_LEFT:      return Keyboard::Scan::Left;
        case SDL_SCANCODE_RIGHT:     return Keyboard::Scan::Right;
        case SDL_SCANCODE_UP:        return Keyboard::Scan::Up;
        case SDL_SCANCODE_DOWN:      return Keyboard::Scan::Down;
        case SDL_SCANCODE_LCTRL:     return Keyboard::Scan::LControl;
        case SDL_SCANCODE_LSHIFT:    return Keyboard::Scan::LShift;
        case SDL_SCANCODE_LALT:      return Keyboard::Scan::LAlt;
        case SDL_SCANCODE_LGUI:      return Keyboard::Scan::LSystem;
        case SDL_SCANCODE_RCTRL:     return Keyboard::Scan::RControl;
        case SDL_SCANCODE_RSHIFT:    return Keyboard::Scan::RShift;
        case SDL_SCANCODE_RALT:      return Keyboard::Scan::RAlt;
        case SDL_SCANCODE_RGUI:      return Keyboard::Scan::RSystem;
        default:                     return Keyboard::Scan::Unknown;
    }
}

} // namespace sdl
} // namespace priv
} // namespace sf
