// SFML Emscripten backend (BOOM Remake web port)
//
// The window is the page's canvas. Its backing size is the size the game asks for; the page
// scales it with CSS, and SDL maps mouse coordinates back to the backing size.

#include <SFML/Window/Emscripten/WindowImplEmscripten.hpp>
#include <SFML/Window/Emscripten/SDLState.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Utf.hpp>

namespace sf
{
namespace priv
{
namespace
{
    Mouse::Button toSfButton(Uint8 button)
    {
        switch (button)
        {
            case SDL_BUTTON_RIGHT:  return Mouse::Right;
            case SDL_BUTTON_MIDDLE: return Mouse::Middle;
            case SDL_BUTTON_X1:     return Mouse::XButton1;
            case SDL_BUTTON_X2:     return Mouse::XButton2;
            default:                return Mouse::Left;
        }
    }
}

////////////////////////////////////////////////////////////
WindowImplEmscripten::WindowImplEmscripten(WindowHandle /* handle */) :
m_keyRepeat(true),
m_hasFocus (true)
{
    sdl::ensureInitialized();
}


////////////////////////////////////////////////////////////
WindowImplEmscripten::WindowImplEmscripten(VideoMode mode, const String& title, unsigned long /* style */, const ContextSettings& /* settings */) :
m_keyRepeat(true),
m_hasFocus (true)
{
    sdl::ensureInitialized();
    setSize(Vector2u(mode.width, mode.height));
    setTitle(title);
}


////////////////////////////////////////////////////////////
WindowImplEmscripten::~WindowImplEmscripten()
{
    // The canvas and its WebGL context outlive every sf::Window
}


////////////////////////////////////////////////////////////
WindowHandle WindowImplEmscripten::getSystemHandle() const
{
    return sdl::window();
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::processEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        Event event;
        switch (e.type)
        {
            case SDL_QUIT:
                event.type = Event::Closed;
                pushEvent(event);
                break;

            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED || e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                {
                    m_hasFocus = (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED);
                    event.type = m_hasFocus ? Event::GainedFocus : Event::LostFocus;
                    pushEvent(event);
                }
                else if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    event.type        = Event::Resized;
                    event.size.width  = static_cast<unsigned int>(e.window.data1);
                    event.size.height = static_cast<unsigned int>(e.window.data2);
                    pushEvent(event);
                }
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                const bool down = (e.type == SDL_KEYDOWN);
                const Keyboard::Key key = sdl::toSfKey(e.key.keysym.sym);
                const Keyboard::Scancode scan = sdl::toSfScancode(e.key.keysym.scancode);
                if (key != Keyboard::Unknown)
                    sdl::keyDown[key] = down;
                if (scan != Keyboard::Scan::Unknown)
                    sdl::scanDown[scan] = down;

                if (down && e.key.repeat && !m_keyRepeat)
                    break;

                const Uint16 mod = e.key.keysym.mod;
                event.type          = down ? Event::KeyPressed : Event::KeyReleased;
                event.key.code      = key;
                event.key.scancode  = scan;
                event.key.alt       = (mod & KMOD_ALT) != 0;
                event.key.control   = (mod & KMOD_CTRL) != 0;
                event.key.shift     = (mod & KMOD_SHIFT) != 0;
                event.key.system    = (mod & KMOD_GUI) != 0;
                pushEvent(event);

                // Browsers often don't send keyup for keys released while Cmd is held:
                // forget all keys when Cmd goes up so none gets stuck
                if (!down && (e.key.keysym.sym == SDLK_LGUI || e.key.keysym.sym == SDLK_RGUI))
                {
                    for (int k = 0; k < Keyboard::KeyCount; ++k)
                        sdl::keyDown[k] = false;
                    for (int s = 0; s < Keyboard::Scan::ScancodeCount; ++s)
                        sdl::scanDown[s] = false;
                }
                break;
            }

            case SDL_TEXTINPUT:
            {
                const char* begin = e.text.text;
                const char* end = begin + SDL_strlen(begin);
                while (begin < end)
                {
                    Uint32 codepoint;
                    begin = Utf8::decode(begin, end, codepoint);
                    event.type = Event::TextEntered;
                    event.text.unicode = codepoint;
                    pushEvent(event);
                }
                break;
            }

            case SDL_MOUSEMOTION:
                sdl::mousePosition = Vector2i(e.motion.x, e.motion.y);
                event.type = Event::MouseMoved;
                event.mouseMove.x = e.motion.x;
                event.mouseMove.y = e.motion.y;
                pushEvent(event);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                const Mouse::Button button = toSfButton(e.button.button);
                sdl::mouseDown[button] = (e.type == SDL_MOUSEBUTTONDOWN);
                sdl::mousePosition = Vector2i(e.button.x, e.button.y);
                event.type = (e.type == SDL_MOUSEBUTTONDOWN) ? Event::MouseButtonPressed : Event::MouseButtonReleased;
                event.mouseButton.button = button;
                event.mouseButton.x = e.button.x;
                event.mouseButton.y = e.button.y;
                pushEvent(event);
                break;
            }

            case SDL_MOUSEWHEEL:
                event.type = Event::MouseWheelScrolled;
                event.mouseWheelScroll.wheel = Mouse::VerticalWheel;
                event.mouseWheelScroll.delta = static_cast<float>(e.wheel.y);
                event.mouseWheelScroll.x = sdl::mousePosition.x;
                event.mouseWheelScroll.y = sdl::mousePosition.y;
                pushEvent(event);
                break;

            default:
                break;
        }
    }
}


////////////////////////////////////////////////////////////
Vector2i WindowImplEmscripten::getPosition() const
{
    return Vector2i(0, 0);
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setPosition(const Vector2i& /* position */)
{
}


////////////////////////////////////////////////////////////
Vector2u WindowImplEmscripten::getSize() const
{
    int w = 0, h = 0;
    SDL_GetWindowSize(sdl::window(), &w, &h);
    return Vector2u(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setSize(const Vector2u& size)
{
    SDL_SetWindowSize(sdl::window(), static_cast<int>(size.x), static_cast<int>(size.y));
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setTitle(const String& /* title */)
{
    // The page sets its own title
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setIcon(unsigned int /* width */, unsigned int /* height */, const Uint8* /* pixels */)
{
    // The page sets its own favicon
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setVisible(bool /* visible */)
{
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursorVisible(bool visible)
{
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursorGrabbed(bool /* grabbed */)
{
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setMouseCursor(const CursorImpl& /* cursor */)
{
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::setKeyRepeatEnabled(bool enabled)
{
    m_keyRepeat = enabled;
}


////////////////////////////////////////////////////////////
void WindowImplEmscripten::requestFocus()
{
}


////////////////////////////////////////////////////////////
bool WindowImplEmscripten::hasFocus() const
{
    return m_hasFocus;
}

} // namespace priv
} // namespace sf
