// SFML Emscripten backend (BOOM Remake web port)
//
// Gamepads through SDL2's Emscripten joystick support (the browser Gamepad API).

#ifndef SFML_JOYSTICKIMPLEMSCRIPTEN_HPP
#define SFML_JOYSTICKIMPLEMSCRIPTEN_HPP

struct _SDL_Joystick;

namespace sf
{
namespace priv
{
class JoystickImpl
{
public:
    JoystickImpl() : m_joystick(0) {}

    static void initialize();
    static void cleanup();
    static bool isConnected(unsigned int index);
    bool open(unsigned int index);
    void close();
    JoystickCaps getCapabilities() const;
    Joystick::Identification getIdentification() const;
    JoystickState update();

private:
    _SDL_Joystick*           m_joystick;
    Joystick::Identification m_identification;
};

} // namespace priv
} // namespace sf

#endif
