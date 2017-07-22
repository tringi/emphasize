#include "Resources_String.hpp"

/* Emphasize Resources library String access routine
// Resources_String.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      28.05.2011 - initial version
*/

#include "Resources_Raw.hpp"
#include "../Windows/Windows_GetCurrentModuleHandle.hpp"
#include "../Windows/Windows_Heap_Vector.hpp"
#include "../Windows/Windows_Lock.hpp"

#include <cstring>
#include <cstdio>

namespace {
    
    // Descriptor
    //  - stores processed block of 16 strings copied out from resources
    
    struct LocalDescriptor {
        public:
            unsigned int    id;
            wchar_t *       data;
        public:
            LocalDescriptor () = default;
            LocalDescriptor (HMODULE, unsigned int id, wchar_t * data = nullptr) : id (id), data (data) {};
            LocalDescriptor (const LocalDescriptor &) = default;
            LocalDescriptor & operator = (const LocalDescriptor &) = default;
            
            inline bool match (HMODULE, unsigned int id) const {
                return this->id == id;
            };
    };

    struct GeneralDescriptor {
        public:
            HMODULE         module;
            unsigned int    marker; // language << 16u | id
            wchar_t *       data;
        public:
            GeneralDescriptor () = default;
            GeneralDescriptor (HMODULE module, unsigned int marker, wchar_t * data = nullptr) : module (module), marker (marker), data (data) {};
            GeneralDescriptor (const GeneralDescriptor &) = default;
            GeneralDescriptor & operator = (const GeneralDescriptor &) = default;

            inline bool match (HMODULE module, unsigned int marker) const {
                return this->module == module
                    && this->marker == marker;
            };
    };
    
    const wchar_t * BlockSubString (const wchar_t * block, unsigned int index) {
        if (!block)
            return L"";
        
        auto p = block;
        auto i = index;
        while (i) {
            if (*p == L'\0') {
                --i;
            };
            ++p;
        };
        return p;
    };

#ifdef __GCC__
#define EXPECT(a) __builtin_expect(a,1)
#else
#define EXPECT(a) a
#endif
    
    template <typename TD, unsigned int StaticSize>
    class Cache {
        private:
            Windows::Lock lock;
            Windows::Heap::Vector <TD, StaticSize> descriptors;
            
        private:
            const wchar_t * FindString (HMODULE module, unsigned int marker, unsigned int sub) const {
                const auto n = this->descriptors.Size ();
                
                for (auto i = 0u; i != n; ++i) {
                    if (this->descriptors [i] .match (module, marker)) {
                        return BlockSubString (this->descriptors [i] .data, sub);
                    };
                };
                
                return nullptr;
            };
            
        public:
            ~Cache () { this->descriptors.Release (); }; // exiting, no need to deallocate
            
            const wchar_t * Retrieve (HMODULE module, unsigned int id, unsigned short language) {
                const auto marker = (language << 16u) | ((id >> 4u) + 1u);
                const auto sub = id & 0x0Fu;

                this->lock.AcquireShared ();

                if (auto string = this->FindString (module, marker, sub)) {
                    this->lock.ReleaseShared ();
                    return string;
                };
                
                if (!this->lock.Upgrade ()) {
                
                    // upgrade was not atomic, race possible, check data again
                
                    if (auto string = this->FindString (module, marker, sub)) {
                        this->lock.ReleaseExclusive ();
                        return string;
                    };
                };

                const Resources::Raw raw (module, RT_STRING,
                                          MAKEINTRESOURCE ((id >> 4u) + 1u),
                                          language);
                if (EXPECT (raw.data)) {
                    auto heap = GetProcessHeap ();
                    auto block = static_cast <wchar_t *> (HeapAlloc (heap, 0, raw.size));
                    auto rdata = static_cast <const unsigned short *> (raw.data);
                    
                    if (EXPECT (block)) {
                        std::memcpy (block, rdata + 1u, raw.size - sizeof (unsigned short));
                        
                        unsigned int zeroindex = *rdata;
                        for (auto i = 0u; i != 16u; ++i) {
                            block [zeroindex] = L'\0';
                            zeroindex += rdata [zeroindex + 1u] + 1u;
                        };
                        
                        if (this->descriptors.Add (TD (module, marker, block))) {
                            this->lock.ReleaseExclusive ();
                            return BlockSubString (block, sub);
                            
                        } else {
                            this->lock.ReleaseExclusive ();
                            HeapFree (heap, 0, block);
                            SetLastError (ERROR_OUTOFMEMORY);
                            return L"\x2205";
                        };
                    } else {
                        this->lock.ReleaseExclusive ();
                        SetLastError (ERROR_OUTOFMEMORY);
                        return L"\x2205"; // empty set symbol
                    };
                } else {
                    this->descriptors.Add (TD (module, marker, nullptr));
                    this->lock.ReleaseExclusive ();
                    return L"";
                };
            };
            
            void Purge () {
                this->lock.AcquireExclusive ();
                
                const auto heap = GetProcessHeap ();
                const auto n = this->descriptors.Size ();
                
                for (auto i = 0u; i != n; ++i) {
                    if (this->descriptors [i] .data) {
                        HeapFree (heap, 0, this->descriptors [i] .data);
                    };
                };
                this->descriptors.Clear ();

                this->lock.ReleaseExclusive ();
                return;
            };
    };
    
    Cache <LocalDescriptor, 1024u>  primary_cache;
    Cache <GeneralDescriptor, 64u>  general_cache;
};

void Resources::ClearStringCache () {
    primary_cache.Purge ();
    return;
};

const wchar_t * Resources::String (HMODULE hModule, unsigned int id,
                                   unsigned int * length,
                                   unsigned short language) {
    if (id < 16u * 65535u) { // 1048560u; 0xFFFF0u;
        const Resources::Raw raw (hModule, MAKEINTRESOURCE (6u),
                                  MAKEINTRESOURCE ((id >> 4u) + 1u), language);
        if (raw.data) {
            const unsigned int index = id % 16u;
            
            const wchar_t * p = static_cast <const wchar_t *> (raw.data);
            unsigned int    s = static_cast <unsigned short > (*p++);
            
            for (unsigned int i = 0; i < index; ++i) {
                p += s;
                s = static_cast <unsigned short> (*p++);
            };
            
            *length = s;
            if (s)
                return p;
        };
    };
    return NULL;
};

const wchar_t * Resources::String (HMODULE hModule, unsigned int id,
                                   unsigned short language) {
    if (EXPECT ((language == 0u) && (hModule == Windows::GetCurrentModuleHandle())))
        return primary_cache.Retrieve (NULL, id, 0);
    else
        return general_cache.Retrieve (hModule, id, language);
};

const wchar_t * Resources::String (unsigned int id) {
    return primary_cache.Retrieve (NULL, id, 0);
};

const wchar_t * Resources::String (unsigned int id, unsigned short language) {
    if (EXPECT (language))
        return general_cache.Retrieve (Windows::GetCurrentModuleHandle (), id, language);
    else
        return primary_cache.Retrieve (NULL, id, 0);
};

const wchar_t * Resources::String (unsigned int id, unsigned int * length,
                                   unsigned short language) {
    const HMODULE hModule = Windows::GetCurrentModuleHandle (); 
    return Resources::String (hModule, id, length, language);
};


