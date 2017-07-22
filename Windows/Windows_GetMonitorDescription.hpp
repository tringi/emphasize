#ifndef WINDOWS_GETMONITORDESCRIPTION_HPP
#define WINDOWS_GETMONITORDESCRIPTION_HPP

/* Windows GetMonitorDescription 
// Windows_GetMonitorDescription.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
//
// Changelog:
//      29.03.2013 - initial version
//      30.05.2016 - merged improvements
*/

#include <windows.h>

namespace Windows {
    
    // Monitor
    //  - structure describing the requested monitor
    
    struct Monitor {
        HMONITOR        handle;     // NULL when not ATTACHED or ON
        unsigned int    index;      // absolute unique display device index
        DWORD           state;      // combination of DISPLAY_DEVICE_xxx flags
        wchar_t         device [32];// device name, e.g. "\\.\DISPLAY1"
        
        struct {
            wchar_t     device [128];   // GPU
            wchar_t     system [128];   // Windows API
            wchar_t     native [40];    // Monitor EDID name or text
            wchar_t     serial [40];    // Monitor EDID serial number
        } name;
        
        struct {
            unsigned short  code;       // 15-bit manufacturer code
            wchar_t         eisa [4];   // 3 character code: SAM/GSM/AUO/...
        } manufacturer;
        
        struct {
            unsigned short  code;
            unsigned int    serial;
        } product;
        
        unsigned short  diagonal;   // monitor diagonal in tenths of inches
        SIZE            size;       // monitor size in millimetes, as reported
        
        struct {
            SIZE        resolution; // current monitor resolution
            UINT        dpi;        // current monitor DPI (actual)
            RECT        workarea;   // current monitor work area
            RECT        rectangle;  // current monitor rectangle
            UINT        scaling;    // current horizontal scaling DPI
        } current;
        
        struct {
            SIZE resolution;        // native monitor resolution
            UINT dpi;               // native monitor DPI
        } native;
        
        union {
            unsigned char raw [128u];
            struct {
                unsigned char   magic [8];
                
                struct {
                    unsigned short  manufacturer;
                    unsigned short  product;
                } code;
                
                unsigned int    serial;
                
                struct {
                    unsigned char   week;
                    unsigned char   year;
                } manufactured;
                
                struct {
                    unsigned char major;
                    unsigned char minor;
                } version;
                
                
                // TODO
            };
        } edid;
    };
    
    // GetMonitorDescription
    //  - fills the Monitor structure (if not NULL) according to requested
    //    monitor (either by handle or by ordinal number)
    //  - returns: true if the Monitor info was (or would be) retrieved
    //             false if no such monitor exists or error occured
    //              - use GetLastError to find out the failure reason
    
    bool GetMonitorDescription (HMONITOR handle, Monitor * monitor);
    bool GetMonitorDescription (unsigned int i, Monitor * monitor);
    
    /// state flags

    // #define DISPLAY_DEVICE_ATTACHED_TO_DESKTOP   0x00000001
    // #define DISPLAY_DEVICE_MULTI_DRIVER          0x00000002
    // #define DISPLAY_DEVICE_PRIMARY_DEVICE        0x00000004
    // #define DISPLAY_DEVICE_MIRRORING_DRIVER      0x00000008
    // #define DISPLAY_DEVICE_VGA_COMPATIBLE        0x00000010
    // #define DISPLAY_DEVICE_REMOVABLE             0x00000020 // Windows 2K+
    // #define DISPLAY_DEVICE_ACC_DRIVER            0x00000040 // Windows 8+
    // #define DISPLAY_DEVICE_UNSAFE_MODES_ON       0x00080000 // Windows VI+
    // #define DISPLAY_DEVICE_TS_COMPATIBLE         0x00200000
    // #define DISPLAY_DEVICE_DISCONNECT            0x02000000 // Windows 2K+
    // #define DISPLAY_DEVICE_REMOTE                0x04000000 // Windows 2K+
    // #define DISPLAY_DEVICE_MODESPRUNED           0x08000000
    
    // #define DISPLAY_DEVICE_ACTIVE                0x00000001 // Windows 2K+
    // #define DISPLAY_DEVICE_ATTACHED              0x00000002 // Windows 2K+
};

#endif

