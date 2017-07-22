#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "Windows_Print.hpp"

/* Windows Print 
// Windows_Print.cpp
//
// Author: Jan Ringos, http://Tringi.TrimCore.cz, jan@ringos.cz
// Version: 1.9
//
// Changelog:
//      18.05.2016 - initial version
*/

#include <cstdio>
#include <cmath>
#include <cwchar>
#include <cwctype>
#include <cstring>
#include <algorithm>

#include "../Resources/Resources_String.hpp"
#include "../Resources/Resources_VersionInfo.hpp"
#include "../Resources/Resources_ErrorMessage.hpp"

static_assert (sizeof (wchar_t) == sizeof (char16_t), "_WIN32/64 ABI required");
static_assert (sizeof (int) == sizeof (long), "LLP64 ABI required");
#ifdef __GNUC__
static_assert (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__, "Little endian platform required");
#else
static_assert (REG_DWORD == REG_DWORD_LITTLE_ENDIAN, "Little endian platform required");
#endif
#ifndef UNICODE
#error "Windows::Print use without UNICODE defined is not supported!"
#endif

namespace ext {
    template <typename T, typename U, unsigned long long N>
    bool exists_in (T (&arr) [N], U c) {
        
        for (auto a : arr) {
            if (a == c)
                return true;
        };
        return false;
    };
};

namespace {
    class Input {
        protected:
            const wchar_t *         format;
            const wchar_t * const   end;
            std::va_list            va;
            wchar_t                 temporary_format [48]; // for cleaned string

            struct Conversion {
                public:
                    const wchar_t * begin;
                    const wchar_t * end;
                    const wchar_t * spec;
                    const wchar_t * spec_end;

                    wchar_t         letter;
                    unsigned short  stars;
                    int             star [2]; // max supported stars now
                
                public:
                    Conversion (const wchar_t * begin = nullptr, const wchar_t * end = nullptr,
                                const wchar_t * sbegin = nullptr, const wchar_t * send = nullptr)
                        :   begin (begin), end (end),
                            spec (sbegin), spec_end (send),
                            letter (end ? *(end - 1) : L'\0'),
                            stars (0) {};
                                    
                    Conversion (const Conversion &) = default;
                
                public:
                    std::size_t  count (wchar_t);
                    bool         contains (wchar_t);// __attribute__((pure));
                    
                    template <unsigned N>
                    bool         any_of (const wchar_t (&) [N]);
                    
                    template <unsigned N>
                    bool         contains (const wchar_t (&) [N]);
                    
                    int          initstars (std::va_list &);
                    int          precision (int = 6);
                    int          width (int = 0);
                    unsigned int resource_string_base (unsigned int = 0u);
            } conversion;

        public:
            explicit Input (unsigned int, std::va_list, unsigned int = 0u);
            explicit Input (const wchar_t *, std::va_list, unsigned int = 0u);
        public:
            Input (const Input &) = default;
            
            enum CleanMode : unsigned int {
                NormalClean = 0,
                RemoveBigLongClean = 1,
                ChangeLongLongToI64Clean = 2,
                ForceNumbersToStarsClean = 4,
            };
            
            const wchar_t *   next () const;
            static Conversion BreakConversion (const wchar_t * format);
            wchar_t *         CleanConversion (unsigned int = NormalClean);
            const wchar_t *   GetCurrentFormatPointer () const { return this->format; };
    };
    
    class Memory {
        private:
            HANDLE      heap;
            void *      data;
            std::size_t size; // HeapSize?
        public:
            Memory ()
                : heap (GetProcessHeap ()),
                  data (nullptr),
                  size (0u) {};
            ~Memory () {
                if (this->size) {
                    HeapFree (this->heap, 0, this->data);
                };
            };
            
            bool Resize (std::size_t n) {
                if (this->size) {
                    if (auto p = HeapReAlloc (this->heap, 0, this->data, n)) {
                        this->data = p;
                        this->size = n;
                        return true;
                    };
                } else {
                    if ((this->data = HeapAlloc (this->heap, 0, n))) {
                        this->size = n;
                        return true;
                    };
                };

                // GetLastError returns ERROR_NOT_ENOUGH_MEMORY at this point
                // but it's documented NOT to, so better be safe and set it
                
                SetLastError (ERROR_NOT_ENOUGH_MEMORY);
                return false;
            };
            
            bool Ensure (std::size_t n) {
                if (n > this->size) {
                    return this->Resize (n);
                } else
                    return true;
            };
            
            std::size_t Size () const { return this->size; };
            unsigned char * Data () const { return static_cast <unsigned char *> (this->data); };
            
        private:
            Memory (const Memory &) = delete;
            Memory & operator = (const Memory &) = delete;
    };
    
    class Output {
        public:
            std::size_t n_out = 0;
        public:
            virtual ~Output () = 0;
            virtual bool String (const wchar_t *, std::size_t) = 0;
            virtual bool Repeat (wchar_t, std::size_t);
            virtual bool IsConsole (CONSOLE_SCREEN_BUFFER_INFO *);
            virtual void MoveCursor (unsigned short, unsigned short);
    };
    
    Output::~Output () { };
    
    class OutputConsole : public Output {
        private:
            HANDLE handle;
        public:
            OutputConsole (HANDLE h)
                :   handle (h) {};
            
            virtual bool String (const wchar_t *, std::size_t) override;
            virtual bool IsConsole (CONSOLE_SCREEN_BUFFER_INFO *) override;
            virtual void MoveCursor (unsigned short, unsigned short) override;
    };
    
    class OutputHandle : public Output {
        private:
            Memory memory;
            HANDLE handle;
            UINT   codepage;
            
        public:
            OutputHandle (HANDLE h, UINT cp)
                :   handle (h),
                    codepage (cp) {};

            virtual bool String (const wchar_t *, std::size_t) override;
    };
    
    class OutputBuffer : public Output {
        private:
            unsigned char * buffer;
            std::size_t     size;
            unsigned int    codepage;
            
        public:
            OutputBuffer (void * buffer, std::size_t size, unsigned int cp)
                :   buffer (static_cast <unsigned char *> (buffer)),
                    size (NulTerminatorSpace (cp, size)),
                    codepage (cp) {};
            
            virtual ~OutputBuffer () override {
                std::memset (this->buffer, 0, NulTerminatorLength (codepage));
            };
            
            virtual bool String (const wchar_t *, std::size_t) override;

        public:
            static std::size_t NulTerminatorSpace (int cp, std::size_t offset);
            static std::size_t NulTerminatorLength (int cp);
    };
    
    
    Input::Input (unsigned int id, std::va_list va, unsigned int length)
        :   format (Resources::String (id, &length)),
            end (format + length),
            va (va) {
        
        this->temporary_format [0] = L'%';
        return;
    };
            
    Input::Input (const wchar_t * string, std::va_list va, unsigned int length)
        :   format (reinterpret_cast <unsigned long long> (string) < 0x10000uLL
                    ? Resources::String ((unsigned int) reinterpret_cast <unsigned long long> (string), &length)
                    : string),
            end (reinterpret_cast <unsigned long long> (string) < 0x10000uLL
                    ? string + length
                    : string + std::wcslen (string)),
            va (va) {
            
        this->temporary_format [0] = L'%';
        return;
    };
    
    class Encoder {
        private:
            const wchar_t * const string;
            std::size_t     const length;
            unsigned int    const codepage;
        public:
            std::size_t   mutable encoded;
            
        public:
            Encoder (unsigned int cp, const wchar_t * string, std::size_t length)
                : string (string), length (length), codepage (cp), encoded (0u) {};
            
            bool Encode (unsigned char *& buffer, std::size_t & max) const;
            std::size_t Size () const; // in bytes
    };
    
    // Remaining letters that can be used as modifiers are:
    //  - H, J, N, O, R, w, W

    static const char proprietary_modifiers [] = { 
        'B', // byte (format of integers, pointers, ...)
        'b', // byte (format of pointers, ...)
        'k', // kernel error code (hexa lowercase) 
        'K', // kernel error code (hexa uppercase) 
        'r', // loaded from resources
        'R', // 
        'U', // Unicode (symbols) or User
        'v', // version info of current module  
        'V', // version info 
        'Q', // formatted for visual quality
        '?', // 
        '$', // localized currency
    };
    static const char standard_modifiers [] = { 
        'h', // standard - half precision (or hh - byte)
        'I', // windows - as in 'I64'
        'j', // standard - intmax_t
        'l', // standard - long (or ll - long long)
        'L', // standard - long long (long double)
        'q', // standard - long long
        't', // standard - ptrdiff_t
        'z', // standard - size_t
    };
    
    const wchar_t * find_closing_bracket (const wchar_t * p) {
        auto nesting = 0u;
        
        while (true) {
            switch (*p) {
                case L'\0':
                    return nullptr;
                case L'[':
                    ++nesting;
                    break;
                case L']':
                    if (!nesting--)
                        return p;
            };
            ++p;
        };
    };

    Input::Conversion Input::BreakConversion (const wchar_t * format) {
        const wchar_t * p = format;
        const wchar_t * spec = nullptr;
        const wchar_t * send = nullptr;

        while (true) {
            auto c = *p++;

            // ASCII letter terminates convention
            //  - unless it's one of modifiers listed below

            if ((c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z')) {

                if (ext::exists_in (standard_modifiers, c))
                    continue;

                if (ext::exists_in (proprietary_modifiers, c))
                    continue;

                return Conversion (format, p, spec, send);
            }

            switch (c) {
                
                // end of format string
                //  - invalid convention, should be ignored
                
                case L'%':
                case L'\0':
                    return Conversion (format, p);
                
                // specification string
                //  - PROPRIETARY extension
                //  - searching for MATCHING bracket!
                
                case L'[':
                    spec = p;
                    if ((send = find_closing_bracket (spec)) != nullptr) {
                        p = send;
                        continue;
                        
                    } else
                        return Conversion (format, p);
            };
        };
    };

    bool IsDigit (wchar_t c) {
        return c >= L'0' && c <= L'9';
    };

    wchar_t * Input::CleanConversion (unsigned int mode) {
        const auto tmplen = sizeof this->temporary_format / sizeof this->temporary_format [0];
        
        // initial char is always '%' character, starting after it
        // and leaving 4 characters left for optionally adding "*.*" at the end
        
        wchar_t * out = &this->temporary_format [1];
        wchar_t * end = &this->temporary_format [tmplen - 5u];
        auto stars = 0u;
        
        struct {
            const wchar_t * begin;
            const wchar_t * end;
        } range [2] = {
            { this->conversion.begin, this->conversion.spec - 1 },
            { this->conversion.spec_end + 1, this->conversion.end }
        };
        
        if (this->conversion.spec == nullptr) {
            range [0] .end = this->conversion.end;
            range [1] .begin = this->conversion.end;
        };
        
        for (auto r = 0u; r != sizeof range / sizeof range [0]; ++r) {
            auto in = range [r] .begin;
            auto ie = range [r] .end;
            
            // remove all proprietary modifiers and characters
            
            while ((in != ie) && (out != end)) {
                if (ext::exists_in (proprietary_modifiers, *in)) {
                    ++in;
                } else {
                    if (mode & RemoveBigLongClean) {
                        if (*in == L'L') {
                            ++in;
                            continue;
                        };
                    };
                    if (mode & ChangeLongLongToI64Clean) {
                        if ((in[0] == L'l') && (in[1] == L'l')) {
                            *out++ = L'I';
                            *out++ = L'6';
                            *out++ = L'4';
                            in += 2;
                            continue;
                        };
                    };
                    if (mode & ForceNumbersToStarsClean) {
                        if (*in == L'*') {
                            ++stars;
                        } else
                        if (IsDigit (*in)) {
                            if (out == &this->temporary_format [1] || out [-1] != L'*') {
                                *out++ = L'*';
                                ++stars;
                            };
                            ++in;
                            continue;
                        };
                    };
                    
                    *out++ = *in++;
                };
            };
        };
        
        *out = L'\0';

        if (mode & ForceNumbersToStarsClean) {
            switch (stars) {
                case 0u:
                    std::memmove (&this->temporary_format [4],
                                  &this->temporary_format [1],
                                  sizeof (wchar_t) * (out - this->temporary_format));
//                                  sizeof (wchar_t) * (tmplen - 5u));
                    this->temporary_format [1] = L'*';
                    this->temporary_format [2] = L'.';
                    this->temporary_format [3] = L'*';
                    break;
                case 1u:
                    if (auto pdot = std::wcschr (&this->temporary_format [1], L'.')) {
                        
                        // found ".*" (no other situation can happen)
                        //  - shift all 1 character right and prepend '*'

                        std::memmove (pdot + 1, pdot, sizeof (wchar_t) * (out - pdot + 1));
                        *pdot = L'*';
                        
                    } else
                    if (auto pstar = std::wcschr (&this->temporary_format [1], L'*')) {
                        
                        // prepend "*." to existing '*'
                        
                        std::memmove (pstar + 2, pstar, sizeof (wchar_t) * (out - pstar + 1));
                        pstar [0] = L'*';
                        pstar [1] = L'.';
                    };
                    break;
            };
        };

        return this->temporary_format;
    };

    class Printer : public Input {
        public:
            Printer (Input input, Output & output)
                : Input (input),
                  output (output) {};
                
            void operator () ();
        
        private:
            Memory   memory;
            Output & output;
            
        private:

            // passing output
            
            bool String (const wchar_t *, std::size_t);
            bool Forward (const wchar_t *, const wchar_t *);
            bool Forward (wchar_t c) { return this->Repeat (c, 1u); };
            bool Repeat (wchar_t character, std::size_t n);
            
            // rendering
            
            bool Character ();
            bool String ();
            bool DecimalInteger ();
            bool NonDecimalInteger ();
            bool FloatingPoint ();
            bool ZeroTermination (bool & terminate);
            bool DateTime ();
            bool BinaryData ();
            bool IPAddress ();
            
            // common rendering
            
            bool CharacterString (UINT, const void *, const void *);

            
            // data prepare
            
            bool LoadStringCodePageParameter (UINT &);
            bool LoadLocalizedConversionParameters (LCID &, DWORD &);
            std::intmax_t LoadSignedInteger ();
            std::uintmax_t LoadUnsignedInteger ();
            bool LoadSpecStringNumber (unsigned int &);

            template <typename T>
            void ApplyValueLowercaseHCrop (T &);

            template <typename T>
            bool LoadVersionInfoInteger (T &);
            
            unsigned long long Float16ToFloat64 (unsigned short);
            wchar_t ErrorCodeHexadecimal (std::intptr_t, bool);
            
            // special rendering
            
            bool LocalNumber (const wchar_t *, LCID, DWORD flags, int precision, bool neg);
            bool LocalCurrency (const wchar_t *, LCID, DWORD flags, int precision, bool neg);
            
            // defaults
            
            template <typename T, typename... Params>
            bool DefaultOperation (const wchar_t *, T value, Params... p);

        public:
            bool        success = true;
            std::size_t n_ops   = 0u;
            
    };

    const wchar_t * Input::next () const {
        auto p = this->format;
        while (p != this->end && *p != L'%') {
            ++p;
        };
        if (p && *p == L'%')
            return p;
        else
            return nullptr;
    };

    void Printer::operator () () {
        while (auto p = this->next ()) {
            
            this->success &= this->Forward (this->format, p + (p[1] == L'%'));
            this->format = p + 1;
            
            this->conversion = this->BreakConversion (this->format);
            switch (this->conversion.letter) {
                
                case L'c':
                    this->success &= this->Character ();
                    break;
                
                case L's':
                    this->success &= this->String ();
                    break;
                
                case L'u': case L'i': case L'd':
                    this->success &= this->DecimalInteger ();
                    break;

                case L'o': case L'x': case L'X':
                    this->success &= this->NonDecimalInteger ();
                    break;
                    
                case L'f': case L'F':
                case L'e': case L'E':
                case L'g': case L'G':
                case L'a': case L'A':
                    this->success &= this->FloatingPoint ();
                    break;

                case L'Z': {
                    bool terminate = false;
                    this->success &= this->ZeroTermination (terminate);
                    if (terminate)
                        return;
                } break;
                
                case L'C': // color [irgb:irgb] color on color, numbers
                case L'W': // window
//                case L'M':
//                    this->ConsoleModeOperation (convention);
                    break;
                    
                case L'D':
                case L'T':
                    this->success &= this->DateTime ();
                    break;
                
                case L'p':
                    if (this->conversion.contains (L'B') || this->conversion.contains (L'b')) {
                        this->success &= this->BinaryData ();
                    } else {
                        this->success &= this->DefaultOperation (this->CleanConversion (),
                                                                 va_arg (this->va, void *));
                    };
                        
                    break;
                case L'P':
                    if (this->conversion.contains (L'I')) {
                        this->success &= this->IPAddress ();
                    } else {
//                        this->success &= this->DefaultOperation (this->CleanConversion (),
//                                                                 va_arg (this->va, void *));
                    };
                    break;
//                case L'G': // ??
//                    TODO: GUID
//                    break;

                case L'%':
                    break;
            };
            
            if (!this->success)
                return;

            this->format = conversion.end;
            this->n_ops++;
        };
        
        // displaying rest of the string
        this->success &= this->Forward (this->format, this->end);
        return;
    };

    template <typename T>
    inline T Swap16 (T v) {
#ifdef __GCC__
        return __builtin_bswap16 (v);
#else
        return ((v & 0x00ff) << 8)
             | ((v & 0xff00) >> 8);
#endif
    };
    
    template <typename T>
    inline T Swap24 (T v) {
        std::swap <unsigned char> (reinterpret_cast <unsigned char *> (&v) [0],
                                   reinterpret_cast <unsigned char *> (&v) [2]);
        return v;
//        return ((v & 0x0000ff) << 16)
//             | ((v & 0x00ff00))
//             | ((v & 0xff0000) >> 16);
    };
    
    template <typename T>
    inline T Swap32 (T v) {
#ifdef __GCC__
        return __builtin_bswap32 (v);
#else
        return ((v & 0x000000ff) << 24)
             | ((v & 0x0000ff00) << 8)
             | ((v & 0x00ff0000) >> 8)
             | ((v & 0xff000000) >> 24);
#endif
    };
    
    bool Printer::Character () {

        // additional star arguments
        
        conversion.initstars (this->va);
        
        // codepage argument
        
        UINT codepage = 0u;
        this->LoadStringCodePageParameter (codepage);
        
        // character value
        //  - relying on little endian architecture here
        //  - converting into string
        
        unsigned int c[2] = { va_arg (this->va, unsigned int), 0u };
        
        // EXTENSION: ? = zero prints nothing
        
        if (conversion.contains (L'?')) {
            if (c[0] == 0) {
                return true;
            };
        };
        
        // codepage-specific
        
        switch (codepage) {

            // drop not-a-characters for UTF-16 and UTF-32
        
            case CP_UTF16:
            case CP_UTF16_BE:
            case CP_UTF32:
                if (c[0] == 0xFFFFu || c[0] == 0xFEFFu || c[0] == 0xFFFEu)
                    return true;
                
                break;

            case CP_UTF32_BE:
                if (c[0] == 0xFFFF0000u || c[0] == 0xFEFF0000u || c[0] == 0xFFFE0000u)
                    return true;
                
                break;
            
            // reverse bytes for multibyte character constants
            //  - TODO: MSVC?
            
            default:
                if (c[0] > 0xFFFFFFu) {
                    c[0] = Swap32 (c[0]);
                } else
                if (c[0] > 0xFFFFu) {
                    c[0] = Swap24 (c[0]);
                } else
                if (c[0] > 0xFFu) {
                    c[0] = Swap16 (c[0]);
                };
        };

        // process as string

        return this->CharacterString (codepage, c, nullptr);
    };

    bool IsBlank (wchar_t c) {
        return c == L' '
            || c == L'\t'
            || c == L'\r'
            || c == L'\n'
            ;
    };
    
    wchar_t Printer::ErrorCodeHexadecimal (std::intptr_t value, bool signedvalue) {
        if ((value >= 100000) || (signedvalue && value < -61440)) {
            
            if (conversion.contains (L'K')) {
                return L'X';
            } else {
                return L'x';
            };
        } else
            return L'\0';
    };

    bool Printer::String () {
        
        // additional star arguments
        
        conversion.initstars (this->va);
        
        // additional arguments
        
        UINT codepage = 0u;
        UINT language = 0u;
        HMODULE module = GetModuleHandle (NULL);
        
        if (conversion.contains (L'V') || conversion.contains (L'R')) {
            
            // HMODULE is always the first
            
            module = va_arg (this->va, HMODULE);
            
        } else
        if (!conversion.contains (L'v') && !conversion.contains (L'r')
         && !conversion.contains (L'k') && !conversion.contains (L'K')) {
            
            // for normal string pointers, convert spec string to codepage
            
            this->LoadStringCodePageParameter (codepage);
        };
        
        // language
        //  - used for resource ID strings
        //  - count of 1 since user might be tempted to write "LL" to request
        //    current language the same as for numeric/currency %f notation

        if (conversion.count (L'L') == 1u) {
            language = va_arg (this->va, unsigned int);
        };
        
        // load the actual parameter
        //  - 'v' and 'V' need to support int/string provided in spec string
        
        const void *    parameter = nullptr;
        wchar_t         vstringname [20];

        wchar_t         altnumber [16];
        unsigned int    altprepare = 0;

        const wchar_t * alternative = nullptr;
        const wchar_t * alternative_end = nullptr;
        
        if ((conversion.contains (L'v') || conversion.contains (L'V'))
                && this->conversion.spec != nullptr
                && this->conversion.spec [0] != L'*') {
            
            unsigned int id = 0;
            if (IsDigit (*this->conversion.spec) && this->LoadSpecStringNumber (id)) {
                parameter = static_cast <const char *> (0) + id;
            } else {
                auto vo = &vstringname [0];
                if (std::size_t (this->conversion.spec_end - this->conversion.spec) < (sizeof vstringname / sizeof vstringname [0])) {
                    for (auto p = this->conversion.spec; p != this->conversion.spec_end; ) {
                        *vo++ = *p++;
                    };
                };
                *vo = L'\0';

                parameter = vstringname;
            };

        } else {
            
            // for r/R/k/K get alternative string
            //  - if spec == [*] fetch another parameter
            //  - if fetched value is number, get rsrc, otherwise use the ptr
            //  - if spec == [#] render number
            
            if (this->conversion.spec != nullptr
                && (conversion.contains (L'r') ||
                    conversion.contains (L'k') ||
                    conversion.contains (L'R') ||
                    conversion.contains (L'K'))) {
                
                if (std::wcsncmp (this->conversion.spec, L"*]", 2) == 0) {
                    
                    auto ap = va_arg (this->va, const wchar_t *);
                    
                    std::size_t aid = reinterpret_cast <const char *> (ap) - static_cast <const char *> (0);
                    if (aid < 65536u) {
                        
                        // load string if alternative parameter is short integer
                        
                        auto length = 0u;
                        if (auto astr = Resources::String (module, (unsigned int) aid, &length, language)) {
                            alternative = astr;
                            alternative_end = astr + length;
                        };
                    } else {
                        
                        // use the pointer if alternative parameter is pointer
                        
                        alternative = ap;
                    };
                    
                } else
                if (std::wcsncmp (this->conversion.spec, L"#]", 2) == 0) {
                    altprepare = 1;
                    alternative = altnumber;
                } else
                if (std::wcsncmp (this->conversion.spec, L"#x]", 3) == 0) {
                    altprepare = 2;
                    alternative = altnumber;
                } else
                if (std::wcsncmp (this->conversion.spec, L"#X]", 3) == 0) {
                    altprepare = 3;
                    alternative = altnumber;
                    
                } else {
                    
                    // otherwise just use the string
                    
                    alternative = this->conversion.spec;
                    alternative_end = this->conversion.spec_end;
                };
            };
            
            
            // finally the primary string pointer/id last
            
            parameter = va_arg (this->va, const void *);
        };
        
        // everything is loaded as a pointer, get the ID

        std::size_t string_id = static_cast <const char *> (parameter) - static_cast <const char *> (0);

        // EXTENSION: ? = zero prints nothing
        
        if (conversion.contains (L'?')) {
            if (parameter == nullptr) {
                return true;
            };
        };
        
        // prepare alternative string number
        
        switch (altprepare) {
            case 1: _snwprintf (altnumber, sizeof altnumber / sizeof altnumber [0], L"#%u", (unsigned int) string_id); break;
            case 2: _snwprintf (altnumber, sizeof altnumber / sizeof altnumber [0], L"#%x", (unsigned int) string_id); break;
            case 3: _snwprintf (altnumber, sizeof altnumber / sizeof altnumber [0], L"#%X", (unsigned int) string_id); break;
        };
        
        // print
        
        if (conversion.contains (L'v') || conversion.contains (L'V')) {
            
            Resources::VersionInfo versioninfo (module, language);
            
            if (string_id < 65536u) {
                return this->CharacterString (CP_UTF16, versioninfo [(unsigned int) string_id], nullptr);
            } else {
                return this->CharacterString (CP_UTF16, versioninfo [static_cast <const wchar_t *> (parameter)], nullptr);
            };
            
        } else
        if (conversion.contains (L'k') || conversion.contains (L'K')) {
            
            if (conversion.contains (L'#')) {
                
                wchar_t fmt [6];
                if (auto c = this->ErrorCodeHexadecimal (string_id, true)) {
                    _snwprintf (fmt, sizeof fmt / sizeof fmt [0], L"0x%%%c", c);

                    if (!this->String (L"0x", 2u))
                        return false;
                } else {
                    fmt [0] = L'%';
                    fmt [1] = L'd';
                    fmt [2] = L'\0';
                };

                wchar_t buffer [6];
                auto length = _snwprintf (buffer, sizeof buffer / sizeof buffer [0], fmt, string_id);
                if (!this->String (buffer, length))
                    return false;
                
                if (!this->String (L": ", 2u))
                    return false;
            };
            
            const auto length = 512u; // ???
            
            if (this->memory.Ensure (sizeof (wchar_t) * length)) {
                auto output = reinterpret_cast <wchar_t *> (this->memory.Data ());
                if (auto n = Resources::ErrorMessage (output, length, (unsigned int) string_id, language)) {
                    
                    // rtrim
                    
                    while (n && IsBlank (output [n - 1])) {
                        --n;
                    };
                    
                    // output

                    return this->CharacterString (CP_UTF16, output, output + n);
                    
                } else {
                    
                    // display alternative, if any
                    
                    return this->CharacterString (CP_UTF16, alternative, alternative_end);
                };
            } else
                return false;
            
        } else
        if (conversion.contains (L'r') || conversion.contains (L'R') || string_id < 65536u) {
            
            auto length = 0u;
            auto string_ptr = Resources::String (module,
                                                 conversion.resource_string_base () + (unsigned int) string_id,
                                                 &length, language);
            if (string_ptr) {
                return this->CharacterString (CP_UTF16, string_ptr, string_ptr + length);
            } else {
                return this->CharacterString (CP_UTF16, alternative, alternative_end);
            };
            
        } else {
            
            // TODO: if LoadStringCodePageParameter failed, try detect and skip BOM here
            
            return this->CharacterString (codepage, parameter, nullptr);
        };
    };
    
    template <typename T>
    const T * FindNulTerminator (const void * input) {
        auto p = static_cast <const T *> (input);
        while (*p) {
            ++p;
        };
        return p;
    };
    
    bool Printer::CharacterString (UINT codepage,
                                   const void * input, const void * end) {

        // for simplicity this check is ommited everywhere else

        if (!input)
            return true;

        // end is nullptr for NUL-terminated strings of unknown charset
        
        if (!end) {
            switch (codepage) {
                default:
                    end = FindNulTerminator <char> (input);
                    break;
                
                case CP_UTF16:
                case CP_UTF16_BE:
                    end = FindNulTerminator <wchar_t> (input);
                    break;
                
                case CP_UTF32:
                case CP_UTF32_BE:
                    end = FindNulTerminator <unsigned int> (input);
                    break;
            };
        };
        
        // empty string means we are done here
        
        if (end == input)
            return true;
        
        // decode
        
        const wchar_t * string = nullptr;
        std::size_t length = 0u;
        
        switch (codepage) {
            
            default:
                if ((length = MultiByteToWideChar (codepage, 0u,
                                                   static_cast <const char *> (input),
                                                   (int) (static_cast <const char *> (end) - static_cast <const char *> (input)),
                                                   nullptr, 0))) {
                    
                    if (this->memory.Ensure (sizeof (wchar_t) * length)) {
                        auto output = reinterpret_cast <wchar_t *> (this->memory.Data ());
                        
                        if (MultiByteToWideChar (codepage, 0u,
                                                 static_cast <const char *> (input),
                                                 (int) (static_cast <const char *> (end) - static_cast <const char *> (input)),
                                                 output, (int) length)) {
                            
                            string = output;
                        };
                    };
                };
                break;
            
//            case CP_ID5:
//                break;
            
            case CP_UTF16:
                // this is native string, no conversion
                string = static_cast <const wchar_t *> (input);
                length = static_cast <const wchar_t *> (end) - string;
                break;
                
            case CP_UTF16_BE:
            case CP_UTF32:
            case CP_UTF32_BE:
                
                switch (codepage) {
                    case CP_UTF16_BE:
                        length = static_cast <const wchar_t *> (end) - static_cast <const wchar_t *> (input);
                        break;
                    case CP_UTF32:
                    case CP_UTF32_BE:
                        length = static_cast <const char32_t *> (end) - static_cast <const char32_t *> (input);
                        break;
                };

                // detect supplementary plane characters
                //  - need to be converted to surrogate pairs
                
                const auto inlen = length;
                
                switch (codepage) {
                    case CP_UTF32:
                        for (std::size_t i = 0u; i != inlen; ++i) {
                            if (static_cast <const char32_t *> (input) [i] >= 65536u) {
                                ++length;
                            };
                        };
                        break;
                        
                    case CP_UTF32_BE:
                        for (std::size_t i = 0u; i != inlen; ++i) {
                            if (Swap32 (static_cast <const char32_t *> (input) [i]) >= 65536u) {
                                ++length;
                            };
                        };
                        break;
                };
                
                // allocate resulting string in scratch memory
                
                if (this->memory.Ensure (sizeof (wchar_t) * length)) {
                    auto output = reinterpret_cast <wchar_t *> (this->memory.Data ());
                    
                    // generate native string
                    //  - swap bytes for BE UTFs
                    //  - split to surrogate pairs for 32-bit UTFs

                    for (std::size_t i = 0u, o = 0u; (i != inlen) && (o != length); ++i) {
                        
                        char32_t c = 0;
                        switch (codepage) {
                            case CP_UTF16_BE:
                                c = Swap16 (static_cast <const wchar_t *> (input) [i]);
                                break;
                            case CP_UTF32_BE:
                                c = Swap32 (static_cast <const char32_t *> (input) [i]);
                                break;
                            case CP_UTF32:
                                c = static_cast <const char32_t *> (input) [i];
                                break;
                        };

                        if (c >= 65536u) {
                            if (c < 0x110000u) {

                                // high surrogate pair
                                output [o++] = 0xD800 | (wchar_t) ((c - 0x10000u) >> 10u);
                                // low surrogate pair
                                output [o++] = 0xDC00 | (wchar_t) ((c - 0x10000u) & 0x03FFu);
                                
                            } else {
                                
                                // outside supplementary planes
                                //  - render as not-a-character
                                
                                output [o++] = 0xFFFD;
                                --length;
                            };
                        } else {
                            output [o++] = (wchar_t) c;
                        };
                    };
                    
                    // done
                    string = output;
                };
                break;
        };
        
        // render string/length
        
        if (string) {

            auto elipsis = L'\0';
            auto elipsis_length = 0u;
            
            // if precision entry is provided, it's used as char count limit
            
            std::size_t maxlen = conversion.precision (-1);
            if (maxlen != std::size_t (-1)) {
                
                // count surrogate pair as single character for string length purposes
                //  - OPTIMIZE: in some cases can be computed above already
            
                for (std::size_t i = 0u; i != length; ++i) {
                    if (string [i] >= 0xD800 && string [i] < 0xDC00) {
                        ++maxlen;
                    };
                };
                
                // apply limit
                
                if (length > maxlen) {
                    length = maxlen;
                    
                    // three dots ... request truncation and appending ...
                    //  - TODO: attempt word truncation, search space?
                    //          if there is no space, search for spacing unicodes
                    
                    if (conversion.contains (L"...")) {
                        if (conversion.contains (L'U')) {
                            if (length > 2) {
                                elipsis = L'\x2026';
                                elipsis_length = 1;
                                length = maxlen - elipsis_length;
                            };
                        } else {
                            if (length > 2) {
                                elipsis = L'.';
                                elipsis_length = 3;
                                length = maxlen - elipsis_length;
                            };
                        };
                    };
                };
            };
            
            auto width = conversion.width (0);
            if (conversion.contains (L'-')) {
                width = -width;
            };
            
            // when width is positive and longer than length
            // then prefix with enough spaces
            
            if (width > 0 && width > length) {
                if (!this->Repeat (L' ', width - length))
                    return false;
            };
            
            if (this->String (string, length)) {
                
                // append elipsis, if any
                
                if (!this->Repeat (elipsis, elipsis_length))
                    return false;
            
                // when width is negative and longer than length
                // then append enough spaces
            
                if (width < 0 && -width > length) {
                    if (!this->Repeat (L' ', -width - length))
                        return false;
                };
                
                return true;
            };
        };
        
        return false;
    };
    
    wchar_t * WcsNChr (wchar_t * s, std::size_t n, wchar_t c) {
        if (s) {
            while (n--) {
                if (*s == c)
                    return s;
                else
                    ++s;
            };
        };
        
        return nullptr;
    };

    const wchar_t * WcsNChr (const wchar_t * s, std::size_t n, wchar_t c) {
        if (s) {
            while (n--) {
                if (*s == c)
                    return s;
                else
                    ++s;
            };
        };
        
        return nullptr;
    };
    
    const wchar_t * GetBytesLocalizedString (unsigned int * length, LCID language) {
        if (language == LOCALE_INVARIANT)
            return nullptr;
        
        if (auto hShell32 = GetModuleHandleW (L"SHELL32")) {
            return Resources::String (hShell32, 4113, length, (unsigned short) language);
        } else {
            return nullptr;
        };
    };
    
    bool Printer::DecimalInteger () {

        // additional star arguments
        
        conversion.initstars (this->va);
        
        // for L$ and LL, eat another argument for 'language' (if present)
        //  - language/flags are for GetCurrencyFormat/GetNumberFormat

        DWORD flags = 0;
        LCID language = LOCALE_USER_DEFAULT; // Resources::GetPreferredLanguage () ???
        
        this->LoadLocalizedConversionParameters (language, flags);
        
        // value
        //  - loading below relies on little endian architecture
        
        std::intmax_t value;
        bool signedvalue;
        
        if (this->LoadVersionInfoInteger (value)) {
            signedvalue = false;
            
        } else
        switch (conversion.letter) {
            case L'd':
            case L'i':
                value = this->LoadSignedInteger ();
                signedvalue = true;
                break;
                
            case L'u':
            default:
                value = this->LoadUnsignedInteger ();
                signedvalue = false;
                break;
        };

        // EXTENSION: restrict size: h (short), hh (char), hhh (nibble), hhhh (bool)
        
        this->ApplyValueLowercaseHCrop (value);

        // EXTENSION: ? = zero prints nothing
        
        if (conversion.contains (L'?')) {
            if (value == 0) {
                return true;
            };
        };
        
        // EXTENSION: $ = displays monetary value

        if (conversion.contains (L'$')) {
            wchar_t number [22];
            if (_snwprintf (number, sizeof number / sizeof number [0],
                            signedvalue ? L"%I64d" : L"%I64u", value)) {

                // precision of 2 is typical for currency
                auto precision = std::abs (conversion.precision (2));

                return this->LocalCurrency (number, language, flags, precision,
                                            signedvalue ? (value < 0) : false);
            };
        };
        
        // EXTENSION: B = size information

        if (conversion.contains (L'B')) {
            
            // value unsigned 64-bit tops at 16 EB - 1
            
            static const char prefix [] = { 'k', 'M', 'G', 'T', 'P', 'E' };
            static const char c0x20 = ' ';
            static const char cB = 'B';
            static const char cK = 'K';
            static const char ci = 'i';
            
            // Example, for Czech locale
            //       0 ...     921 -  921 bajtù
            //    1000 ...   10239 -  0,9 kB ...  9,9 kB
            //   10240 ... 1023999 -   10 kB ...  999 kB
            // 1024000 ... 1048575 -  0,9 MB ...  1,0 MB
            
            if (((signedvalue && (std::abs (value) < 922))
             || (!signedvalue && (((std::uintmax_t) value) < 922uLL)))) {

                // Attempt to use "%s bytes" (bytes) string
                
                auto byteslength = 0u;
                auto bytesformat = GetBytesLocalizedString (&byteslength, language);
                auto bytesplaceholder = WcsNChr (bytesformat, byteslength, L'%');
                
                // First part of the string, if any
                
                if (bytesplaceholder && bytesplaceholder != bytesformat) {
                    if (!this->Forward (bytesformat, bytesplaceholder))
                        return false;
                };
                
                // The value
                        
                wchar_t number [4];
                if (_snwprintf (number, sizeof number / sizeof number [0],
                                signedvalue ? L"%I64d" : L"%I64u", value)) {
                    
                    if (!this->LocalNumber (number, language, flags, 0,
                                            signedvalue ? (value < 0) : false))
                        return false;
                };
                
                if (!this->Forward (c0x20))
                    return false;

                // Trailing part of the 'bytes' string
                
                if (bytesformat) {
                    if (bytesplaceholder) {
                        auto traillength = byteslength - (bytesplaceholder - bytesformat);
                        
                        if (auto trail = WcsNChr (bytesplaceholder, traillength, L' ')) {
                            return this->String (trail + 1, byteslength - (trail - bytesformat) - 1);
                        } else
                        if (traillength > 2) {
                            return this->String (trail + 2, traillength - 2);
                        } else
                            return true;
                    } else
                        return this->String (bytesformat, byteslength);
                } else
                    return this->Forward (cB);
                
            } else {
                auto v = 0.0;
                if (signedvalue) {
                    v = (double) value;
                } else {
                    v = (double) (std::uintmax_t) value;
                };
                
                auto m = -1;
                while (std::abs (v) >= 922) {
                    v /= 1024.0;
                    m++;
                };
                
                wchar_t number [16];
                if (v < 10.0) {
                    if (_snwprintf (number, sizeof number / sizeof number [0], L"%.1f", v)) {
                        if (!this->LocalNumber (number, language, flags, 1, false))
                            return false;
                    };
                } else {
                    if (_snwprintf (number, sizeof number / sizeof number [0], L"%.0f", v)) {
                        if (!this->LocalNumber (number, language, flags, 0, false))
                            return false;
                    };
                };

                // Units
                
                if (!this->Forward (c0x20))
                    return false;
                
                if (conversion.contains (L'#')) {
                    return this->Forward (m ? prefix [m] : cK)
                        && this->Forward (ci)
                        && this->Forward (cB);
                } else
                    return this->Forward (prefix [m])
                        && this->Forward (cB);
            };
        };
    
        // EXTENSION: L = displays value in locale-specific manner

        if (conversion.contains (L'L')) {
            wchar_t number [22];
            if (_snwprintf (number, sizeof number / sizeof number [0],
                            signedvalue ? L"%I64d" : L"%I64u", value)) {
                
                return this->LocalNumber (number, language, flags, 0,
                                          signedvalue ? (value < 0) : false);
            };
        };
        
        // following conversions need clean format string

        auto fmt = this->CleanConversion (ChangeLongLongToI64Clean);
        
        // EXTENSION: k/K = error code formatting
        //  - NOTE: mind the continuation to standard rendering!

        if (conversion.contains (L'k') || conversion.contains (L'K')) {
            if (auto c = this->ErrorCodeHexadecimal (value, signedvalue)) {
                
                fmt [std::wcslen (fmt) - 1] = c;
                
                if (!this->String (L"0x", 2u))
                    return false;
            };
        };
        
        // standard rendering

        switch (conversion.stars) {
            default:
                return this->DefaultOperation (fmt, value);
            case 1:
                return this->DefaultOperation (fmt, value, conversion.star [0]);
            case 2:
                return this->DefaultOperation (fmt, value, conversion.star [0], conversion.star [1]);
        };
    };

    bool Printer::NonDecimalInteger () {

        // additional star arguments
        
        conversion.initstars (this->va);
        
        // value
        //  - relying on little endian architecture here

        std::uintmax_t value;

        if (!this->LoadVersionInfoInteger (value)) {
            value = this->LoadUnsignedInteger ();
        };
        
        // EXTENSION: restrict size: h (short), hh (char), hhh (nibble), hhhh (bool)
        
        this->ApplyValueLowercaseHCrop (value);
        
        // EXTENSION: ? = zero prints nothing
        
        if (conversion.contains (L'?')) {
            if (value == 0) {
                return true;
            };
        };
        
        // standard rendering
        
        auto fmt = this->CleanConversion (ChangeLongLongToI64Clean);
        switch (conversion.stars) {
            default:
                return this->DefaultOperation (fmt, value);
            case 1:
                return this->DefaultOperation (fmt, value, conversion.star [0]);
            case 2:
                return this->DefaultOperation (fmt, value, conversion.star [0], conversion.star [1]);
        };
    };

    bool Printer::FloatingPoint () {
        
        // additional star arguments
        
        conversion.initstars (this->va);
        
        // for L$ and LL, eat another argument for 'language' (if present)
        //  - language/flags are for GetCurrencyFormat/GetNumberFormat
        
        const bool decimal = conversion.letter == L'f' || conversion.letter == L'g'
                          || conversion.letter == L'F' || conversion.letter == L'G';

        DWORD flags = 0;
        LCID language = LOCALE_USER_DEFAULT;
        
        if (decimal) {
            this->LoadLocalizedConversionParameters (language, flags);
        };
        
        // MSVCRT doesn't know that standard 'L' == "long double"
        //  - using 'll' instead because 'L' here means 'local'
        //  - TODO: support 'j' for '128-bit quadruple float'
        
        double value;
        if (conversion.count (L'h')) {
            auto half = va_arg (this->va, unsigned int);
            auto converted = this->Float16ToFloat64 (half);
            std::memcpy (&value, &converted, sizeof value);
        } else
        if (conversion.contains (L"ll")) {
            value = va_arg (this->va, long double);
        } else {
            value = va_arg (this->va, double);
        };
        
        // EXTENSION: ? = displays nothing if NaN

        if (conversion.contains (L'?')) {
            if (std::isnan (value)) {
                return true;
            };
        };
        
        // EXTENSION: U = displays unicode symbol for negative and not finite values
        
        if (conversion.contains (L'U')) {
            if (std::isnan (value)) {
                return this->String (L"\x00F8", 1u);
                // TODO: negative NaN?
                // TODO: NaN payload to string ID conversion?
            } else
            if (std::isinf (value)) {
                if (value > 0.0) {
                    if (conversion.contains (L'+')) {
                        return this->String (L"+\x221E", 2u);
                    } else
                    if (conversion.contains (L' ')) {
                        return this->String (L" \x221E", 2u);
                    } else {
                        return this->String (L"\x221E", 1u);
                    };
                } else {
                    return this->String (L"\x2013\x221E", 2u);
                };
/*            } else
            if (value < 0.0) {
                value = -value;
                this->String (L"\x2013", 1u);*/
            };
        };
        
        if (decimal) {
            
            // currency/numeric extensions require finite numbers
            
            if (std::isfinite (value)) {
                    
                // EXTENSION: Q = guess best precision
                //  - precision means number of consequent 0's or 9's concatenated
                
                if (conversion.contains (L'Q')) {
                    
                    // default precision is 3
                    //  - 16 is maximum precision for which msvcrt.dll printf can
                    //    render fractional part
                    
                    auto precision = std::abs (conversion.precision (3));
                    if (precision) {
                        if (precision > 16) {
                            precision = 16;
                        };
                    
                        // render
                        //  - cannot use less memory to support large floats

                        if (this->memory.Ensure (384 * sizeof (wchar_t))) {
                            
                            auto number = reinterpret_cast <wchar_t *> (this->memory.Data ());
                            auto nchars = this->memory.Size () / sizeof (wchar_t);
                            
                            if (_snwprintf (number, nchars, L"%.*f", 316, value)) {
            
                                // generate needles
                                
                                wchar_t zeros [17];
                                wchar_t nines [17];
                                
                                for (auto i = 0; i != precision; ++i) {
                                    zeros [i] = L'0';
                                    nines [i] = L'9';
                                };
                                
                                zeros [precision] = L'\0';
                                nines [precision] = L'\0';
                                
                                // search
                                
                                if (auto pDot = std::wcschr (number, L'.')) {
                                    auto pZeros = std::wcsstr (pDot, zeros);
                                    auto pNines = std::wcsstr (pDot, nines);
                                    
                                    // compute distance
                                    
                                    if (pZeros && pNines) {
                                        precision = (int) std::min (pZeros - pDot - 1, pNines - pDot - 1);
                                    } else
                                    if (pZeros) {
                                        precision = (int) (pZeros - pDot) - 1;
                                    } else
                                    if (pNines) {
                                        precision = (int) (pNines - pDot) - 1;
                                    };
                                    
                                    // when L or LL is present then render as localized number
                                    
                                    if (conversion.contains (L'L') && !conversion.contains (L'$')) {
                                        return this->LocalNumber (number, language, flags, precision, value < 0.0);
                                        
                                    } else {
                                        
                                        // otherwise replacing numbers in string by '*' and passing computed precision
                                        
                                        auto fmt = this->CleanConversion (RemoveBigLongClean | ForceNumbersToStarsClean);
                                        return this->DefaultOperation (fmt, value, conversion.width (), precision);
                                    };
                                };
                            };
                        };
                    };
                };
                
                // EXTENSION: $ = displays monetary value
    
                if (conversion.contains (L'$')) {
                
                    // format undecorated string for GetCurrencyFormat
                    //  - precision of 2 is typical for currency
                    
                    auto precision = conversion.precision (2);
                    
                    // 128 should suffice for readably long currency amounts

                    if (this->memory.Ensure (128 * sizeof (wchar_t))) {
                        
                        auto number = reinterpret_cast <wchar_t *> (this->memory.Data ());
                        auto nchars = this->memory.Size () / sizeof (wchar_t);
                        
                        if (_snwprintf (number, nchars, L"%.*f", precision, value)) {
                    
                            return this->LocalCurrency (number, language, flags, precision, value < 0.0);
                        };
                    };
                };
            
                // EXTENSION: L = displays value in locale-specific manner
    
                if (conversion.contains (L'L')) {
        
                    // format undecorated string for GetNumberFormat
                    //  - precision 6 is specified in standard
                    
                    auto precision = conversion.precision (6);

                    // 316 is length of largest double plus some precission (alloc?)

                    if (this->memory.Ensure (384 * sizeof (wchar_t))) {
                        
                        auto number = reinterpret_cast <wchar_t *> (this->memory.Data ());
                        auto nchars = this->memory.Size () / sizeof (wchar_t);
                        
                        if (_snwprintf (number, nchars, L"%.*f", precision, value)) {
                        
                            return this->LocalNumber (number, language, flags, precision, value < 0.0);
                        };
                    };
                };
    
            };
        };
        
        // standard rendering
        
        auto fmt = this->CleanConversion (RemoveBigLongClean);
        switch (conversion.stars) {
            default:
                return this->DefaultOperation (fmt, value);
            case 1:
                return this->DefaultOperation (fmt, value, conversion.star [0]);
            case 2:
                return this->DefaultOperation (fmt, value, conversion.star [0], conversion.star [1]);
        };
    };
    
    bool Printer::ZeroTermination (bool & terminate) {
        
        // value
        //  - relying on little endian architecture here
        
        auto value = this->LoadUnsignedInteger ();
        
        // EXTENSION: restrict size: h (short), hh (char), hhh (nibble), hhhh (bool)
        
        this->ApplyValueLowercaseHCrop (value);
        
        // MODIFIER: - means negative logic

        if (conversion.contains (L'-')) {
            value = !value;
        };
        
        // non-zero means terminate string at this point
        //  - displaying spec string if any
        
        if (value) {
            terminate = true;
            if (this->conversion.spec)
                return this->Forward (this->conversion.spec, this->conversion.spec_end);
        };
        
        return true;
    };
    
    bool Printer::DateTime () {
        
        DWORD flags = 0;
        LCID language;
        
        wchar_t * format = nullptr;
        if (this->conversion.spec) {
            const auto length = this->conversion.spec_end - this->conversion.spec;

            if (length == 2u
                && (this->conversion.spec [0] == 'd' || this->conversion.spec [0] == 'D')
                && (this->conversion.spec [1] == 'b' || this->conversion.spec [1] == 'B')) {

                switch (conversion.letter) {
                    case L'D': format = L"yyyy-MM-dd"; break;
                    case L'T': format = L"HH:mm:ss"; break; // TODO: milliseconds?
                };

            } else
            if (this->memory.Ensure (sizeof (wchar_t) * (length + 1))) {
                format = reinterpret_cast <wchar_t *> (this->memory.Data ());

                std::wcsncpy (format, this->conversion.spec, length);
                format [length] = L'\0';
            } else
                return false;
        };

        switch (conversion.count (L'L')) {
            case 0u:
                language = LOCALE_INVARIANT;
                break;
            case 1u:
                language = LOCALE_USER_DEFAULT;
                break;
            default:
                flags = LOCALE_NOUSEROVERRIDE;
                language = va_arg (this->va, unsigned int);
                break;
        };

        if (conversion.contains (L'+')) {
            switch (conversion.letter) {
                case L'D': flags |= DATE_LONGDATE; break;
                case L'T': flags |= TIME_FORCE24HOURFORMAT; break;
            };
        };
        if (conversion.contains (L'-')) {
            switch (conversion.letter) {
                case L'D': flags |= DATE_SHORTDATE; break;
                case L'T': flags |= TIME_NOSECONDS; break;
            };
        };
        
        // date/time to render
        //  - K - do not load parameter, use system time
        //  - otherwise parameter is either pointer to SYSTEMTIME or NULL
        //    in which case current local time is used
        
        SYSTEMTIME st;

        if (conversion.contains (L'K')) {
            GetSystemTime (&st);
        } else {
            if (auto pst = va_arg (this->va, SYSTEMTIME *)) {
                st = *pst;
            } else {
                GetLocalTime (&st);
            };
        };
        
        // 384 bytes should cover even esoteric date/time formats very well
        
        wchar_t output [384];
        switch (conversion.letter) {
            case L'D':
                if (int length = GetDateFormat (language, flags, &st, format, output, sizeof output / sizeof output [0])) {
                    return this->String (output, length - 1);
                };
                break;
            case L'T':
                if (int length = GetTimeFormat (language, flags, &st, format, output, sizeof output / sizeof output [0])) {
                    return this->String (output, length - 1);
                };
                break;
        };
        
        return false;
    };
    
    wchar_t NibbleToHex (unsigned char v, wchar_t base = L'A') {
        if (v < 10)
            return L'0' + v;
        else
            return base + v - 10;
    };
    
    bool Printer::BinaryData () {
        
        // additional star arguments
        //  - width = row length in printed bytes
        //  - precision = bytes grouping
        
        conversion.initstars (this->va);
        
        // data

        auto data = va_arg (this->va, const unsigned char *);
        auto size = va_arg (this->va, std::size_t);
        
        // parameters
        
        const auto nth = conversion.precision (1);
        const auto base = conversion.contains (L'B') ? L'A' : L'a';
        const auto prefix = L'\'';
        const auto readable = conversion.contains (L'h');
        const auto nice = conversion.contains (L'Q');

        auto width = conversion.width (65536);
        auto leads = 0;
        auto row = 0;
        
        // TODO: specifying width results in 'nice' positioning
        
        if (nice) {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (this->output.IsConsole (&info)) {
                
                row = info.dwCursorPosition.Y;
                leads = info.dwCursorPosition.X;
                
                if (width == 65536) {
                    width = info.dwSize.X - leads; // columns available
                    if (nth) {
                        width -= ((width / nth) + 2) / 3; // TODO: compute width
                    };
                    width /= 2;
                };
            };
        };
        
        if (width != 65536 && conversion.contains (L'-')) {
            // set leads to align right
        };
        
        if (size) {
            bool first = true;
            for (std::size_t i = 0u; i != size; ++i, ++data) {
                if (!first && nth && !(i % nth)) {
                    if (!this->Forward (L' '))
                        return false;
                };
                
                if (readable) {
                    if ((*data >= '0' && *data <= '9')
                        || (*data >= 'a' && *data <= 'f')
                        || (*data >= 'A' && *data <= 'F')) {

                        this->Forward (prefix);
                        this->Forward ((wchar_t) *data);
                    } else {
                        this->Forward (NibbleToHex (*data >> 4, base));
                        this->Forward (NibbleToHex (*data & 0x0F, base));
                    };
                } else {
                    this->Forward (NibbleToHex (*data >> 4, base));
                    this->Forward (NibbleToHex (*data & 0x0F, base));
                };
                
                if (nice && !((i + 1) % width)) {
                    this->output.MoveCursor (leads, ++row);
                    first = true;
                } else
                    first = false;
            };
        };
        
        return true;
    };

#ifdef _MSC_VER
    uint32_t __inline ctz (uint32_t value) {
        DWORD trailing_zero = 0;

        if (_BitScanForward (&trailing_zero, value)) {
            return trailing_zero;
        } else {
            return 32;
        }
    }
#elif __GNUC__
    static uint32_t ctz (uint32_t x) {
        return __builtin_ctz (x);
    };
#else
    static uint32_t clz (uint32_t x) {
        x |= (x >> 1);
        x |= (x >> 2);
        x |= (x >> 4);
        x |= (x >> 8);
        x |= (x >> 16);
        return 32 - popcnt (x);
    };
    static uint32_t ctz (uint32_t x) {
        return popcnt ((x & -x) - 1);
    };
#endif

    bool longest_set_bits_string_in_byte (unsigned int mask, int & _o, int & _n) {
        unsigned int ii = 0;
        unsigned int nn = 0;

        while (mask) {
            ii = ctz (mask);
            ++nn;
            mask &= (mask >> 1u);
        };

        if (nn > 1u) {
            _o = ii;
            _n = nn;
            return true;
        } else
            return false;
    };

    bool Printer::IPAddress () {
        auto address = va_arg (this->va, const sockaddr_storage *);
        bool decorate6 = true;
        bool showport = true;

            /*switch (ss->ss_family) {
                case AF_INET:
                    family = ss->ss_family;
                    in4 = &reinterpret_cast <const sockaddr_in *> (ss)->sin_addr;
                    port = bswap16 (reinterpret_cast <const sockaddr_in *> (ss)->sin_port);
                    break;
                case AF_INET6:
                    family = ss->ss_family;
                    in6 = &reinterpret_cast <const sockaddr_in6 *> (ss)->sin6_addr;
                    port = bswap16 (reinterpret_cast <const sockaddr_in6 *> (ss)->sin6_port);
                    break;
            };*/
        
        wchar_t string [64];
        wchar_t * p = string;

        switch (address->ss_family) {
            default:
                string [0] = 0; // symbol???
                break;

            case AF_INET:
/*                switch (special) {
                    case 2:
                        snwprintf (string, 40u, L"%u.%u%s",
                                   in4->s_net,
                                   65536 * in4->s_host
                                   + 256 * in4->s_lh
                                   + in4->s_impno,
                                   portsz);
                        break;
                    case 1:
                        snwprintf (string, 40u, L"%u.%u.%u%s",
                                   in4->s_net, in4->s_host,
                                   256 * in4->s_lh + in4->s_impno,
                                   portsz);
                        break;
                    default:*/
                _snwprintf (p, sizeof string / sizeof string [0] - 8,
                            L"%u.%u.%u.%u",
                            reinterpret_cast <const sockaddr_in *> (address)->sin_addr.s_net,
                            reinterpret_cast <const sockaddr_in *> (address)->sin_addr.s_host,
                            reinterpret_cast <const sockaddr_in *> (address)->sin_addr.s_lh,
                            reinterpret_cast <const sockaddr_in *> (address)->sin_addr.s_impno);
                
                if (auto port = Swap16 (reinterpret_cast <const sockaddr_in *> (address)->sin_port)) {
                    _snwprintf (p + std::wcslen (p), 8, L":%u", port);
                };
/*                        break;
                };*/
                break;

            case AF_INET6:
                decorate6 = (reinterpret_cast <const sockaddr_in6 *> (address)->sin6_port != 0);

                int o = 0; // index of first shortened part
                int n = 0; // number of shortened parts

                unsigned char mask = 0u;
                auto w = reinterpret_cast <const unsigned short *> (&reinterpret_cast <const sockaddr_in6 *> (address)->sin6_addr);
                for (unsigned int j = 0u; j != 8u; ++j)
                    if (!w [j])
                        mask |= 1 << j;

                int i = 0;

                if (decorate6) {
                    *p++ = L'[';
                };

                if (longest_set_bits_string_in_byte (mask, o, n)) {
                    for (; i != o; ++i) {
                        if (i) {
                            *p++ = L':';
                        };

                        _snwprintf (p, 5, L"%x", Swap16 (w [i]));
                        while (*p)
                            ++p;
                    };

                    *p++ = L':';
                    *p++ = L':';
                    i += n;
                };

                for (; i != 8; ++i) {
                    _snwprintf (p, 5, L"%x", Swap16 (w [i]));
                    while (*p)
                        ++p;

                    if (i != 7) {
                        *p++ = L':';
                    };
                };
                if (decorate6) {
                    *p++ = L']';
                };

                if (decorate6) { // TODO: separate parameter for port
                    _snwprintf (p, 7, L":%u", Swap16 (reinterpret_cast <const sockaddr_in *> (address)->sin_port));
                };

                break;
        };

        // unsigned int port;
        this->String (string, std::wcslen (string));
        return true;
    };
    
    bool CheckCodePageStringPrefix (const wchar_t * generalized, const wchar_t * prefix,
                                    UINT & codepage, unsigned int offset = 0u) {
        auto length = std::wcslen (prefix);
        
        if (std::wcsncmp (generalized, prefix, length) == 0) {
            if (IsDigit (generalized [length])) {
                codepage = offset + std::wcstoul (&generalized [length], nullptr, 10);
                return true;
            };
        };
        
        return false;
    };
    
    bool ParseCodePageString (const wchar_t * sp, const wchar_t * se, UINT & codepage) {
        wchar_t generalized [14];
        wchar_t * gp = &generalized [0];
        
        if (se == nullptr) {
            se = sp + std::wcslen (sp);
        };
        
        // generalize the string
        
        while ((sp != se) && (gp != &generalized [sizeof generalized / sizeof generalized [0]])) {
            switch (auto c = *sp++) {
                case L'-':
                case L'_':
                case L' ':
                case L'.':
                case L'/':
                    break;
                default:
                    if (c >= L'A' && c <= L'Z') {
                        *gp++ = c - L'A' + L'a';
                    } else {
                        *gp++ = c;
                    }
                    break;
            };
        };
        *gp = L'\0';
        
        // find meaningful code page prefix
        
        if (IsDigit (generalized [0])) {
            codepage = std::wcstoul (generalized, nullptr, 10);
            return true;
        };
        
        if (CheckCodePageStringPrefix (generalized, L"cp", codepage)
         || CheckCodePageStringPrefix (generalized, L"dos", codepage)
         || CheckCodePageStringPrefix (generalized, L"xcp", codepage)
         || CheckCodePageStringPrefix (generalized, L"windows", codepage)
         || CheckCodePageStringPrefix (generalized, L"iso8859", codepage, 28590))
            return true;
        
        if (std::wcsncmp (generalized, L"ibm", 3) == 0) {
            if (auto cp = std::wcstoul (&generalized [3], nullptr, 10)) {
                if ((cp >= 273 && cp < 300) || (cp >= 420 && cp < 425)) {
                    codepage = 20000 + cp;
                } else
                switch (cp) {
                    case 871:
                    case 880:
                    case 905:
                    case 924:
                        codepage = 20000 + cp;
                        break;
                    default:
                        codepage = cp;
                        break;
                };
                return true;
            };
        };

        if (std::wcsncmp (generalized, L"utf", 3) == 0) {
            // TODO: check for BOM
            
            wchar_t * endianess = nullptr;
            switch (std::wcstoul (&generalized [3], &endianess, 10)) {
                case 7:
                    codepage = CP_UTF7;
                    return true;
                case 8:
                    codepage = CP_UTF8;
                    return true;
                case 16:
                    if (*endianess != L'b')
                        codepage = CP_UTF16;
                    else
                        codepage = CP_UTF16_BE;
                    return true;

                case 32:
                    if (*endianess != L'b')
                        codepage = CP_UTF32;
                    else
                        codepage = CP_UTF32_BE;
                    return true;
            };
        };

        if (std::wcscmp (generalized, L"usascii") == 0) {
            codepage = 20127;
            return true;
        };
        if (std::wcscmp (generalized, L"xebcdic") == 0) {
            codepage = 20833;
            return true;
        };
        if (std::wcscmp (generalized, L"koi8r") == 0) {
            codepage = 20866;
            return true;
        };
        if (std::wcscmp (generalized, L"koi8u") == 0) {
            codepage = 21866;
            return true;
        };
        if (std::wcscmp (generalized, L"big5") == 0) {
            codepage = 950;
            return true;
        };
        if (std::wcscmp (generalized, L"eucjp") == 0) {
            codepage = 20932;
            return true;
        };
        if (std::wcscmp (generalized, L"gb2312") == 0) {
            codepage = 936;
            return true;
        };

        // TODO: 'id5'
        
        // TODO: 'base64'
        
        return false;
    };
    
    bool Printer::LoadSpecStringNumber (unsigned int & result) {
        if (this->conversion.spec) {
            switch (this->conversion.spec [0]) {
                
                // decimal or hexadecimal number as provided
                
                case L'0':
                    switch (this->conversion.spec [1]) {
                        case L'x':
                        case L'X':
                            result = std::wcstoul (&this->conversion.spec [2], nullptr, 16);
                            return true;
                        default:
                            ; // continues to base-10 (explicitely no octal support)...
                    };
                case L'1':
                case L'2':
                case L'3':
                case L'4':
                case L'5':
                case L'6':
                case L'7':
                case L'8':
                case L'9':
                    result = std::wcstoul (this->conversion.spec, nullptr, 10);
                    return true;
            };
        };
        return false;
    };
    

    bool Printer::LoadStringCodePageParameter (UINT & codepage) {
        if (this->conversion.spec) {
            switch (this->conversion.spec [0]) {
                
                // * - fetch from variable arguments
                
                case L'*': {
                    auto parameter = va_arg (this->va, const wchar_t *);
                    auto numeric = reinterpret_cast <const char *> (parameter) - static_cast <const char *> (0);
                    
                    if ((std::size_t) numeric >= 65536u) {
                        if (ParseCodePageString (parameter, nullptr, codepage))
                            return true;
                        
                    } else {
                        codepage = (unsigned int) numeric;
                        return true;
                    };
                } break;
                
                // decimal or hexadecimal number as provided
                
                case L'0':
                case L'1':
                case L'2':
                case L'3':
                case L'4':
                case L'5':
                case L'6':
                case L'7':
                case L'8':
                case L'9':
                    return this->LoadSpecStringNumber (codepage);
                
                // string name
                //  - generalized: lowercase, removed dashes, underscores, spaces and dots
                
                default:
                    if (ParseCodePageString (this->conversion.spec, this->conversion.spec_end, codepage))
                        return true;
            };
        };
        
        // no explicit codepage, use defaults

        if (this->conversion.contains (L'h')) {
            codepage = CP_ACP;
            return true;
        } else
        if (this->conversion.contains (L"ll")) {
            codepage = CP_UTF32;
            return true;
        } else {
            codepage = CP_UTF16;
            return false;
        };
    };

    bool Printer::LoadLocalizedConversionParameters (LCID & language, DWORD & flags) {

        if (conversion.contains (L'$')) {
            if (conversion.count (L'L') == 1) {
            
                // currency formatting, but not local (i.e. by provided LCID)
            
                language = va_arg (this->va, unsigned int);
                flags = LOCALE_NOUSEROVERRIDE;
                return true;
            };
        } else {
            if (conversion.count (L'L') == 2) {
                
                // localized number, but not local (i.e. localized by LCID)
                
                language = va_arg (this->va, unsigned int);
                flags = LOCALE_NOUSEROVERRIDE;
                return true;
            };
        };
        
        return false;
    };

    std::intmax_t Printer::LoadSignedInteger () {
        if (conversion.contains (L"ll")) {
            return va_arg (this->va, long long); // 64
        } else
        if (conversion.contains (L'j')) {
            return va_arg (this->va, std::intmax_t); // 64
        } else
        if (conversion.contains (L't')) {
            return va_arg (this->va, std::ptrdiff_t); // 64/32
        } else
        if (conversion.contains (L'z')) {
            return va_arg (this->va, std::size_t); // 64/32
        } else
        if (conversion.contains (L'l')) {
            return va_arg (this->va, long); // 32
        } else
            return va_arg (this->va, int); // 32
    };
    
    std::uintmax_t Printer::LoadUnsignedInteger () {
        if (conversion.contains (L"ll")) {
            return va_arg (this->va, unsigned long long); // 64
        } else
        if (conversion.contains (L'j')) {
            return va_arg (this->va, std::uintmax_t); // 64
        } else
        if (conversion.contains (L't')) {
            return va_arg (this->va, std::ptrdiff_t); // 64/32
        } else
        if (conversion.contains (L'z')) {
            return va_arg (this->va, std::size_t); // 64/32
        } else
        if (conversion.contains (L'l')) {
            return va_arg (this->va, unsigned long); // 32
        } else
            return va_arg (this->va, unsigned int); // 32
    };

    template <typename T>
    unsigned short VersionInfoValue (const T & info, int index) {
        switch (index) {
            case 0:
            case '0':
            case 'M':
            case 'm':
                return info.major;
            case 1:
            case '1':
            case 'N':
            case 'n':
                return info.minor;
            case 2:
            case '2':
            case 'R':
            case 'r':
                return info.release;
            case 3:
            case '3':
            case 'B':
            case 'b':
                return info.build;
            
            default:
                return 0xFFFF;
        };
    };
    
    unsigned short VersionInfoValue (HMODULE module, bool product, int index) {

        // OPTIMIZE: cache 'versioninfo' for given 'module' in 'Printer'
        
        Resources::VersionInfo vi (module);
        if (product) {
            return VersionInfoValue (vi.info->product, index);
        } else
            return VersionInfoValue (vi.info->file, index);
    };

    unsigned short VersionInfoValue (HMODULE module, const char * p) {
        switch (p [0]) {
            
            // product
            case 'p':
            case 'P':
                switch (p [1]) {
                    
                    // wchar_t string? try third byte
                    case '\0':
                        return VersionInfoValue (module, true, p [2]);
                    
                    // char string, use second byte
                    default:
                        return VersionInfoValue (module, true, p [1]);
                };
            
            // file
            case 'f':
            case 'F':
                switch (p [1]) {
                    
                    // wchar_t string? try third byte
                    case '\0':
                        return VersionInfoValue (module, false, p [2]);
                    
                    // char string, use second byte
                    default:
                        return VersionInfoValue (module, false, p [1]);
                };
            
            default:
                return VersionInfoValue (module, false, p [0]);
        };
    };

    template <typename T>
    bool Printer::LoadVersionInfoInteger (T & value) {
        
        // module is always the first

        HMODULE module;
        if (conversion.contains (L'V')) {
            module = va_arg (this->va, HMODULE);
        } else
        if (conversion.contains (L'v')) {
            module = GetModuleHandle (NULL);
        } else
            return false;
        
        // using spec string if any
        
        if (conversion.spec != nullptr) {
            
            value = VersionInfoValue (module, reinterpret_cast <const char *> (conversion.spec));
            return true;
            
        } else {
            auto parameter = va_arg (this->va, const char *);
            auto integer = parameter - static_cast <const char *> (0);
            
            if (integer > 65535) {
                
                // string pointer
                
                value = VersionInfoValue (module, parameter);
                return true;
                
            } else {
                
                // character or two-character constant
                //  - if 'pX' or 'PX' then read product info, otherwise file info
                
                const auto index = integer & 0xFF;
                const auto product = (((integer >> 8) & 0xFF) == 'p')
                                  || (((integer >> 8) & 0xFF) == 'P');
                
                value = VersionInfoValue (module, product, (unsigned int) index);
                return true;
            };
        };
        
        return false;
    };

    template <typename T>
    void Printer::ApplyValueLowercaseHCrop (T & value) {
        
        // restricts size: h (short), hh (char), hhh (nibble), hhhh (bool)
        
        switch (conversion.count (L'h')) {
            case 0u: break;
            case 1u: value &= 0x0000FFFFuLL; break;
            case 2u: value &= 0x000000FFuLL; break;
            case 3u: value &= 0x0000000FuLL; break;
            case 4u: value &= 0x00000001uLL; break;
        };
    };
    
    unsigned long long Printer::Float16ToFloat64 (unsigned short half) {
        auto exponent = half & 0x7c00u;
        auto sign = ((unsigned long long) (half & 0x8000u)) << 48;
        
        switch (exponent) {
            
            // zero or subnormal
            case 0x0000u: 
                if (auto significant = half & 0x03ffu) {
                    
                    // subnormal
                    significant <<= 1;
                    while ((significant & 0x0400u) == 0) {
                        significant <<= 1;
                        exponent++;
                    };
                    
                    return sign
                         | (((unsigned long long) (1023 - 15 - exponent)) << 52)
                         | (((unsigned long long) (significant & 0x03ffu)) << 42);
                } else
                    // zero
                    return sign;
                
            // inf or NaN
            case 0x7c00u: 
                // all-ones exponent and a copy of the significand
                return sign 
                     | 0x7ff0000000000000uLL
                     | (((unsigned long long) (half & 0x03ffu)) << 42);
                
            // normalized
            default: 
                // adjust the exponent and shift
                return sign 
                     | (((unsigned long long) (half & 0x7fffu) + 0xfc000u) << 42);
        };
        
    };

    bool Printer::LocalNumber (const wchar_t * number,
                               LCID language, DWORD flags,
                               int precision, bool negative) {
        NUMBERFMT fmt;
        std::memset (&fmt, 0, sizeof fmt);
        
        wchar_t decimal [16];
        wchar_t thousand [16];
        wchar_t grouping [32];

#define NUMBERFMT_FIELD(name) reinterpret_cast <LPWSTR> (&name), sizeof (name) / sizeof (WCHAR)
        
        if (GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_IDIGITS, NUMBERFMT_FIELD (fmt.NumDigits))
                && fmt.NumDigits != (unsigned int) precision) {
            
            if (   GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_ILZERO, NUMBERFMT_FIELD (fmt.LeadingZero))
                && GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_INEGNUMBER, NUMBERFMT_FIELD (fmt.NegativeOrder))
                && GetLocaleInfo (language, flags | LOCALE_SDECIMAL, decimal, sizeof decimal / sizeof decimal [0])
                && GetLocaleInfo (language, flags | LOCALE_STHOUSAND, thousand, sizeof thousand / sizeof thousand [0])
                && GetLocaleInfo (language, flags | LOCALE_SGROUPING, grouping, sizeof grouping / sizeof grouping [0])) {
                
                fmt.NumDigits = precision;
                fmt.lpDecimalSep = decimal;
                fmt.lpThousandSep = thousand;
                
                auto * p = grouping;
                while (*p) {
                    if ((*p >= L'1') && (*p <= L'9')) {
                        fmt.Grouping = fmt.Grouping * 10u + (*p - L'0');
                    };
                    if ((*p != L'0') && !p[1]) {
                        fmt.Grouping *= 10u;
                    };
                    ++p;
                };
                
            } else {
                precision = -1;
            };
            
            // when using custom NUMBERFMT, flags must be 0 (propagated above)
            flags = 0;
        } else {
            // precision provided is same as default or error, use NULL below
            precision = -1;
        };

#undef NUMBERFMT_FIELD
        
        // cannot use this->memory since it is already used for 'number'
        // 512 should be enough to format up-to about 316 characters long number
        //  - wcslen (thousand) * (wcslen (number) / 3) // 3 - from Grouping???
        
        wchar_t localized [512];
        if (auto nlocalized = GetNumberFormat (language, flags, number, (precision == -1) ? NULL : &fmt,
                                               &localized[1], sizeof localized / sizeof localized [0] - 1)) {

            // make proper string conversion format
            //  - reusing requested width, alignmnt (-) and plus symbol request
            
            auto width = conversion.width ();
            if (conversion.contains (L'-')) {
                width = -width;
            };

            int offset = 1;
            if (conversion.contains (L'+') && !negative) {
                localized [0] = L'+';
                offset = 0;
            };
            if (conversion.contains (L' ') && !negative) {
                localized [0] = L' ';
                offset = 0;
            };
            
            // request buffer to fit currency string
            //  - or whatever minimum field width was required
            
            using namespace std;
            auto size = max ((int) (nlocalized - 1), std::abs (width)) + 1;
            if (this->memory.Ensure (size * sizeof (wchar_t))) {
                
                // ask msvcrt to format the string appropriately
                
                auto p = reinterpret_cast <wchar_t *> (this->memory.Data ());
                if (_snwprintf (p, size, L"%*s", width, localized + offset)) {
                    
                    if (conversion.contains (L'U') && negative) {
                        if (auto minus = WcsNChr (p, size, L'-')) {
                            *minus = L'\x2013';
                        };
                    };
                    
                    // pass to Output
                    return this->String (p, size - 1);
                };
            };
        };
        return false;
    };
    
    bool Printer::LocalCurrency (const wchar_t * number,
                                 LCID language, DWORD flags,
                                 int precision, bool negative) {
        CURRENCYFMT fmt;
        std::memset (&fmt, 0, sizeof fmt);

        wchar_t decimal [16];
        wchar_t thousand [16];
        wchar_t grouping [32];
        wchar_t symbol [32];

#define NUMBERFMT_FIELD(name) reinterpret_cast <LPWSTR> (&name), sizeof (name) / sizeof (WCHAR)

        if (GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_ICURRDIGITS, NUMBERFMT_FIELD (fmt.NumDigits))
            && fmt.NumDigits != (unsigned int) precision) {

            if (   GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_ILZERO, NUMBERFMT_FIELD (fmt.LeadingZero))
                && GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_INEGCURR, NUMBERFMT_FIELD (fmt.NegativeOrder))
                && GetLocaleInfo (language, flags | LOCALE_RETURN_NUMBER | LOCALE_ICURRENCY, NUMBERFMT_FIELD (fmt.PositiveOrder))
                && GetLocaleInfo (language, flags | LOCALE_SDECIMAL, decimal, sizeof decimal / sizeof decimal [0])
                && GetLocaleInfo (language, flags | LOCALE_STHOUSAND, thousand, sizeof thousand / sizeof thousand [0])
                && GetLocaleInfo (language, flags | LOCALE_SGROUPING, grouping, sizeof grouping / sizeof grouping [0])
                && GetLocaleInfo (language, flags | LOCALE_SCURRENCY, symbol, sizeof symbol / sizeof symbol [0])) {

                fmt.NumDigits = precision;
                fmt.lpDecimalSep = decimal;
                fmt.lpThousandSep = thousand;
                fmt.lpCurrencySymbol = symbol;

                auto * p = grouping;
                while (*p) {
                    if ((*p >= L'1') && (*p <= L'9')) {
                        fmt.Grouping = fmt.Grouping * 10u + (*p - L'0');
                    };
                    if ((*p != L'0') && !p [1]) {
                        fmt.Grouping *= 10u;
                    };
                    ++p;
                };

            } else {
                precision = -1;
            };

            // when using custom CURRENCYFMT, flags must be 0 (propagated above)
            flags = 0;
        } else {
            // precision provided is same as default or error, use NULL below
            precision = -1;
        };

#undef NUMBERFMT_FIELD

        // cannot use this->memory since it is already used for 'number'
        // 192 should be enough considering we use 128 as max length for 'number'
        
        wchar_t currency [192];
        if (auto ncurrency = GetCurrencyFormat (language, flags, number, (precision == -1) ? NULL : &fmt,
                                                &currency[1], sizeof currency / sizeof currency [0] - 1)) {
            
            // make proper string conversion format
            //  - reusing requested width, alignmnt (-) and plus symbol request

            auto width = conversion.width ();
            if (conversion.contains (L'-')) {
                width = -width;
            };

            int offset = 1;
            if (conversion.contains (L'+') && !negative) {
                currency [0] = L'+';
                offset = 0;
            };
            if (conversion.contains (L' ') && !negative) {
                currency [0] = L' ';
                offset = 0;
            };
            
            // request buffer to fit currency string
            //  - or whatever minimum field width was required
            
            using namespace std;
            auto size = max ((int) ncurrency, std::abs (width)) + 1;
            if (this->memory.Ensure (size * sizeof (wchar_t))) {
                
                // ask msvcrt to format the string appropriately
                
                auto p = reinterpret_cast <wchar_t *> (this->memory.Data ());
                if (_snwprintf (p, size, L"%*s", width, currency + offset)) {
                
                    // pass to Output
                    return this->String (p, size - 1);
                };
            };
        };
        return false;
    };

    template <typename T>
    struct MantissaMax { static const auto value = 22; /* octal int64 */ };
    
    template <> struct MantissaMax <float> { static const auto value = 50; };
    template <> struct MantissaMax <double> { static const auto value = 320; };
    template <> struct MantissaMax <long double> { static const auto value = 4945; };

    template <typename T, typename... Params>
    bool Printer::DefaultOperation (const wchar_t * fmt, T value, Params... parameters) {
        using namespace std;
        auto size = 2 + max (abs (conversion.width ()),
                             MantissaMax <T> ::value + conversion.precision ());
        if (this->memory.Ensure (size * sizeof (wchar_t))) {

//        printf (" >%ls;%u;%f,%d< ", fmt, (unsigned int) sizeof value, value, parameters...);

            auto p = reinterpret_cast <wchar_t *> (this->memory.Data ());        
            auto length = _snwprintf (p, size, fmt, parameters..., value);

//        printf (" >%d;%d< ", length, errno);

            if (length >= 0 && length <= size)
                return this->String (p, length);
        };
        return false;
    };

    bool Printer::String (const wchar_t * ptr, std::size_t length) {
        if (length)
            return this->output.String (ptr, length);
        else
            return true;
    };
    bool Printer::Forward (const wchar_t * begin, const wchar_t * end) {
        return this->String (begin, end - begin);
    };

    bool Printer::Repeat (wchar_t character, std::size_t n) {
        return this->output.Repeat (character, n);
    };
    
    bool Output::Repeat (wchar_t character, std::size_t n) {
        for (std::size_t i = 0u; i != n; ++i) {
            if (!this->String (&character, 1u))
                return false;
        };
        return true;
    };

    bool Output::IsConsole (CONSOLE_SCREEN_BUFFER_INFO *) {
        return false;
    };
    bool OutputConsole::IsConsole (CONSOLE_SCREEN_BUFFER_INFO * info) {
        return GetConsoleScreenBufferInfo (this->handle, info);
    };
    void Output::MoveCursor (unsigned short, unsigned short) {};
    void OutputConsole::MoveCursor (unsigned short x, unsigned short y) {
        SetConsoleCursorPosition (this->handle, { (SHORT) x, (SHORT) y });
    };
    
    // TODO: override OutputConsole::Repeat to minimize WriteConsole calls
    // TODO: override OutputHandle::Repeat to minimize WriteFile calls

    bool OutputConsole::String (const wchar_t * string, std::size_t length) {
        DWORD written = 0u;
        BOOL  result = WriteConsole (this->handle, string, (DWORD) length, &written, NULL);
        
        this->n_out += written;
        return result && written == length;
    };

    bool OutputHandle::String (const wchar_t * string, std::size_t length) {
        DWORD written = 0u;
        switch (this->codepage) {
            
            case CP_UTF16: {
                length *= sizeof (wchar_t);
                auto result = WriteFile (this->handle, string, (DWORD) length, &written, NULL);
                
                this->n_out += written / sizeof (wchar_t);
                
                return result && written == length;
            } break;
                    
            default:
                Encoder e (this->codepage, string, length);
                
                if (auto n = e.Size ()) {
                    if (!this->memory.Ensure (n)) {
                        return false;
                    };
                    
                    auto output = this->memory.Data ();
                    auto maxout = this->memory.Size ();
                    auto all = e.Encode (output, maxout);
                    auto write = output - this->memory.Data ();
                    
                    auto result = WriteFile (this->handle, this->memory.Data (), (DWORD) write, &written, NULL);
                    
                    this->n_out += written;
                    
                    return result
                        && written == (DWORD) write
                        && all;
                } else
                    return false;
        };
    };

    bool OutputBuffer::String (const wchar_t * string, std::size_t length) {
        Encoder e (this->codepage, string, length);
        auto result = e.Encode (this->buffer, this->size);
        this->n_out += e.encoded;
        return result;
    };

    std::size_t Input::Conversion::count (wchar_t c) {
        if (this->spec) {
            return std::count (this->begin, this->spec, c)
                 + std::count (this->spec_end, this->end, c);
        } else {
            return std::count (this->begin, this->end, c);
        };
    };
    
    template <unsigned N>
    bool Input::Conversion::contains (const wchar_t (&substring) [N]) {
        const wchar_t * sb = &substring [0];
        const wchar_t * se = &substring [N - 1];
        if (this->spec) {
            return std::search (this->begin, this->spec, sb, se) != this->spec
                || std::search (this->spec_end, this->end, sb, se) != this->end;
        } else {
            return std::search (this->begin, this->end, sb, se) != this->end;
        };
    };

    bool Input::Conversion::contains (wchar_t c) {
        if (this->spec) {
            return std::find (this->begin, this->spec, c) != this->spec
                || std::find (this->spec_end, this->end, c) != this->end;
        } else {
            return std::find (this->begin, this->end, c) != this->end;
        };
    };

    template <typename IT, typename LT>
    bool any_of_helper (IT i, IT e, LT lb, LT le) {
        for (; i != e; ++i) 
            for (auto li = lb; li != le; ++li)
                if (*i == *li)
                    return true;
        
        return false;
    };

    template <unsigned N>
    bool Input::Conversion::any_of (const wchar_t (&list) [N]) {
        const wchar_t * lb = &list [0];
        const wchar_t * le = &list [N - 1];
        if (this->spec) {
            return any_of_helper (this->begin, this->spec, lb, le)
                || any_of_helper (this->spec_end, this->end, lb, le);
        } else {
            return any_of_helper (this->begin, this->end, lb, le);
        };
    };

    int Input::Conversion::initstars (std::va_list & va) {
        const auto max = sizeof this->star / sizeof this->star [0];
        
        this->stars = (unsigned short) this->count ('*');
        if (this->stars > max) {
            this->stars = max;
        };
        
        for (auto i = 0u; i != this->stars; ++i) {
            this->star [i] = va_arg (va, int);
        };
        
        return this->stars;
    };

    int Input::Conversion::precision (int default_) {
        using namespace std;
        switch (this->stars) {
            case 2:
                return this->star [1];
            case 1:
                if (this->contains (L".*")) {
                    return this->star [0];
                } else {
            case 0:
                    long result;
                    wchar_t * e;
                    
                    if (this->spec) {
                        auto i = std::find (this->begin, this->spec, L'.');
                        if (i != this->spec)
                            if ((result = wcstol (i + 1, &e, 10)) || (e != i + 1))
                                return result;
            
                        auto j = std::find (this->spec_end, this->end, L'.');
                        if (j != this->end)
                            if ((result = wcstol (j + 1, &e, 10)) || (e != j + 1))
                                return result;
                            
                    } else {
                        auto i = std::find (this->begin, this->end, L'.');
                        if (i != this->end)
                            if ((result = wcstol (i + 1, &e, 10)) || (e != i + 1))
                                return result;
                    };
                };
        };
        
        return default_;
    };

    int Input::Conversion::width (int default_) {
        using namespace std;
        switch (this->stars) {
            case 2:
                return this->star [0];
            case 1:
                if (!this->contains (L".*")) {
                    return this->star [0];
                } else {
            case 0:
                    bool r = (this->letter == L's')
                          && (this->contains (L'r') || this->contains (L'R'));
                
                    if (this->spec) {
                        for (auto i = this->begin; i != this->spec; ++i) {
                            if (*i == L'.' || (r && (*i == L'+')))
                                return default_;
                                
                            if (IsDigit (*i))
                                return wcstol (i, nullptr, 10);
                        };
                        for (auto i = this->spec_end; i != this->end; ++i) {
                            if (*i == L'.' || (r && (*i == L'+')))
                                return default_;
                                
                            if (IsDigit (*i))
                                return wcstol (i, nullptr, 10);
                        };
                            
                    } else {
                        for (auto i = this->begin; i != this->end; ++i) {
                            if (*i == L'.' || (r && (*i == L'+')))
                                return default_;
                                
                            if (IsDigit (*i))
                                return wcstol (i, nullptr, 10);
                        };
                    };
                };
        };
        
        return default_;
    };
    
    unsigned int Input::Conversion::resource_string_base (unsigned int default_) {
        if ((this->letter == L's')
         && (this->contains (L'r') || this->contains (L'R'))) {
             
            if (this->spec) {
                auto i = std::find (this->begin, this->spec, L'+');
                if (i != this->spec)
                    return wcstol (i + 1, nullptr, 10);
    
                auto j = std::find (this->spec_end, this->end, L'+');
                if (j != this->end)
                    return wcstol (j + 1, nullptr, 10);
                    
            } else {
                auto i = std::find (this->begin, this->end, L'+');
                if (i != this->end)
                    return wcstol (i + 1, nullptr, 10);
            };
        };
        
        return default_;
    };

    
    // default code pages

    int default_cp_print = -1;
    int default_cp_error = -1;
    int default_cp_string = -1;
    int default_cp_socket = -1;
    int default_cp_file = -1;
    int default_cp_pipe = -1;
    
    int CurrentCharSPrintCP () {
        if (default_cp_string != -1)
            return default_cp_string;
        else
            return CP_ACP;
    };

    int CurrentErrorPrintCP (int cp) {
        if (cp != -1)
            return cp;
        else
        if (default_cp_error != -1)
            return default_cp_error;
        else
            return -1;
    };

    int CurrentFilePrintCP (int cp) {
        if (cp != -1)
            return cp;
        else
        if (default_cp_file != -1)
            return default_cp_file;
        else
            return CP_UTF16;
    };
    
    int CurrentPipePrintCP (int cp) {
        if (cp != -1)
            return cp;
        else
        if (default_cp_pipe != -1)
            return default_cp_pipe;
        else
            return CP_UTF16;
    };
    
    int CurrentPrintCP (int cp) {
        if (cp != -1)
            return cp;
        else
        if (default_cp_print != -1)
            return default_cp_print;
        else
            return CP_ACP;
    };
    
    int CurrentWSAPrintCP (int cp) {
        if (cp != -1)
            return cp;
        else
        if (default_cp_socket != -1)
            return default_cp_socket;
        else
            return 20127; // US-ASCII
    };
    
    bool ResolvePrintCP (void * handle, unsigned int & cp) {
        DWORD dw;
        switch (GetFileType (handle)) {
            case FILE_TYPE_DISK: // file
                cp = CurrentFilePrintCP (cp);
                return true;
                
            case FILE_TYPE_PIPE: // pipe, also redirection
                cp = CurrentPipePrintCP (cp);
                return true;
                
            case FILE_TYPE_CHAR:
                if (GetConsoleMode (handle, &dw)) { // console
                    return false;
                };
                
                // else?
            default:
                cp = CurrentPrintCP (cp);
                return true;
        };
    };
    
    bool SetDefaultCodePage (int & tg, int cp) {
        switch (cp) {
            case -1: // default
            case CP_ACP:
            case CP_OEMCP:
            case CP_MACCP:
            case CP_THREAD_ACP:
            case CP_UTF16:
            case CP_UTF16_BE:
            case CP_UTF32:
            case CP_UTF32_BE:
            case CP_UTF7:
            case CP_UTF8:
                tg = cp;
                return true;
            case 0xFEFF:
                tg = CP_UTF16;
                return true;
            case 0xFFFE:
                tg = CP_UTF16_BE;
                return true;

            default:
                if (IsValidCodePage (cp)) {
                    tg = cp;
                    return true;
                } else {
                    SetLastError (ERROR_INVALID_PARAMETER);
                    return false;  
                };
        };
    };
    
    std::size_t OutputBuffer::NulTerminatorLength (int cp) {
        switch (cp) {
            default:
                return 1u;
            case CP_UTF16:
            case CP_UTF16_BE:
                return 2u;
            case CP_UTF32:
            case CP_UTF32_BE:
                return 4;
        };
    };

    std::size_t OutputBuffer::NulTerminatorSpace (int cp, std::size_t offset) {
        offset -= NulTerminatorLength (cp);
        
        switch (cp) {
            default:
                return offset;
            case CP_UTF16:
            case CP_UTF16_BE:
                return offset & ~1;
            case CP_UTF32:
            case CP_UTF32_BE:
                return offset & ~3;
        };
    };

    std::size_t Encoder::Size () const {
        switch (this->codepage) {
            default:
                return WideCharToMultiByte (this->codepage, 0u,
                                            this->string, (int) this->length,
                                            NULL, 0, NULL, NULL);
                
            case CP_UTF16:
            case CP_UTF16_BE:
                return this->length * sizeof (char16_t);
                
            case CP_UTF32:
            case CP_UTF32_BE:
                std::size_t surrogates = 0u;
                
                for (std::size_t i = 0u; i != this->length; ++i) {
                    if (this->string [i] >= 0xD800 && this->string [i] < 0xDC00) {
                        if (i + 1u != this->length) {
                            if (this->string [i + 1u] >= 0xDC00 && this->string [i + 1u] < 0xE000) {
                                ++surrogates;
                            };
                        };
                    };
                };
                return (this->length - surrogates) * sizeof (char32_t);
        };
    };

    bool Encoder::Encode (unsigned char *& buffer, std::size_t & max) const {
        bool result = true;
        auto n = this->length;
        auto p = this->string;
        
        switch (this->codepage) {
            
            case CP_UTF16: // no conversion
                n *= sizeof (char16_t);
                if (n > max) {
                    n = max;
                    result = false;
                };
                std::memcpy (buffer, p, n);
                buffer += n;
                max -= n;
                this->encoded = n;
                break;
                    
            case CP_UTF16_BE:
                if (n > max / sizeof (char16_t)) {
                    n = max / sizeof (char16_t);
                    result = false;
                };
                this->encoded = n;
                while (n--) {
                    *buffer++ = *p >> 8u;
                    *buffer++ = *p++ & 0xFF;
                };
                break;
                
            case CP_UTF32:
            case CP_UTF32_BE:
                while (n-- && max) {
                    char32_t c = *p++;
                    if (c >= 0xD800 && c < 0xDC00) { // high surrogate
                    
                        char16_t x = *p++;
                        if (x >= 0xDC00 && x < 0xE000) { // low surrogate
                        
                            c = 0x10000u + ((c - 0xD800) << 10) + ((x - 0xDC00));
                            
                        } else {
                            c = 0xFFFD; // not low surrogate, error
                        };
                    } else
                    if (c >= 0xDC00 && c < 0xE000) { // low surrogate, error
                        c = 0xFFFD;
                    };
                    
                    if (this->codepage == CP_UTF32_BE) {
                        c = Swap32 (c);
                    };
                    
                    std::memcpy (buffer, &c, sizeof c);
                    
                    buffer += sizeof c;
                    max -= sizeof c;
                    ++this->encoded;
                };
                
                result = !n;
                break;

            default:
                auto w = WideCharToMultiByte (this->codepage, 0u,
                                              this->string, (int) this->length,
                                              (LPSTR) buffer, (int) max, NULL, NULL);
                if (w) {
                    buffer += w;
                    max -= w;
                    this->encoded = w;
                } else {
                    if (this->codepage != CP_SYMBOL // documented
                            && GetLastError () == ERROR_INSUFFICIENT_BUFFER) {
                        
                        this->encoded = max;
                        buffer += max;
                        max = 0u;
                    };
                    result = false;
                };
                break;
        };

        return result;
    };
};

bool Windows::SetDefaultPrintCodePage (int cp) { return SetDefaultCodePage (default_cp_print, cp); };
bool Windows::SetDefaultErrPrintCodePage (int cp) { return SetDefaultCodePage (default_cp_error, cp); };
bool Windows::SetDefaultSPrintCodePage (int cp) { return SetDefaultCodePage (default_cp_string, cp); };
bool Windows::SetDefaultWSAPrintCodePage (int cp) { return SetDefaultCodePage (default_cp_socket, cp); };
bool Windows::SetDefaultFilePrintCodePage (int cp) { return SetDefaultCodePage (default_cp_file, cp); };
bool Windows::SetDefaultPipePrintCodePage (int cp) { return SetDefaultCodePage (default_cp_pipe, cp); };

Windows::PR <void>
Windows::FilePrintCPVA (void * handle, unsigned int cp, const wchar_t * format, std::va_list va) {
    if (ResolvePrintCP (handle, cp)) {
        OutputHandle output (handle, cp);
        Printer printer (Input (format, va), output);
        printer ();
        
        return { printer.success, nullptr, printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
        
    } else {
        OutputConsole output (handle);
        Printer printer (Input (format, va), output);
        printer ();
        
        return { printer.success, nullptr, printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
    };
};

Windows::PR <void>
Windows::FilePrintCPVA (void * handle, unsigned int cp, unsigned int format, std::va_list va) {
    if (ResolvePrintCP (handle, cp)) {
        OutputHandle output (handle, cp);
        Printer printer (Input (format, va), output);
        printer ();

        return { printer.success, nullptr, printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
        
    } else {
        OutputConsole output (handle);
        Printer printer (Input (format, va), output);
        printer ();

        return { printer.success, nullptr, printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
    };
};

Windows::PR <void>
Windows::WSAPrintCPVA (unsigned int socket, unsigned int cp,
                       const wchar_t * format, std::va_list va) {
    OutputHandle output (reinterpret_cast <void *> ((std::uintptr_t) socket),
                         CurrentWSAPrintCP (cp));
    Printer printer (Input (format, va), output);
    printer ();
    
    return { printer.success, nullptr, printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
};

Windows::PR <void>
Windows::WSAPrintCPVA (unsigned int socket, unsigned int cp,
                       unsigned int format, std::va_list va) {
    OutputHandle output (reinterpret_cast <void *> ((std::uintptr_t) socket),
                         CurrentWSAPrintCP (cp));
    Printer printer (Input (format, va), output);
    printer ();
    
    return { printer.success, nullptr, printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
};

Windows::PR <char>
Windows::SPrintCPVA (void * buffer, std::size_t size, unsigned int cp,
                     const wchar_t * format, std::va_list va) {
    // if (!size) return ...
    
    OutputBuffer output (buffer, size, cp);
    Printer printer (Input (format, va), output);
    printer ();

    return { printer.success, static_cast <char *> (buffer), printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
};

Windows::PR <char>
Windows::SPrintCPVA (void * buffer, std::size_t size, unsigned int cp,
                     unsigned int format, std::va_list va) {
    OutputBuffer output (buffer, size, cp);
    Printer printer (Input (format, va), output);
    printer ();
    
    return { printer.success, static_cast <char *> (buffer), printer.GetCurrentFormatPointer (), printer.n_ops, output.n_out };
};


// CodePage and VA_LIST forwarders
//  - XxxPrint (Ppp, ...) -> XxxPrintVA (Ppp, va_args)
//  - XxxPrintVA (Ppp, va_list) -> XxxPrintCPVA (Ppp, -1, va_args)
//  - XxxPrintCP (Ppp, cp, ...) -> XxxPrintCPVA (Ppp, cp, va_args)

#define DEFINE_BASE0_FORWARDERS(rtype,name,ftype) \
    Windows::PR <rtype> Windows::name (ftype format, ...) {                     \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::name ## VA (format, args);                       \
        va_end (args);                                                          \
        return result;                                                          \
    };                                                                          \
    Windows::PR <rtype> Windows::name ## VA (ftype format, std::va_list va) {   \
        return Windows::name ## CPVA (-1, format, va);                          \
    };                                                                          \
    Windows::PR <rtype> Windows::name ## CP (unsigned int cp, ftype format, ...) { \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::name ## CPVA (cp, format, args);                 \
        va_end (args);                                                          \
        return result;                                                          \
    }

#define DEFINE_BASE1_FORWARDERS(rtype,p0type,name,ftype) \
    Windows::PR <rtype> Windows::name (p0type p0, ftype format, ...) {          \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::name ## VA (p0, format, args);                   \
        va_end (args);                                                          \
        return result;                                                          \
    };                                                                          \
    Windows::PR <rtype> Windows::name ## VA (p0type p0, ftype format, std::va_list va) { \
        return Windows::name ## CPVA (p0, -1, format, va);                      \
    };                                                                          \
    Windows::PR <rtype> Windows::name ## CP (p0type p0, unsigned int cp, ftype format, ...) { \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::name ## CPVA (p0, cp, format, args);             \
        va_end (args);                                                          \
        return result;                                                          \
    }

#define DEFINE_BASE2S_FORWARDERS(rtype,p0type,p1type,name,ftype) \
    Windows::PR <rtype> Windows::name (p0type p0, p1type p1, ftype format, ...) { \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::name ## VA (p0, p1, format, args);               \
        va_end (args);                                                          \
        return result;                                                          \
    };                                                                          \
    Windows::PR <rtype> Windows::name ## VA (p0type p0, p1type p1, ftype format, std::va_list va) { \
        auto r = Windows::name ## CPVA (p0, p1, CP_UTF16, format, va);            \
        return { r.success, reinterpret_cast <rtype *> (r.buffer), r.next, r.n, r.size }; \
    };                                                                          \
    Windows::PR <char> Windows::name ## CP (void * p0, p1type p1, unsigned int cp, ftype format, ...) { \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::name ## CPVA (p0, p1, cp, format, args);         \
        va_end (args);                                                          \
        return result;                                                          \
    }

#define DEFINE_STRING_CP_FORWARDERS(outtype,cp,ftype) \
    Windows::PR <outtype> Windows::SPrint (outtype * buffer, std::size_t length, ftype format, ...) { \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto r = Windows::SPrintCPVA (buffer, length, cp, format, args);        \
        va_end (args);                                                          \
        return { r.success, reinterpret_cast <outtype *> (r.buffer), r.next, r.n, r.size }; \
    };                                                                          \
    Windows::PR <outtype> Windows::SPrintVA (outtype * buffer, std::size_t length, ftype format, std::va_list va) { \
        auto r = Windows::SPrintCPVA (buffer, length, cp, format, va);          \
        return { r.success, reinterpret_cast <outtype *> (r.buffer), r.next, r.n, r.size }; \
    }

DEFINE_BASE0_FORWARDERS (void, Print, const wchar_t *);
DEFINE_BASE0_FORWARDERS (void, Print, unsigned int);
DEFINE_BASE0_FORWARDERS (void, ErrPrint, const wchar_t *);
DEFINE_BASE0_FORWARDERS (void, ErrPrint, unsigned int);
DEFINE_BASE1_FORWARDERS (void, void *, FilePrint, const wchar_t *);
DEFINE_BASE1_FORWARDERS (void, void *, FilePrint, unsigned int);
DEFINE_BASE1_FORWARDERS (void, unsigned int, WSAPrint, const wchar_t *);
DEFINE_BASE1_FORWARDERS (void, unsigned int, WSAPrint, unsigned int);
DEFINE_BASE2S_FORWARDERS (wchar_t, wchar_t *, std::size_t, SPrint, const wchar_t *);
DEFINE_BASE2S_FORWARDERS (wchar_t, wchar_t *, std::size_t, SPrint, unsigned int);
DEFINE_STRING_CP_FORWARDERS (char16_t, CP_UTF16, const wchar_t *);
DEFINE_STRING_CP_FORWARDERS (char16_t, CP_UTF16, unsigned int);
DEFINE_STRING_CP_FORWARDERS (char32_t, CP_UTF32, const wchar_t *);
DEFINE_STRING_CP_FORWARDERS (char32_t, CP_UTF32, unsigned int);

Windows::PR <void>
Windows::PrintCPVA (unsigned int cp, const wchar_t * format, std::va_list va) {
    // TODO: ability to redirect
    return Windows::FilePrintCPVA (GetStdHandle (STD_OUTPUT_HANDLE),
                                   cp, format, va);
};
Windows::PR <void>
Windows::PrintCPVA (unsigned int cp, unsigned int format, std::va_list va) {
    // TODO: ability to redirect
    return Windows::FilePrintCPVA (GetStdHandle (STD_OUTPUT_HANDLE),
                                   cp, format, va);
};
Windows::PR <void>
Windows::ErrPrintCPVA (unsigned int cp, const wchar_t * format, std::va_list va) {
    // TODO: ability to redirect
    return Windows::FilePrintCPVA (GetStdHandle (STD_ERROR_HANDLE),
                                   CurrentErrorPrintCP (cp), format, va);
};
Windows::PR <void>
Windows::ErrPrintCPVA (unsigned int cp, unsigned int format, std::va_list va) {
    // TODO: ability to redirect
    return Windows::FilePrintCPVA (GetStdHandle (STD_ERROR_HANDLE),
                                   CurrentErrorPrintCP (cp), format, va);
};

Windows::PR <char>
Windows::SPrint (char * buffer, std::size_t size, const wchar_t * format, ...) {
    va_list args;
    va_start (args, format);
    
    auto result = Windows::SPrintCPVA (buffer, size, CurrentCharSPrintCP (), format, args);
    
    va_end (args);
    return result;
};
Windows::PR <char>
Windows::SPrint (char * buffer, std::size_t size, unsigned int format, ...) {
    va_list args;
    va_start (args, format);
    
    auto result = Windows::SPrintCPVA (buffer, size, CurrentCharSPrintCP (), format, args);
    
    va_end (args);
    return result;
};

Windows::PR <char>
Windows::SPrintVA (char * buffer, std::size_t size, const wchar_t * format, std::va_list va) {
    return Windows::SPrintCPVA (buffer, size, CurrentCharSPrintCP (), format, va);
};
Windows::PR <char>
Windows::SPrintVA (char * buffer, std::size_t size, unsigned int format, std::va_list va) {
    return Windows::SPrintCPVA (buffer, size, CurrentCharSPrintCP (), format, va);
};

