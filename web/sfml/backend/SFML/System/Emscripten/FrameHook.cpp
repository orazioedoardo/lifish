// SFML Emscripten backend (BOOM Remake web port)
//
// Called once per frame from EmscriptenContext::display(). The audio module sets it to
// SoundStream::pumpStreams, since there are no threads to stream music on the web.
// It lives in sfml-system so that sfml-window and sfml-audio don't depend on each other.

namespace sf
{
namespace priv
{
void (*emscriptenFrameHook)() = 0;
} // namespace priv
} // namespace sf
