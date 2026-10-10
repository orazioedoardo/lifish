// SFML Emscripten backend (BOOM Remake web port)

#ifndef SFML_WINDOWIMPLEMSCRIPTEN_HPP
#define SFML_WINDOWIMPLEMSCRIPTEN_HPP

#include <SFML/Window/WindowImpl.hpp>

namespace sf
{
namespace priv
{
class WindowImplEmscripten : public WindowImpl
{
public:
    WindowImplEmscripten(WindowHandle handle);
    WindowImplEmscripten(VideoMode mode, const String& title, unsigned long style, const ContextSettings& settings);
    ~WindowImplEmscripten();

    virtual WindowHandle getSystemHandle() const;
    virtual Vector2i getPosition() const;
    virtual void setPosition(const Vector2i& position);
    virtual Vector2u getSize() const;
    virtual void setSize(const Vector2u& size);
    virtual void setTitle(const String& title);
    virtual void setIcon(unsigned int width, unsigned int height, const Uint8* pixels);
    virtual void setVisible(bool visible);
    virtual void setMouseCursorVisible(bool visible);
    virtual void setMouseCursorGrabbed(bool grabbed);
    virtual void setMouseCursor(const CursorImpl& cursor);
    virtual void setKeyRepeatEnabled(bool enabled);
    virtual void requestFocus();
    virtual bool hasFocus() const;

protected:
    virtual void processEvents();

private:
    bool m_keyRepeat;
    bool m_hasFocus;
};

} // namespace priv
} // namespace sf

#endif
