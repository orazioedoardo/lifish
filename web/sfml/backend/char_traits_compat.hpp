// SFML Emscripten backend (BOOM Remake web port)
//
// SFML 2.6 uses std::basic_string<Uint8/Uint16/Uint32> (sf::String, sf::Utf). Recent libc++
// (Emscripten's included) no longer provides the generic std::char_traits those need, so define
// them here. Force-included in every C++ file that uses SFML.

#ifndef SFML_EMSCRIPTEN_CHAR_TRAITS_COMPAT_HPP
#define SFML_EMSCRIPTEN_CHAR_TRAITS_COMPAT_HPP

#include <cstddef>
#include <cstring>
#include <cwchar>
#include <ios>
#include <string>

namespace sf_compat
{
template <typename CharT, typename IntT>
struct CharTraits
{
    typedef CharT          char_type;
    typedef IntT           int_type;
    typedef std::streamoff off_type;
    typedef std::streampos pos_type;
    typedef std::mbstate_t state_type;

    static constexpr void assign(char_type& a, const char_type& b) noexcept { a = b; }
    static constexpr bool eq(char_type a, char_type b) noexcept { return a == b; }
    static constexpr bool lt(char_type a, char_type b) noexcept { return a < b; }

    static constexpr int compare(const char_type* a, const char_type* b, std::size_t n)
    {
        for (; n; --n, ++a, ++b)
        {
            if (lt(*a, *b)) return -1;
            if (lt(*b, *a)) return 1;
        }
        return 0;
    }

    static constexpr std::size_t length(const char_type* s)
    {
        std::size_t n = 0;
        while (!eq(s[n], char_type())) ++n;
        return n;
    }

    static constexpr const char_type* find(const char_type* s, std::size_t n, const char_type& c)
    {
        for (; n; --n, ++s)
            if (eq(*s, c)) return s;
        return nullptr;
    }

    static char_type* move(char_type* dst, const char_type* src, std::size_t n)
    {
        return n ? static_cast<char_type*>(std::memmove(dst, src, n * sizeof(char_type))) : dst;
    }

    static char_type* copy(char_type* dst, const char_type* src, std::size_t n)
    {
        return n ? static_cast<char_type*>(std::memcpy(dst, src, n * sizeof(char_type))) : dst;
    }

    static char_type* assign(char_type* s, std::size_t n, char_type c)
    {
        for (std::size_t i = 0; i < n; ++i) s[i] = c;
        return s;
    }

    static constexpr int_type  not_eof(int_type c) noexcept { return eq_int_type(c, eof()) ? ~eof() : c; }
    static constexpr char_type to_char_type(int_type c) noexcept { return static_cast<char_type>(c); }
    static constexpr int_type  to_int_type(char_type c) noexcept { return static_cast<int_type>(c); }
    static constexpr bool      eq_int_type(int_type a, int_type b) noexcept { return a == b; }
    static constexpr int_type  eof() noexcept { return static_cast<int_type>(-1); }
};
} // namespace sf_compat

namespace std
{
template <> struct char_traits<unsigned char>  : sf_compat::CharTraits<unsigned char, unsigned int> {};
template <> struct char_traits<unsigned short> : sf_compat::CharTraits<unsigned short, unsigned int> {};
template <> struct char_traits<unsigned int>   : sf_compat::CharTraits<unsigned int, unsigned long long> {};
} // namespace std

#endif
