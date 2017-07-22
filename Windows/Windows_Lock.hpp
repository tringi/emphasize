#ifndef WINDOWS_LOCK_HPP
#define WINDOWS_LOCK_HPP

/* Windows Lock 
// Windows_Lock.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.1
//
// Changelog:
//      03.05.2015 - initial version
*/

#include <windows.h>

#ifdef __MINGW_VERSION
#if WINVER >= 0x0600
extern "C" { // missing in MinGW (4.8.1, 32-bit, standard distro)
WINBASEAPI VOID WINAPI AcquireSRWLockShared (void **);
WINBASEAPI VOID WINAPI AcquireSRWLockExclusive (void **);
WINBASEAPI VOID WINAPI ReleaseSRWLockShared (void **);
WINBASEAPI VOID WINAPI ReleaseSRWLockExclusive (void **);
}
#endif
#endif

namespace Windows {
    
    // Lock
    //  - SRW/CS abstraction for simplest non-recursive locking
    
    class Lock {
#if WINVER >= 0x0601
        private:
#ifdef __MINGW_VERSION
            void * srw;
#else
            SRWLOCK srw;
#endif
        public:
            Lock () : srw {nullptr} {};
            
        public:
            void AcquireShared () { AcquireSRWLockShared (&this->srw); };
            void ReleaseShared () { ReleaseSRWLockShared (&this->srw); };
            
            void AcquireExclusive () { AcquireSRWLockExclusive (&this->srw); };
            void ReleaseExclusive () { ReleaseSRWLockExclusive (&this->srw); };

            bool TryAcquireShared () { return TryAcquireSRWLockShared (&this->srw); };
            bool TryAcquireExclusive () { return TryAcquireSRWLockExclusive (&this->srw); };

            bool Upgrade () {
                this->ReleaseShared ();
                this->AcquireExclusive ();
                return false;
            };
            bool Upgrade (bool * atomic) {
                bool atomically_upgraded = this->Upgrade ();
                if (atomic) {
                    *atomic = atomically_upgraded;
                };
                return true;
            };
            
            bool attempt () { return this->TryAcquireExclusive (); };
            void acquire () { return this->AcquireExclusive (); };
            void release () { return this->ReleaseExclusive (); };
        
#else
        private:
            CRITICAL_SECTION cs;
        public:
            Lock () { InitializeCriticalSection (&this->cs); };
            ~Lock () { DeleteCriticalSection (&this->cs); };
        public:
            void AcquireShared () { return this->acquire (); };
            void ReleaseShared () { return this->release (); };
            
            void AcquireExclusive () { return this->acquire (); };
            void ReleaseExclusive () { return this->release (); };

            bool TryAcquireShared () { return TryEnterCriticalSection (&this->cs); };
            bool TryAcquireExclusive () { return TryEnterCriticalSection (&this->cs); };

            bool Upgrade () { return true; };
            bool Upgrade (bool * atomic) {
                if (atomic) {
                    *atomic = true;
                };
                return true;
            };
            
            bool attempt () { return TryEnterCriticalSection (&this->cs); };
            void acquire () { EnterCriticalSection (&this->cs); };
            void release () { LeaveCriticalSection (&this->cs); };
#endif
        
        private:
            Lock (const Lock &) = delete;
            Lock & operator = (const Lock &) = delete;
    };
};

#endif

