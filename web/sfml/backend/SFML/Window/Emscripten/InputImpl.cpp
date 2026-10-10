// SFML Emscripten backend (BOOM Remake web port)
//
// Real-time input state comes from the SDL events seen by WindowImplEmscripten.

#include <SFML/Window/Emscripten/InputImpl.hpp>
#include <SFML/Window/Emscripten/SDLState.hpp>
#include <SFML/System/String.hpp>

namespace sf
{
namespace priv
{
bool InputImpl::isKeyPressed(Keyboard::Key key)
{
    return key >= 0 && key < Keyboard::KeyCount && sdl::keyDown[key];
}

bool InputImpl::isKeyPressed(Keyboard::Scancode code)
{
    return code >= 0 && code < Keyboard::Scan::ScancodeCount && sdl::scanDown[code];
}

// Scancode <-> key translation isn't used by the game
Keyboard::Key InputImpl::localize(Keyboard::Scancode /* code */)
{
    return Keyboard::Unknown;
}

Keyboard::Scancode InputImpl::delocalize(Keyboard::Key /* key */)
{
    return Keyboard::Scan::Unknown;
}

String InputImpl::getDescription(Keyboard::Scancode /* code */)
{
    return "";
}

void InputImpl::setVirtualKeyboardVisible(bool /* visible */)
{
}

bool InputImpl::isMouseButtonPressed(Mouse::Button button)
{
    return button >= 0 && button < Mouse::ButtonCount && sdl::mouseDown[button];
}

Vector2i InputImpl::getMousePosition()
{
    return sdl::mousePosition;
}

Vector2i InputImpl::getMousePosition(const WindowBase& /* relativeTo */)
{
    return sdl::mousePosition;
}

void InputImpl::setMousePosition(const Vector2i& /* position */)
{
}

void InputImpl::setMousePosition(const Vector2i& /* position */, const WindowBase& /* relativeTo */)
{
}

bool InputImpl::isTouchDown(unsigned int /* finger */)
{
    return false;
}

Vector2i InputImpl::getTouchPosition(unsigned int /* finger */)
{
    return Vector2i();
}

Vector2i InputImpl::getTouchPosition(unsigned int /* finger */, const WindowBase& /* relativeTo */)
{
    return Vector2i();
}

} // namespace priv
} // namespace sf
