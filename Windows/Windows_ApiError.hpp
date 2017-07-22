#ifndef WINDOWS_APIERROR_HPP
#define WINDOWS_APIERROR_HPP

/* Windows ApiError - exception thrown for API function fails in constructors
// Windows_ApiError.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.2
//
// Changelog:
//      25.04.2014 - initial version
//      14.09.2015 - removed introduction of <windows.h> in the header file
*/

#include <stdexcept>
#include <cstdarg>

namespace Windows {
    
    // ApiError
    //  - exception...
    //  - each class might create own exception inheriting from this one
    
    class ApiError
        : public std::runtime_error {
        
//        static_assert ('ABCD' == 0x41424344, "");
        public:
            const unsigned int code;
            const unsigned int component; // Windows/Resources/Emphasize/Shadow/... identifier, see table below
            const char * const call;
        private:
            va_list va; // TODO: how to make this temporary in constructor?
        public:
            explicit ApiError (unsigned int component = 0u, unsigned int code = ApiError::GetLastError ());
            
            explicit ApiError (const char * call, const char * parameter = nullptr, ...);
            explicit ApiError (unsigned int, const char * call, const char * parameter = nullptr, ...);
            explicit ApiError (unsigned int, unsigned int, const char * call, const char * parameter = nullptr, ...);
            
        private:
            static unsigned int GetLastError ();
//        public:
//            virtual const char * what () const noexcept (true) override;
    };
    
    // component codes table (DWORD so switch can be used (Emphasize::ID)):
    //  - 'WESD' - Windows::EveryoneSecurityDescriptor
    //  - 'WSCK' - Windows::WinSock
    //  - 'WIML' - Windows::ImageList
    //  - 'WLVI' - Windows::ListViewControlInterface
    //  - 'WVMB' - Windows::VirtualMemoryBuffer
    //  - 'WTP ' - Windows::ThreadPool

};

#endif

