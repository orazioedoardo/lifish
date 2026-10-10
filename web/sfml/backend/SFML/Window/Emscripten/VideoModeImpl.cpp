// SFML Emscripten backend (BOOM Remake web port)
//
// The page is the "screen": fullscreen is the browser's (Cmd+F on the page), not a video mode.

#include <SFML/Window/VideoModeImpl.hpp>
#include <emscripten.h>

namespace sf
{
namespace priv
{
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    return std::vector<VideoMode>(1, getDesktopMode());
}

VideoMode VideoModeImpl::getDesktopMode()
{
    const int width  = EM_ASM_INT({ return screen.width; });
    const int height = EM_ASM_INT({ return screen.height; });
    return VideoMode(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
}

} // namespace priv
} // namespace sf
