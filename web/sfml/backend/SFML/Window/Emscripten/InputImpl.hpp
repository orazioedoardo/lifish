// SFML Emscripten backend (BOOM Remake web port)

#ifndef SFML_INPUTIMPLEMSCRIPTEN_HPP
#define SFML_INPUTIMPLEMSCRIPTEN_HPP

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace sf
{
class WindowBase;

namespace priv
{
class InputImpl
{
public:
    static bool isKeyPressed(Keyboard::Key key);
    static bool isKeyPressed(Keyboard::Scancode code);
    static Keyboard::Key localize(Keyboard::Scancode code);
    static Keyboard::Scancode delocalize(Keyboard::Key key);
    static String getDescription(Keyboard::Scancode code);
    static void setVirtualKeyboardVisible(bool visible);
    static bool isMouseButtonPressed(Mouse::Button button);
    static Vector2i getMousePosition();
    static Vector2i getMousePosition(const WindowBase& relativeTo);
    static void setMousePosition(const Vector2i& position);
    static void setMousePosition(const Vector2i& position, const WindowBase& relativeTo);
    static bool isTouchDown(unsigned int finger);
    static Vector2i getTouchPosition(unsigned int finger);
    static Vector2i getTouchPosition(unsigned int finger, const WindowBase& relativeTo);
};

} // namespace priv
} // namespace sf

#endif
