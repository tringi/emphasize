#include "Emphasize_Settings_Registry.hpp"

/* Emphasize Settings_Registry 
// Emphasize_Settings_Registry.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      17.05.2012 - initial version
*/

#include <cstdio>

Emphasize::Settings::Registry::Registry (Settings & settings,
                                         HKEY _parent, const wchar_t * _path)
    :   Emphasize::Settings::Store (settings),
        hkey (_parent),
        path (_path),
        handle (NULL) {};
        
bool Emphasize::Settings::Registry::Read () {
    return RegCreateKeyEx (this->hkey, this->path, 0u, NULL, 0,
                           KEY_READ, NULL, &this->handle, NULL)
        == ERROR_SUCCESS;
};
bool Emphasize::Settings::Registry::Write () {
    return RegCreateKeyEx (this->hkey, this->path, 0u, NULL, 0,
                           KEY_READ | KEY_WRITE,
                           NULL, &this->handle, NULL)
        == ERROR_SUCCESS;
};
void Emphasize::Settings::Registry::Close () {
    RegCloseKey (this->handle);
    this->handle = NULL;
    return;
};

Emphasize::Settings::Store::Type
Emphasize::Settings::Registry::Query (const wchar_t * subkey,
                                      const wchar_t * name,
                                      std::size_t * size) {
    DWORD rtype = 0u;
    DWORD rsize = 0u;
    HKEY hKey = NULL;
    
    if (subkey) {
        this->failure = RegCreateKeyEx (this->handle, subkey, 0u, NULL, 0,
                                        KEY_READ, NULL, &hKey, NULL);
        
        if (this->failure != ERROR_SUCCESS) {
            return Store::Missing;
        };
    } else
        hKey = this->handle;
    
    this->failure = RegQueryValueEx (hKey, name, NULL, &rtype, NULL, &rsize);
    
    if (subkey)
        RegCloseKey (hKey);
    
    if (this->failure == ERROR_SUCCESS) {
        *size = rsize;
        
        switch (rtype) {
            case REG_SZ: // 1
            case REG_EXPAND_SZ: // 2
                return Store::StringType;
                
            case REG_DWORD: // 4
            case REG_QWORD: // 11
                return Store::IntegerType;
                
            case REG_NONE: // 0
            case REG_BINARY: // 3
            case REG_DWORD_BIG_ENDIAN: // 5
            case REG_LINK: // 6
            case REG_MULTI_SZ: // 7
            case REG_RESOURCE_LIST: // 8
            case REG_FULL_RESOURCE_DESCRIPTOR: // 9
            case REG_RESOURCE_REQUIREMENTS_LIST: // 10
            default:
                return Store::BinaryType;
        };
    } else
        return Store::Missing;
};

bool Emphasize::Settings::Registry::Get (const wchar_t * subkey,
                                         const wchar_t * name,
                                         void * data, std::size_t size) {
    if (size > MAXDWORD) {
        this->failure = ERROR_FILE_TOO_LARGE;
        return false;
    };

    DWORD rsize = (DWORD) size;
    HKEY hKey = NULL;
    
    if (subkey) {
        this->failure = RegCreateKeyEx (this->handle, subkey, 0u, NULL, 0,
                                        KEY_READ, NULL, &hKey, NULL);
        
        if (this->failure != ERROR_SUCCESS) {
            return Store::Missing;
        };
    } else
        hKey = this->handle;

    this->failure = RegQueryValueEx (hKey, name, NULL, NULL,
                                     reinterpret_cast <BYTE *> (data), &rsize);
    if (subkey)
        RegCloseKey (hKey);

    return this->failure == ERROR_SUCCESS;
};

bool Emphasize::Settings::Registry::Set (const wchar_t * subkey,
                                         const wchar_t * name, long value) {
    return this->Write (subkey, name, REG_DWORD, &value, sizeof value);
};
bool Emphasize::Settings::Registry::Set (const wchar_t * subkey,
                                         const wchar_t * name, long long value) {
    return this->Write (subkey, name, REG_QWORD, &value, sizeof value);
};
bool Emphasize::Settings::Registry::Set (const wchar_t * subkey,
                                         const wchar_t * name,
                                         unsigned long value) {
    return this->Write (subkey, name, REG_DWORD, &value, sizeof value);
};
bool Emphasize::Settings::Registry::Set (const wchar_t * subkey,
                                         const wchar_t * name,
                                         unsigned long long value) {
    return this->Write (subkey, name, REG_QWORD, &value, sizeof value);
};
bool Emphasize::Settings::Registry::Set (const wchar_t * subkey,
                                         const wchar_t * name,
                                         const wchar_t * string) {
    return this->Write (subkey, name, REG_SZ,
                        string, (std::wcslen (string) + 1u) * sizeof (wchar_t));
};
bool Emphasize::Settings::Registry::Set (const wchar_t * subkey,
                                         const wchar_t * name,
                                         const void * data, std::size_t size) {
    return this->Write (subkey, name, REG_BINARY, data, size);
};

bool Emphasize::Settings::Registry::Write (const wchar_t * subkey,
                                           const wchar_t * name, DWORD type,
                                           const void * data, std::size_t size) {
    HKEY hKey = NULL;
    if (size > MAXDWORD) {
        this->failure = ERROR_FILE_TOO_LARGE;
        return false;
    };

    if (subkey) {
        this->failure = RegCreateKeyEx (this->handle, subkey, 0u, NULL, 0,
                                        KEY_WRITE, NULL, &hKey, NULL);
        
        if (this->failure != ERROR_SUCCESS) {
            return false;
        };
    } else
        hKey = this->handle;

    this->failure = RegSetValueEx (hKey, name, 0u, type,
                                   reinterpret_cast <const BYTE *> (data), (DWORD) size);
    if (subkey)
        RegCloseKey (hKey);
    
    return this->failure == ERROR_SUCCESS;
};

bool Emphasize::Settings::Registry::Erase (const wchar_t * subkey,
                                           const wchar_t * name) {
    HKEY hKey = NULL;
    
    if (subkey) {
        this->failure = RegCreateKeyEx (this->handle, subkey, 0u, NULL, 0,
                                        KEY_WRITE, NULL, &hKey, NULL);
        
        if (this->failure != ERROR_SUCCESS) {
            return false;
        };
    } else
        hKey = this->handle;

    this->failure = RegDeleteValue (hKey, name);
    
    if (subkey)
        RegCloseKey (hKey);
    
    return this->failure == ERROR_SUCCESS;
};
