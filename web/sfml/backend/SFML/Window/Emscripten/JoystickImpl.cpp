// SFML Emscripten backend (BOOM Remake web port)

#include <SFML/Window/JoystickImpl.hpp>
#include <SFML/Window/Emscripten/SDLState.hpp>
#include <algorithm>

namespace sf
{
namespace priv
{
namespace
{
    // SDL axis index -> SFML axis (POV axes come from the first hat)
    const Joystick::Axis axisOrder[] = { Joystick::X, Joystick::Y, Joystick::Z, Joystick::R, Joystick::U, Joystick::V };
    const int axisOrderCount = sizeof(axisOrder) / sizeof(axisOrder[0]);
}

////////////////////////////////////////////////////////////
void JoystickImpl::initialize()
{
    sdl::ensureInitialized();
}


////////////////////////////////////////////////////////////
void JoystickImpl::cleanup()
{
}


////////////////////////////////////////////////////////////
bool JoystickImpl::isConnected(unsigned int index)
{
    SDL_JoystickUpdate();
    return static_cast<int>(index) < SDL_NumJoysticks();
}


////////////////////////////////////////////////////////////
bool JoystickImpl::open(unsigned int index)
{
    m_joystick = SDL_JoystickOpen(static_cast<int>(index));
    if (!m_joystick)
        return false;

    const char* name = SDL_JoystickName(m_joystick);
    m_identification.name      = name ? name : "Gamepad";
    m_identification.vendorId  = SDL_JoystickGetVendor(m_joystick);
    m_identification.productId = SDL_JoystickGetProduct(m_joystick);
    return true;
}


////////////////////////////////////////////////////////////
void JoystickImpl::close()
{
    if (m_joystick)
        SDL_JoystickClose(m_joystick);
    m_joystick = NULL;
}


////////////////////////////////////////////////////////////
JoystickCaps JoystickImpl::getCapabilities() const
{
    JoystickCaps caps;
    caps.buttonCount = static_cast<unsigned int>(std::min(SDL_JoystickNumButtons(m_joystick), static_cast<int>(Joystick::ButtonCount)));

    const int axes = std::min(SDL_JoystickNumAxes(m_joystick), axisOrderCount);
    for (int i = 0; i < axes; ++i)
        caps.axes[axisOrder[i]] = true;

    if (SDL_JoystickNumHats(m_joystick) > 0)
        caps.axes[Joystick::PovX] = caps.axes[Joystick::PovY] = true;

    return caps;
}


////////////////////////////////////////////////////////////
Joystick::Identification JoystickImpl::getIdentification() const
{
    return m_identification;
}


////////////////////////////////////////////////////////////
JoystickState JoystickImpl::update()
{
    JoystickState state;
    state.connected = m_joystick && SDL_JoystickGetAttached(m_joystick);
    if (!state.connected)
        return state;

    const int axes = std::min(SDL_JoystickNumAxes(m_joystick), axisOrderCount);
    for (int i = 0; i < axes; ++i)
        state.axes[axisOrder[i]] = static_cast<float>(SDL_JoystickGetAxis(m_joystick, i)) * 100.f / 32767.f;

    if (SDL_JoystickNumHats(m_joystick) > 0)
    {
        const Uint8 hat = SDL_JoystickGetHat(m_joystick, 0);
        state.axes[Joystick::PovX] = (hat & SDL_HAT_RIGHT) ? 100.f : (hat & SDL_HAT_LEFT) ? -100.f : 0.f;
        state.axes[Joystick::PovY] = (hat & SDL_HAT_UP) ? 100.f : (hat & SDL_HAT_DOWN) ? -100.f : 0.f;
    }

    const int buttons = std::min(SDL_JoystickNumButtons(m_joystick), static_cast<int>(Joystick::ButtonCount));
    for (int i = 0; i < buttons; ++i)
        state.buttons[i] = SDL_JoystickGetButton(m_joystick, i) != 0;

    return state;
}

} // namespace priv
} // namespace sf
