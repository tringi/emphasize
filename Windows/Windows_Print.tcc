#ifndef WINDOWS_PRINT_TCC
#define WINDOWS_PRINT_TCC

/* Windows Print 
// Windows_Print.tcc
//
// Author: Jan Ringos, http://Tringi.TrimCore.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      18.05.2015 - initial version
*/

#define DEFINE_WINDOWS_SPRINT_N_FORWARDERS(outtype,ftype) \
    template <unsigned long long N>                                             \
    Windows::PR <outtype> Windows::SPrint (outtype (&buffer) [N], ftype format, ...) { \
        va_list args;                                                           \
        va_start (args, format);                                                \
        auto result = Windows::SPrintVA (buffer, sizeof buffer, format, args);  \
        va_end (args);                                                          \
        return result;                                                          \
    };                                                                          \
    template <unsigned long long N>                                             \
    Windows::PR <outtype> Windows::SPrintVA (outtype (&buffer) [N],             \
                                      ftype format, std::va_list parameters) {  \
        return Windows::SPrintVA (buffer, sizeof buffer, format, parameters);   \
    }

DEFINE_WINDOWS_SPRINT_N_FORWARDERS (char, const wchar_t *);
DEFINE_WINDOWS_SPRINT_N_FORWARDERS (wchar_t, const wchar_t *);
DEFINE_WINDOWS_SPRINT_N_FORWARDERS (char16_t, const wchar_t *);
DEFINE_WINDOWS_SPRINT_N_FORWARDERS (char32_t, const wchar_t *);

#undef DEFINE_WINDOWS_SPRINT_N_FORWARDERS
#endif

