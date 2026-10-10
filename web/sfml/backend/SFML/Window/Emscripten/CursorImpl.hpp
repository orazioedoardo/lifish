// SFML Emscripten backend (BOOM Remake web port): the browser's default cursor only

#ifndef SFML_CURSORIMPLEMSCRIPTEN_HPP
#define SFML_CURSORIMPLEMSCRIPTEN_HPP

#include <SFML/Window/Cursor.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Vector2.hpp>

namespace sf
{
namespace priv
{
class CursorImpl : NonCopyable
{
public:
    CursorImpl() {}
    ~CursorImpl() {}
    bool loadFromPixels(const Uint8* /* pixels */, Vector2u /* size */, Vector2u /* hotspot */) { return false; }
    bool loadFromSystem(Cursor::Type /* type */) { return false; }
};

} // namespace priv
} // namespace sf

#endif
