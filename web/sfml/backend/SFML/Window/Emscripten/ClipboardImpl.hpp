// SFML Emscripten backend (BOOM Remake web port): in-memory clipboard

#ifndef SFML_CLIPBOARDIMPLEMSCRIPTEN_HPP
#define SFML_CLIPBOARDIMPLEMSCRIPTEN_HPP

#include <SFML/System/String.hpp>

namespace sf
{
namespace priv
{
class ClipboardImpl
{
public:
    static String getString() { return contents(); }
    static void setString(const String& text) { contents() = text; }

private:
    static String& contents() { static String text; return text; }
};

} // namespace priv
} // namespace sf

#endif
