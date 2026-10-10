// SFML Emscripten backend (BOOM Remake web port)
//
// WebGL contexts can't share objects, so every SFML context (the hidden shared one, transient
// ones, the window's) is the same single WebGL context. Activating one is a no-op.

#ifndef SFML_EMSCRIPTENCONTEXT_HPP
#define SFML_EMSCRIPTENCONTEXT_HPP

#include <SFML/Window/GlContext.hpp>

namespace sf
{
namespace priv
{
class WindowImpl;

class EmscriptenContext : public GlContext
{
public:
    EmscriptenContext(EmscriptenContext* shared);
    EmscriptenContext(EmscriptenContext* shared, const ContextSettings& settings, const WindowImpl* owner, unsigned int bitsPerPixel);
    EmscriptenContext(EmscriptenContext* shared, const ContextSettings& settings, unsigned int width, unsigned int height);
    ~EmscriptenContext();

    static GlFunctionPointer getFunction(const char* name);

    virtual void display();
    virtual void setVerticalSyncEnabled(bool enabled);

protected:
    virtual bool makeCurrent(bool current);
};

} // namespace priv
} // namespace sf

#endif
