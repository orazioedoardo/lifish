// SFML Emscripten backend (BOOM Remake web port)

#include <SFML/Window/Emscripten/EmscriptenContext.hpp>
#include <SFML/Window/Emscripten/SDLState.hpp>
#include <gl4esinit.h>
#include <emscripten.h>

namespace
{
    // Wait for the next display refresh. SDL's swap only yields with a 0 ms timeout and ignores
    // the swap interval without an emscripten main loop, so the game would render out of step with
    // the display (judder), and loops that only draw would never let the browser present.
    EM_ASYNC_JS(void, waitForAnimationFrame, (), {
        await new Promise(resolve => requestAnimationFrame(resolve));
    });
}

namespace sf
{
namespace priv
{
extern void (*emscriptenFrameHook)();

////////////////////////////////////////////////////////////
EmscriptenContext::EmscriptenContext(EmscriptenContext* /* shared */)
{
    sdl::ensureInitialized();
}


////////////////////////////////////////////////////////////
EmscriptenContext::EmscriptenContext(EmscriptenContext* /* shared */, const ContextSettings& settings, const WindowImpl* /* owner */, unsigned int /* bitsPerPixel */)
{
    sdl::ensureInitialized();
    m_settings = settings;
}


////////////////////////////////////////////////////////////
EmscriptenContext::EmscriptenContext(EmscriptenContext* /* shared */, const ContextSettings& settings, unsigned int /* width */, unsigned int /* height */)
{
    sdl::ensureInitialized();
    m_settings = settings;
}


////////////////////////////////////////////////////////////
EmscriptenContext::~EmscriptenContext()
{
}


////////////////////////////////////////////////////////////
GlFunctionPointer EmscriptenContext::getFunction(const char* name)
{
    // All GL goes through gl4es, which translates it to WebGL
    return reinterpret_cast<GlFunctionPointer>(gl4es_GetProcAddress(name));
}


////////////////////////////////////////////////////////////
void EmscriptenContext::display()
{
    waitForAnimationFrame();

    if (emscriptenFrameHook)
        emscriptenFrameHook();
}


////////////////////////////////////////////////////////////
void EmscriptenContext::setVerticalSyncEnabled(bool /* enabled */)
{
    // Always in sync: display() waits for requestAnimationFrame
}


////////////////////////////////////////////////////////////
bool EmscriptenContext::makeCurrent(bool /* current */)
{
    return true;
}

} // namespace priv
} // namespace sf
