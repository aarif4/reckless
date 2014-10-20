#include "asynclog.hpp"
#include "ntoa.hpp"

#include <cstdio>
#include <cstring>
#include <sstream>
#include <cstdint>

namespace asynclog {
namespace {

    unsigned atou(char const*& s)
    {
        unsigned v = *s - '0';
        ++s;
        if(!isdigit(*s))
            return v;
        v = 10*v + *s - '0';
        ++s;
        while(isdigit(*s)) {
            v = 10*v + *s - '0';
            ++s;
        }
        return v;
    }

    bool isdigit(char c)
    {
        return c >= '0' && c <= '9';
    }
    
    char const* parse_conversion_specification(conversion_specification* pspec, char const* pformat)
    {
        bool left_justify = false;
        bool alternative_form = false;
        bool show_plus_sign = false;
        bool blank_sign = false;
        bool pad_with_zeroes = false;
        while(true) {
            char flag = *pformat;
            if(flag == '-')
                left_justify = true;
            else if(flag == '+')
                show_plus_sign = true;
            else if (flag == ' ')
                blank_sign = true;
            else if(flag == '#')
                alternative_form = true;
            else if(flag == '0')
                pad_with_zeroes = true;
            else
                break;
            ++pformat;
        }
        pspec->left_justify = left_justify;
        pspec->alternative_form = alternative_form;
        pspec->pad_with_zeroes = pad_with_zeroes;
        if(show_plus_sign)
            pspec->plus_sign = '+';
        else if(blank_sign)
            pspec->plus_sign = ' ';
        else
            pspec->plus_sign = 0;
        
        pspec->minimum_field_width = isdigit(*pformat)? atou(pformat) : 0;
        if(*pformat == '.') {
            ++pformat;
            pspec->precision = isdigit(*pformat)? atou(pformat) : UNSPECIFIED_PRECISION;
        } else {
            pspec->precision = UNSPECIFIED_PRECISION;
        }
        return pformat;
    }
        
    template <typename T>
    char const* generic_format_int(output_buffer* pbuffer, char const* pformat, T v)
    {
        conversion_specification spec;
        pformat = parse_conversion_specification(&spec, pformat);
        char f = *pformat;
        if(f == 'd') {
            itoa_base10(pbuffer, v, spec);
            return pformat + 1;
        } else if(f == 'x') {
            // FIXME
            return nullptr;
        } else if(f == 'b') {
            // FIXME
            return nullptr;
        } else {
            return nullptr;
        }
    }

    template <typename T>
    char const* generic_format_float(output_buffer* pbuffer, char const* pformat, T v)
    {
        char f = *pformat;
        if(f != 'd')
            return nullptr;

        ftoa_base10_precision(pbuffer, v, 6u);
        return pformat + 1;
    }

    template <typename T>
    char const* generic_format_char(output_buffer* pbuffer, char const* pformat, T v)
    {
        char f = *pformat;
        if(f == 's') {
            char* p = pbuffer->reserve(1);
            *p = static_cast<char>(v);
            pbuffer->commit(1);
            return pformat + 1;
        } else {
            return generic_format_int(pbuffer, pformat, static_cast<int>(v));
        }
    }
}   // anonymous namespace
}   // namespace dlog

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, char v)
{
    return generic_format_char(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, signed char v)
{
    return generic_format_char(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, unsigned char v)
{
    return generic_format_char(pbuffer, pformat, v);
}

//char const* asynclog::format(output_buffer* pbuffer, char const* pformat, wchar_t v);
//char const* asynclog::format(output_buffer* pbuffer, char const* pformat, char16_t v);
//char const* asynclog::format(output_buffer* pbuffer, char const* pformat, char32_t v);

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, short v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, unsigned short v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, int v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, unsigned int v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, long v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, unsigned long v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, long long v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, unsigned long long v)
{
    return generic_format_int(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, float v)
{
    return generic_format_float(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, double v)
{
    return generic_format_float(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, long double v)
{
    return generic_format_float(pbuffer, pformat, v);
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, char const* v)
{
    if(*pformat != 's')
        return nullptr;
    auto len = std::strlen(v);
    char* p = pbuffer->reserve(len);
    std::memcpy(p, v, len);
    pbuffer->commit(len);
    return pformat + 1;
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, std::string const& v)
{
    if(*pformat != 's')
        return nullptr;
    auto len = v.size();
    char* p = pbuffer->reserve(len);
    std::memcpy(p, v.data(), len);
    pbuffer->commit(len);
    return pformat + 1;
}

char const* asynclog::format(output_buffer* pbuffer, char const* pformat, void const* p)
{
    char c = *pformat;
    if(c != 'p' && c !='s')
        return nullptr;
    char* s = pbuffer->reserve(2);
    s[0] = '0';
    s[1] = 'x';
    pbuffer->commit(2);
    itoa_base16(pbuffer, reinterpret_cast<std::uintptr_t>(p), false, nullptr);
    return pformat+1;
}

void asynclog::template_formatter::append_percent(output_buffer* pbuffer)
{
    auto p = pbuffer->reserve(1u);
    *p = '%';
    pbuffer->commit(1u);
}

char const* asynclog::template_formatter::next_specifier(output_buffer* pbuffer,
        char const* pformat)
{
    while(true) {
        char const* pspecifier = std::strchr(pformat, '%');
        if(pspecifier == nullptr) {
            format(pbuffer, pformat);
            return nullptr;
        }

        auto len = pspecifier - pformat;
        auto p = pbuffer->reserve(len);
        std::memcpy(p, pformat, len);
        pbuffer->commit(len);

        pformat = pspecifier + 1;

        if(*pformat != '%')
            return pformat;

        ++pformat;
        append_percent(pbuffer);
    }
}

void asynclog::template_formatter::format(output_buffer* pbuffer, char const* pformat)
{
    auto len = std::strlen(pformat);
    char* p = pbuffer->reserve(len);
    std::memcpy(p, pformat, len);
    pbuffer->commit(len);
}

