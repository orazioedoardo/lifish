// SFML Emscripten backend (BOOM Remake web port): no sensors

#ifndef SFML_SENSORIMPLEMSCRIPTEN_HPP
#define SFML_SENSORIMPLEMSCRIPTEN_HPP

#include <SFML/Window/Sensor.hpp>
#include <SFML/System/Vector3.hpp>

namespace sf
{
namespace priv
{
class SensorImpl
{
public:
    static void initialize() {}
    static void cleanup() {}
    static bool isAvailable(Sensor::Type /* sensor */) { return false; }
    bool open(Sensor::Type /* sensor */) { return false; }
    void close() {}
    Vector3f update() { return Vector3f(); }
    void setEnabled(bool /* enabled */) {}
};

} // namespace priv
} // namespace sf

#endif
