#include "Emphasize_Settings_Ini.hpp"

/* Emphasize Settings_Ini 
// Emphasize_Settings_Ini.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      17.05.2012 - initial version
*/

#include <windows.h>
#include <winioctl.h>
#include <shlobj.h>

#include <algorithm>
#include <cwctype>
#include <cstdio>

#include "../Windows/Windows_GetCurrentModuleHandle.hpp"
#include "../Windows/Windows_Heap_DuplicateString.hpp"

namespace {
    bool IsAbsolutePath (const wchar_t * path) /* __attribute__ ((pure)) */;
};

Emphasize::Settings::Ini::Ini (Settings & settings,
                               const wchar_t * _path, Mode _mode) noexcept
    :   Emphasize::Settings::Store (settings),
        mode (_mode),
        absp (IsAbsolutePath (_path)),
        orig (Windows::Heap::DuplicateString (_path)),
        path (this->absp
                ? this->orig
                : NULL),
        main (this->FindName (_path)) {};

Emphasize::Settings::Ini::~Ini () noexcept {
    if (this->orig) {
        HeapFree (GetProcessHeap (), 0, this->orig);
    };
    return;
};

bool Emphasize::Settings::Ini::Read () {
    if (!this->absp)
        this->path = this->Find (this->orig);
    
    if (this->path)
        return GetFileAttributes (this->path) != INVALID_FILE_ATTRIBUTES;
    else
        return false;
};

bool Emphasize::Settings::Ini::Write () {
    switch (this->mode) {
        
        // Only reading existing INI file
        
        case WriteExisting:
            if (!this->absp)
                this->path = this->Find (this->orig);
        
            break;
        
        // WriteAlways - Create the INI file
        //  - only supported for absolute paths
        //     - no point in complicated determination of proper file location
        //       when the application logic can provide that path
        //  - writes UNICODE BOM (to have the file in Unicode by default)
        //    and the main section name, to have it first and without initial
        //    blank line
        //     - that is, only IF the file did not existed before
    
        case WriteAlways:
            if (this->absp) {
                HANDLE h = CreateFile (this->path,
                                       GENERIC_READ | GENERIC_WRITE, 0,  NULL,
                                       CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
                if (h != INVALID_HANDLE_VALUE) {
                    
                    // set compression?
                    //  - for large INI files???
                    //  - failure of this DeviceIoControl can be safely ignored
                    //  - FSCTL_SET_COMPRESSION requires GENERIC_READ access
                    /*
                    DWORD result = 0u;
                    USHORT type = COMPRESSION_FORMAT_DEFAULT;
                    
                    DeviceIoControl (hFile, FSCTL_SET_COMPRESSION,
                                     &type, sizeof type, NULL, 0u, &result, NULL);
                    */
                    DWORD n;
                    if (WriteFile (h, "\xFF\xFE", 2u, &n, NULL)) {
                        WriteFile (h, L"[", 2u, &n, NULL);
                        WriteFile (h, this->main, (DWORD) (2u * std::wcslen (this->main)), &n, NULL);
                        WriteFile (h, L"]", 2u, &n, NULL);
                    };
                    CloseHandle (h);
                };
            };
            break;
    };
    
    return !!this->path;
};

void Emphasize::Settings::Ini::Close () {
    if (!this->absp && this->path) {
        HeapFree (GetProcessHeap (), 0, this->path);
        this->path = NULL;
    };
    return;
};

bool Emphasize::Settings::Ini::Get (const wchar_t * section, const wchar_t * name,
                                    void * data, std::size_t size) {
    if (size > MAXDWORD) {
        this->failure = ERROR_FILE_TOO_LARGE;
        return false;
    };

    if (section == NULL)
        section = this->main;
    
    if (this->IsPrivateProfileString (section, name)) {
        GetPrivateProfileString (section, name, L"",
                                 (LPTSTR) data, (DWORD) (size / sizeof (wchar_t)),
                                 this->path);
        this->failure = 0;
        return true;
    } else {
        this->failure = GetLastError ();
        return false;
    };
};

bool Emphasize::Settings::Ini::Set (const wchar_t * section, const wchar_t * name,
                                    const wchar_t * value) {
    if (section == NULL)
        section = this->main;
    
    if (this->mode == WriteAlways
            || this->IsPrivateProfileString (section, name)) {
        
        if (WritePrivateProfileString (section, name, value, this->path)) {
            this->failure = 0;
            return true;
        } else {
            this->failure = GetLastError ();
            return false;
        };
    } else
        return false;
};

bool Emphasize::Settings::Ini::Erase (const wchar_t * section,
                                      const wchar_t * name) {
    if (section == NULL)
        section = this->main;
    
    if (this->IsPrivateProfileString (section, name)) {
        if (WritePrivateProfileString (section, name, NULL, this->path)) {
            this->failure = 0;
            return true;
        } else {
            this->failure = GetLastError ();
            return false;
        };
    } else
        return false;
};

// IsPrivateProfileString
//  - returns true if file contains section/key pair, even if empty

bool Emphasize::Settings::Ini::IsPrivateProfileString (LPCTSTR section,
                                                       LPCTSTR key) {
    wchar_t buffer [8];
    buffer [0] = L'\xFDD0';
    buffer [1] = L'\0';
    
    GetPrivateProfileString (section, key, L"\xFDD0", buffer, 8, this->path);
    
    if (buffer [0] != L'\xFDD0' || buffer [1] != L'\0')
        return true;
    else
        return false;
};

wchar_t * Emphasize::Settings::Ini::Find (const wchar_t * name,
                                          wchar_t * buffer, unsigned int maximum) {
    wchar_t path [32768u];
    if (name) {
        
        // Attempt to locate file in current directory
        //  - to allow cases where exe is run from network
        //    but the working directory is set locally
        
        if (GetCurrentDirectory (sizeof path / sizeof path [0], path)) {
            std::wcscat (path, L"\\");
            std::wcscat (path, name);
            std::wcscat (path, L".ini");
            if (GetFileAttributes (path) != INVALID_FILE_ATTRIBUTES)
                goto found;
        };
        
        // Application directory
        //  - the most default location, search for appdir\'name'.ini

        if (GetModuleFileName (Windows::GetCurrentModuleHandle (),
                               path, sizeof path / sizeof path [0])) {
                    
            wchar_t * separator = std::wcsrchr (path, L'\\');
            if (!separator)
                separator = std::wcsrchr (path, L'/');
            
            if (separator) {
                separator [1] = L'\0';
                
                std::wcscat (path, name);
                std::wcscat (path, L".ini");
            
                if (GetFileAttributes (path) != INVALID_FILE_ATTRIBUTES)
                    goto found;
            };
        };

    };

    // Application directory
    //  - with the EXE file name

    if (GetModuleFileName (Windows::GetCurrentModuleHandle (),
                           path, sizeof path / sizeof path [0])) {
                
        std::size_t length = std::wcslen (path);
        if (length > 4u
            && std::towlower (path [length - 1u]) == L'e'
            && std::towlower (path [length - 2u]) == L'x'
            && std::towlower (path [length - 3u]) == L'e'
            && path [length - 4u] == L'.') {
            
            path [length - 1u] = L'i';
            path [length - 2u] = L'n';
            path [length - 3u] = L'i';
        } else {
            std::wcscat (path, L".ini");
        };
        
        if (GetFileAttributes (path) != INVALID_FILE_ATTRIBUTES)
            goto found;
    };
    
    if (name) {
        
        // Application user local directory
        //  - that is writable by user and common non-admin applications
        //  - the user may have his specific wishes for application startup
        //  - using Roaming profile instead of Local (CSIDL_LOCAL_APPDATA) to allow
        //    the ini file to be transported over the network if the user logs on
        //    to different computer in the domain

#if WINVER >= _WIN32_WINNT_VISTA
        PWSTR folder = NULL;
        if (SHGetKnownFolderPath (FOLDERID_RoamingAppData, 0, NULL, &folder) == S_OK) {
            std::wcscpy (path, folder);
            CoTaskMemFree (folder);
#else
        // not available starting Server 2016 Nano
        if (SHGetFolderPath (NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK) {
#endif
            std::wcscat (path, L"\\");
            std::wcscat (path, name);
            
            if (CreateDirectory (path, NULL)
                    || GetLastError () == ERROR_ALREADY_EXISTS) {
                
                std::wcscat (path, L"\\");
                std::wcscat (path, name);
                std::wcscat (path, L".ini");
                
                if (GetFileAttributes (path) != INVALID_FILE_ATTRIBUTES)
                    goto found;
            };
        };
    };
    
    // Not found
    //  - return NULL so no INI file is accessed, nor created if not exists
    
    return NULL;
    
    // Found
    //  - copy only neccessary bytes from stack to free store and finish
    //  - this small block is never released (only at application exit)
    
    found:
    
    if (buffer && maximum) {
        const auto length = std::wcslen (path) + 1u;
        if (length <= maximum) {
            return std::wmemcpy (buffer, path, length);
        } else
            return NULL;
    } else
        return Windows::Heap::DuplicateString (path);
};

wchar_t * Emphasize::Settings::Ini::FindName (const wchar_t * path) {
    if (path) {
        auto extension = std::wcsrchr (path, L'.');
        auto filename1 = std::wcsrchr (path, L'\\');
        auto filename2 = std::wcsrchr (path, L'/');
        auto filename = filename1 && filename2
                           ? filename1 > filename2
                                ? filename1 + 1
                                : filename2 + 1
                           : filename1 != nullptr
                                ? filename1 + 1
                                : filename2 != nullptr
                                    ? filename2 + 1
                                    : path;
    
        if (extension && (filename < extension))
            return Windows::Heap::DuplicateString (filename, (unsigned int) (extension - filename));
        else
            return Windows::Heap::DuplicateString (filename);
        
    } else {
        TCHAR module [32768u];
        if (GetModuleFileName (Windows::GetCurrentModuleHandle (),
                               module, sizeof module / sizeof module [0])) {
            return FindName (module);
        } else
            return NULL;
    };
};

wchar_t * Emphasize::Settings::Ini::ListKeyValues (const wchar_t * section) {
    TCHAR * buffer = NULL;

    if (this->Read ()) {
        if (section == NULL)
            section = this->main;

        auto heap = GetProcessHeap ();
        SIZE_T n = 0u;
        SIZE_T size = 127u;
        if ((buffer = (TCHAR *) HeapAlloc (heap, 0, size * sizeof (TCHAR)))) {

            while ((n = GetPrivateProfileSection (section, buffer, size, this->path)) >= (size - 2u)) {

                size *= 2u;
                if (TCHAR * newbuffer = (TCHAR *) HeapReAlloc (heap, 0, buffer, size * sizeof (TCHAR))) {
                    buffer = newbuffer;

                } else {
                    HeapFree (heap, 0, buffer);
                    buffer = NULL;
                    break;
                };
            };

            /*if (n < 3u) {
                HeapFree (heap, 0, buffer);
                buffer = NULL;
            };*/
        };

        this->Close ();
    };
    return buffer;
};

namespace {
    bool IsAbsolutePath (const wchar_t * path) {
        if (path)
            return (path [0] == L'\\')
                || (path [1] == L':' && path [2] == L'\\')
                || (path [1] == L':' && path [2] == L'/')
                ;
        else
            return false;
    };
};
