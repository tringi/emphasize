#ifndef WINDOWS_PRINT_HPP
#define WINDOWS_PRINT_HPP

/* Emphasize Text Output Print function
// Windows_Print.hpp
//
// Author: Jan Ringos, http://Tringi.TrimCore.cz, jan@ringos.cz
// Version: 1.9
//
// Changelog:
//      28.05.2011 - initial version
//      16.11.2011 - added %vs and %M
//      25.03.2012 - added %hs, %D, %T and %Z
//      18.09.2012 - added %?ki
//      30.11.2012 - added %B and %b
//      09.07.2013 - added %IP and %G
//      xx.02.2017 - formerly "Resources::Printf" rewritten to "Windows::Print"
//
// Known issues:
//      - none of the extended string conventions support string size modifiers
*/

#include <cstddef>
#include <cstdarg>

#ifndef CP_UTF16
#define CP_UTF16 1200
#endif
#ifndef CP_UTF16_BE
#define CP_UTF16_BE 1201
#endif
#ifndef CP_UTF32
#define CP_UTF32 12000
#endif
#ifndef CP_UTF32_BE
#define CP_UTF32_BE 12001
#endif

#ifndef LOCALE_INVARIANT
#if WINVER >= 0x0600
#define LOCALE_INVARIANT 0x007f
#else
#define LOCALE_INVARIANT LANG_ENGLISH
#endif
#endif

namespace Windows {
    
    // [Err|File|WSA|S]Print[CP][VA]
    //  - formats w[s|v]nprintf string with additional extensions
    
    //  - prefix: [] - prints into default output (console) handle
    //            Err - prints into default error handle
    //            File - prints into any generic handle, first (void*) argument
    //            WSA - prints into socket passed as first argument
    //            S - prints into buffer, first argument is pointer, second is
    //                size of the buffer in BYTES
    //  - suffix: [] - default codepage, variable parameters
    //            CP - codepage, see below, is parameter of the function that
    //                 folows handle/socket
    //            VA - va_list instead of variable arguments as a last parameter

    //  - general conversion convention format is:
    //     % standard-opts [ optional-spec ] proprietary-opts type
    // 
    //    NOTES:
    //     - optional-spec - opening and closing square brackets must match
    
    //  - DEVIATIONS FROM THE STANDARD 'printf':
    //     - use %llf instead of %Lf which is now for localized float formatting
    
    //  - for integer values (decimal, octal and hexadecimal) and also for %Z
    //     - h - truncates to lower 16 bits before using/displaying
    //     - hh - truncates to lower 8 bits before using/displaying
    //     - hhh - truncates to lower 4 bits before using/displaying
    //     - hhhh - truncates to lowest 1 bit before using/displaying
    //     - ? - displays nothing when 0
    //     - v/V - reads version integer, parameter is number, letter or string
    //           - all syntax is recognized, number, letter or string (wide or
    //             narrow), i.e.: 0,'0','M','m',"0","M","m",L"0",L"M",L"m" where
    //             0/m yields major, 1/n minor, 2/r release and 3/b yields build
    //             version number or the current module
    //           - prepend spec/value with 'p' or 'P' and pass HMODULE before
    //             the value argument to get version number of the HMODULE
    
    //  - decimal integer values (%u, %i, %d)
    //     - k/K - displays as hexadecimal when over 99999 or less than -61440
    //           - k letter case implies case of hexadecimal letters
    //     - $ - displays value as currency
    //         - explicit precision for currency conversion is ignored
    //         - %*.*$d - requires 3 parameters
    //                  - Windows::Print (L"%*.*$d", 8, 2, value);
    //         - %*.*L$d - requires 4 parameters, LCID is third, value is last
    //                   - Windows::Print (L"%*.*L$d", 8, 2, 0x0405, value);
    //     - L - displays as localized number
    //         - %*.*Ld - requires 3 parameters
    //                  - Windows::Print (L"%*.*Ld", 8, 2, value);
    //         - %*.*LLd - requires 4 parameters, LCID is third, value is last
    //                   - Windows::Print (L"%*.*LLd", 8, 2, 0x0405, value);
    //     - B - displays value as nice short size with proper unit (kB, MB,...)
    //         - the value is limited to 3 characters including optional comma
    //         - a localized text "bytes" is appended to values smaller than 922
    //         - %*.*Bd - requires 3 parameters
    //                  - Windows::Print (L"%*.*Bd", 8, 2, value);
    //         - %*.*LBd - requires 4 parameters, LCID is third, value is last
    //                   - Windows::Print (L"%*.*LBd", 8, 2, 0x0405, value);
    //                   - LCID parameter chooses translation of "bytes" string
    //                     and number formatting (same as for 'L' above) locale
    //                   - using LOCALE_INVARIANT yields "B" instead of "bytes"
    //         - %#Bi - # for alternative form uses IEC units: KiB, MiB, GiB,...
    
    //  - floating point extensions and changes (%f etc.)
    //     - IMPORTANT: use "ll" for 'long double' parameter, 'L' means local
    //     - most of the following extensions can be combined
    //     - ? - displays nothing on NaN
    //     - U - displays +Inf, -Inf and NaN as unicode symbol(s)
    //           and longer - for negative values
    //     - $ - displays value as currency
    //         - %*.*$f - requires 3 parameters
    //                  - Windows::Print (L"%*.*$f", 8, 2, value);
    //         - %*.*L$f - requires 4 parameters, LCID is third, value is last
    //                   - Windows::Print (L"%*.*L$f", 8, 2, 0x0405, value);
    //     - L - displays value as localized number
    //         - %*.*Lf - requires 3 parameters
    //                  - Windows::Print (L"%*.*Lf", 8, 2, value);
    //         - %*.*LLf - requires 4 parameters, LCID is third, value is last
    //                   - Windows::Print (L"%*.*LLf", 8, 2, 0x0405, value);
    //     - Q - displays 'nice' number i.e. detects visually good precision
    //         - precision (default 3, max 16) tells how long sequence of '0' or
    //           '9' in '%f' rendering of the number to consider as long enough
    //           for the string representation to be terminated at that point
    
    //  - character extensions (%c)
    //     - code-points FFFF, FEFF and FFFE will not print anything
    //     - processed as 1-character string with all options, see below
    //     - %hc or %[852]c - single-byte char-set characters
    //                      - can contain up to 4 characters, and are processed
    //                        as strings of such characters
    
    //  - string extensions (%s)
    //     - source codepage/character set can be provided as a [spec] string,
    //       e.g.: %[utf-8]s, numeric %[852]s, or as an string/integer argument
    //       when written as %[*]s (3rd argument when used with precision/width
    //       as: %*.*[*]s), when not provided, following are supported:
    //        - %hs = CP_ACP
    //        - %lls = UTF-32
    //        - default is UTF-16
    //     - r/R - load string from executable resources by integer identifier
    //           - default when the argument is an integer smaller than 65536
    //           - if the whole resources group of 16 strings is missing then
    //             [spec] string is output instead, unless it is [*], then next
    //             argument is read which can be either string or different
    //             resource string identifier
    //        - + - sets string ID base, i.e. ("%+256rs", 1) will get string 257
    //            - used as last numeric parameter: "%16.4+256rs"
    //        - r - lowercase implies using current module
    //        - R - reads and uses HMODULE argument to identify string source
    //        - L - can be used, to select language from the module
    //            - e.g.: "%LRs", HMODULE, LANGID, string-id
    //     - v/V - loads string from executable or HMODULE version information
    //        - [spec] string can specify string name, e.g. "ProductName" or
    //          index; [*] means default: read the value from an argument
    //        - V/L - uppercase V and L work same as R/L for resource strings
    //              - e.g.: "%*.*[*]LVs", width, precission, HMODULE, LANGID, s
    //     - k - translates error code to string through Resources::ErrorMessage
    //         - "%#ks" is shorthand for "%ki: %ks"
    //         - if the error message is not available then [spec] string is
    //           output instead, unless it's [*], in that case next argument is
    //           read which can be either string or resource string identifier
    //     - ? - displays nothing when the argument is NULL or 0
    //     - adding ... after width, e.g. "%.10...s" will truncate with elipsis
    //        - adding 'U' will truncate with single unicode elipsis character
    //     - NOTE: for 'width' a surrogate pair is considered a single character
    
    //  - date/time format
    //     - % [+|-] [LL|L] [K] [D|T]
    //     - plus - long date or 24h time (can be used with minus for time)
    //     - minus - short date format, no-seconds time format
    //     - local formatting: - L - use current locale
    //                         - LL - load LCID argument
    //                         - if [xx] string is present, xx is used as format
    //                             - use "%[db]D %[db]T" for database format
    //                               "yyyy-MM-dd HH:mm:ss"
    //                         - otherwise locale is neutral
    //     - K - do not consume another argument, use current system time
    //     - argument: - pointer to SYSTEMTIME 
    //                 - NULL to use current local time
    
    //  - zero terminating the string
    //     - % Z
    //     - terminates the string at that point if the parameter is nonzero
    //       displaying the optional text in square brackets
    //        - reverse logic using - character, i.e.: "%-Z"
    //     - the parameter is of type 'unsigned int' but following standard
    //       modifiers apply: h, hh, hhh, hhhh, l, ll, z, t, j, I64
    
    //  - binary data
    //     - % [h] [Q] [B|b] p
    //     - displays binary data in hexadecimal representation
    //     - h - bytes with ASCII values of 0..9, a..z or A..Z are displayed 
    //           directly with "'" prefix
    //     - Q - 
    //     - B|b - distinguishes uppercase and lowercase hexadecimal letters
    //     - arguments: pointer to the data to display
    //                  std::size_t length of the data
    //     - precission (1 (default) or larger) specifies byte separation
    //     - width 
    
    //  - IP
    //  - GUID
    //  - console

    //  - GetLastError
    //            - ERROR_NO_UNICODE_TRANSLATION
    
    // TODO: APrint -> Windows::String?
    
    // SetPrintPreferredLanguage???
    
    // PR
    //  - Windows::Print results structure
    
    template <typename T>
    struct PR {
        bool success;       // true when everything succeeded
        T *  buffer;        // pointer to result buffer or nullptr
        
        const wchar_t * next;   // pointer after format string
        std::size_t     n;      // processed conversions
        std::size_t     size;   // written characters
        
        explicit operator bool () const { return this->success; };
    };
    
    // cp - ignored, when printing onto console,
    //    - -1 means default: UTF-16 is default by default

    PR <void> Print   (const wchar_t * format, ...);
    PR <void> PrintVA (const wchar_t * format, std::va_list);
    PR <void> PrintCP   (unsigned int cp, const wchar_t * format, ...);
    PR <void> PrintCPVA (unsigned int cp, const wchar_t * format, std::va_list);

    PR <void> Print   (unsigned int format, ...);
    PR <void> PrintVA (unsigned int format, std::va_list);
    PR <void> PrintCP   (unsigned int cp, unsigned int format, ...);
    PR <void> PrintCPVA (unsigned int cp, unsigned int format, std::va_list);

    PR <void> FilePrint   (void *, const wchar_t * format, ...);
    PR <void> FilePrintVA (void *, const wchar_t * format, std::va_list);
    PR <void> FilePrintCP   (void *, unsigned int cp, const wchar_t * format, ...);
    PR <void> FilePrintCPVA (void *, unsigned int cp, const wchar_t * format, std::va_list);
    
    PR <void> FilePrint   (void *, unsigned int format, ...);
    PR <void> FilePrintVA (void *, unsigned int format, std::va_list);
    PR <void> FilePrintCP   (void *, unsigned int cp, unsigned int format, ...);
    PR <void> FilePrintCPVA (void *, unsigned int cp, unsigned int format, std::va_list);
    
    PR <void> ErrPrint   (const wchar_t * format, ...);
    PR <void> ErrPrintVA (const wchar_t * format, std::va_list);
    PR <void> ErrPrintCP   (unsigned int cp, const wchar_t * format, ...);
    PR <void> ErrPrintCPVA (unsigned int cp, const wchar_t * format, std::va_list);

    PR <void> ErrPrint   (unsigned int format, ...);
    PR <void> ErrPrintVA (unsigned int format, std::va_list);
    PR <void> ErrPrintCP   (unsigned int cp, unsigned int format, ...);
    PR <void> ErrPrintCPVA (unsigned int cp, unsigned int format, std::va_list);

    PR <void> WSAPrint   (unsigned int, const wchar_t * format, ...);
    PR <void> WSAPrintVA (unsigned int, const wchar_t * format, std::va_list);
    PR <void> WSAPrintCP   (unsigned int, unsigned int cp, const wchar_t * format, ...);
    PR <void> WSAPrintCPVA (unsigned int, unsigned int cp, const wchar_t * format, std::va_list);

    PR <void> WSAPrint   (unsigned int, unsigned int format, ...);
    PR <void> WSAPrintVA (unsigned int, unsigned int format, std::va_list);
    PR <void> WSAPrintCP   (unsigned int, unsigned int cp, unsigned int format, ...);
    PR <void> WSAPrintCPVA (unsigned int, unsigned int cp, unsigned int format, std::va_list);
    
    // UTF-16 LE or whatever CP is defined
    PR <wchar_t> SPrint   (_Out_writes_z_ (n) wchar_t *, std::size_t n, const wchar_t * format, ...);
    PR <wchar_t> SPrintVA (_Out_writes_z_ (n) wchar_t *, std::size_t n, const wchar_t * format, std::va_list);
    PR <char> SPrintCP   (void *, std::size_t, unsigned int cp, const wchar_t * format, ...);
    PR <char> SPrintCPVA (void *, std::size_t, unsigned int cp, const wchar_t * format, std::va_list);
    
    PR <wchar_t> SPrint   (_Out_writes_z_ (n) wchar_t *, std::size_t n, unsigned int format, ...);
    PR <wchar_t> SPrintVA (_Out_writes_z_ (n) wchar_t *, std::size_t n, unsigned int format, std::va_list);
    PR <char> SPrintCP   (void *, std::size_t, unsigned int cp, unsigned int format, ...);
    PR <char> SPrintCPVA (void *, std::size_t, unsigned int cp, unsigned int format, std::va_list);

    template <unsigned long long N> PR <wchar_t> SPrint (_Out_writes_z_ (N) wchar_t (&) [N], const wchar_t * format, ...);
    template <unsigned long long N> PR <wchar_t> SPrintVA (_Out_writes_z_ (N) wchar_t (&) [N], const wchar_t * format, std::va_list);
    template <unsigned long long N> PR <wchar_t> SPrintVA (_Out_writes_z_ (N) wchar_t (&) [N], unsigned int format, std::va_list);

    // CP_ACP if not overriden by SetDefaultSPrintCodePage or SetDefaultPrintCodePage
    PR <char> SPrint   (_Out_writes_z_ (n) char *, std::size_t n, const wchar_t * format, ...);
    PR <char> SPrint   (_Out_writes_z_ (n) char *, std::size_t n, unsigned int format, ...);
    PR <char> SPrintVA (_Out_writes_z_ (n) char *, std::size_t n, const wchar_t * format, std::va_list);
    PR <char> SPrintVA (_Out_writes_z_ (n) char *, std::size_t n, unsigned int format, std::va_list);

    template <unsigned long long N> PR <char> SPrint   (_Out_writes_z_ (N) char (&) [N], const wchar_t * format, ...);
    template <unsigned long long N> PR <char> SPrintVA (_Out_writes_z_ (N) char (&) [N], const wchar_t * format, std::va_list);
    template <unsigned long long N> PR <char> SPrintVA (_Out_writes_z_ (N) char (&) [N], unsigned int format, std::va_list);
    
    // UTF-16 LE
    PR <char16_t> SPrint   (_Out_writes_z_ (n) char16_t *, std::size_t n, const wchar_t * format, ...);
    PR <char16_t> SPrint   (_Out_writes_z_ (n) char16_t *, std::size_t n, unsigned int format, ...);
    PR <char16_t> SPrintVA (_Out_writes_z_ (n) char16_t *, std::size_t n, const wchar_t * format, std::va_list);
    PR <char16_t> SPrintVA (_Out_writes_z_ (n) char16_t *, std::size_t n, unsigned int format, std::va_list);

    template <unsigned long long N> PR <char16_t> SPrint   (_Out_writes_z_ (N) char16_t (&) [N], const wchar_t * format, ...);
    template <unsigned long long N> PR <char16_t> SPrintVA (_Out_writes_z_ (N) char16_t (&) [N], const wchar_t * format, std::va_list);
    template <unsigned long long N> PR <char16_t> SPrintVA (_Out_writes_z_ (N) char16_t (&) [N], unsigned int format, std::va_list);

    // UTF-32 LE
    PR <char32_t> SPrint   (_Out_writes_z_ (n) char32_t *, std::size_t n, const wchar_t * format, ...);
    PR <char32_t> SPrint   (_Out_writes_z_ (n) char32_t *, std::size_t n, unsigned int format, ...);
    PR <char32_t> SPrintVA (_Out_writes_z_ (n) char32_t *, std::size_t n, const wchar_t * format, std::va_list);
    PR <char32_t> SPrintVA (_Out_writes_z_ (n) char32_t *, std::size_t n, unsigned int format, std::va_list);

    template <unsigned long long N> PR <char32_t> SPrint   (_Out_writes_z_ (N) char32_t (&) [N], const wchar_t * format, ...);
    template <unsigned long long N> PR <char32_t> SPrintVA (_Out_writes_z_ (N) char32_t (&) [N], const wchar_t * format, std::va_list);
    template <unsigned long long N> PR <char32_t> SPrintVA (_Out_writes_z_ (N) char32_t (&) [N], unsigned int format, std::va_list);

    // TODO: adding custom encodings?
    // TODO: Print (console) redirection (HANDLE / function / virt class)
    // TODO: ErrPrint redirection (HANDLE / function / virt class)
    // TODO: Print (class ptr, ...) - calls virtual function
    
    // SetDefault[Err|WSA|File|Pipe]PrintCodePage
    //  - 
    //  - NOTE: SetDefaultSPrintCodePage sets charset only for output to char*
    //  - NOTE: SetDefaultPipePrintCodePage
    //           - when shell forwarding as "program.exe | other.exe"
    //           - for explicit namedpipe handle
    //  - cp: -1 - use default
    //        0 - CP_ACP (8-bit)
    //        1 - CP_OEMCP (8-bit)
    //        2 - CP_MACCP (8-bit)
    //        3 - CP_THREAD_ACP (8-bit)
    //        1200 - UTF-16 LE (default)
    //        1201 - UTF-16 BE
    //        12000 - UTF-32 LE
    //        12001 - UTF-32 BE
    //        65000 - CP_UTF7
    //        65001 - CP_UTF8
    
    bool SetDefaultPrintCodePage (int cp = -1);
    bool SetDefaultSPrintCodePage (int cp = -1);
    bool SetDefaultErrPrintCodePage (int cp = -1);
    bool SetDefaultWSAPrintCodePage (int cp = -1);
    bool SetDefaultFilePrintCodePage (int cp = -1);
    bool SetDefaultPipePrintCodePage (int cp = -1);
    
    // TODO: SetPrintOutputBuffering
    //  - when enabled, all print calls except output to console will first
    //    construct complete string into a buffer and then output it as whole
    
    void SetPrintOutputBuffering (bool);
    
    // PrintBOM
    //  - 

    bool PrintBOM ();
};

#include "Windows_Print.tcc"
#endif

