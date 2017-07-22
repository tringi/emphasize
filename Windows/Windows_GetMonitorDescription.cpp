#include "Windows_GetMonitorDescription.hpp"

/* Windows GetMonitorDescription 
// Windows_GetMonitorDescription.cpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 1.1
//
// Changelog:
//      29.03.2013 - initial version
//      30.05.2016 - merged improvements
*/

#include <cstring>
#include <cstdio>
#include <cwchar>
#include <cmath>

#include "Windows_GetDesktopDPI.hpp"
#include "Windows_Symbol.hpp"

namespace {
    struct MonitorEnumParam {
        DISPLAY_DEVICE * dd;
        MONITORINFOEX    mi;
        HMONITOR         handle;
        bool             found;
    };
    
    BOOL CALLBACK MonitorEnumProc (HMONITOR, HDC, LPRECT, LPARAM);
    bool FillMonitorDescription (Windows::Monitor *, unsigned int, HMONITOR,
                                 const MONITORINFOEX *, const DISPLAY_DEVICE &);
    bool ExtractMonitorRegPath (const wchar_t * interfaceID,
                                wchar_t * path, unsigned int max);
    bool ExtractEdidText (BYTE * edid, BYTE code, wchar_t * out);
};

bool Windows::GetMonitorDescription (HMONITOR handle, Monitor * monitor) {
    
    MONITORINFOEX mi;
    mi.cbSize = sizeof mi;
    if (GetMonitorInfo (handle, &mi)) {
        
        DISPLAY_DEVICE dd;
        dd.cb = sizeof dd;
        
        unsigned int i = 0u;
        while (EnumDisplayDevices (NULL, i, &dd, 0)) {
            if (!std::wcsncmp (mi.szDevice, dd.DeviceName, CCHDEVICENAME)) {
                
                // fill 'monitor'
                //  - if NULL just say TRUE that such monitor exists
                
                if (monitor)
                    return FillMonitorDescription (monitor, i, handle, &mi, dd);
                else
                    return true;
            };
        };
    };
    
    return false;
};
bool Windows::GetMonitorDescription (unsigned int i, Monitor * monitor) {

    DISPLAY_DEVICE dd;
    dd.cb = sizeof dd;
    
    if (EnumDisplayDevices (NULL, i, &dd, 0)) {
        if (monitor) {
            MonitorEnumParam p;
            
            p.dd = &dd;
            p.mi.cbSize = sizeof p.mi;
            p.found = false;
            
            EnumDisplayMonitors (NULL, NULL, MonitorEnumProc,
                                 reinterpret_cast <LPARAM> (&p));
            if (p.found)
                return FillMonitorDescription (monitor, i, p.handle, &p.mi, dd);
            else
                return FillMonitorDescription (monitor, i, NULL, NULL, dd);
        } else
            return true;
    } else
        return false;
};

namespace {
    BOOL CALLBACK MonitorEnumProc (HMONITOR handle, HDC,
                                   LPRECT, LPARAM dwData) {
        auto * const param = reinterpret_cast <MonitorEnumParam *> (dwData);
        if (GetMonitorInfo (handle, &param->mi)) {
            if (!std::wcsncmp (param->mi.szDevice, param->dd->DeviceName, CCHDEVICENAME)) {
                param->found = true;
                param->handle = handle;
                return FALSE;
            } else
                return TRUE; // request next
        } else
            return FALSE;
    };

    bool FillMonitorDescription (Windows::Monitor * monitor,
                                 unsigned int index, HMONITOR handle,
                                 const MONITORINFOEX * mi,
                                 const DISPLAY_DEVICE & dd) {

        std::memset (monitor, 0, sizeof *monitor);

        // return all base known details

        monitor->handle = handle;
        monitor->index = index;
        monitor->state = dd.StateFlags;
        
        if (mi) {
            monitor->current.resolution.cx = mi->rcMonitor.right - mi->rcMonitor.left;
            monitor->current.resolution.cy = mi->rcMonitor.bottom - mi->rcMonitor.top;
            monitor->current.workarea = mi->rcWork;
            monitor->current.rectangle = mi->rcMonitor;
        };

        std::wcsncpy (monitor->device, dd.DeviceName, 32u);
        std::wcsncpy (monitor->name.device, dd.DeviceString, 128u);
        
        // NOTE: here we have dd.DeviceID and dd.DeviceKey for GPU
        
        // attempt retrieve more details
        
        DISPLAY_DEVICE ddd;
        ddd.cb = sizeof ddd;
        
        if (EnumDisplayDevices (dd.DeviceName, 0, &ddd, 0)) {
            
            monitor->state |= ddd.StateFlags;
            std::wcsncpy (monitor->name.system, ddd.DeviceString, 128u);

            // NOTE: here we have ddd.DeviceID and ddd.DeviceKey for Monitor

            DISPLAY_DEVICE dif;
            dif.cb = sizeof dif;

            HKEY hKey;
            wchar_t edid_path [2048];

            if (   EnumDisplayDevices (dd.DeviceName, 0, &dif, 1)
                && ExtractMonitorRegPath (dif.DeviceID, edid_path,
                                          sizeof edid_path / sizeof edid_path [0])
                && RegOpenKeyEx (HKEY_LOCAL_MACHINE, edid_path, 0,
                                 STANDARD_RIGHTS_READ | KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {

                // NOTE: here we have dif.DeviceID as interface for Monitor
            
                BYTE edid [32768]; // E-EDID size
                DWORD size = sizeof edid;
                
                if (RegQueryValueEx (hKey, L"EDID", NULL, NULL, edid, &size) == ERROR_SUCCESS) {
                    
                    std::memcpy (monitor->edid.raw, edid, sizeof monitor->edid.raw);
                    
                    unsigned short manufacturer = (edid [8] << 8) | edid [9];
                    
                    monitor->manufacturer.code = manufacturer;
                    monitor->manufacturer.eisa [0] = L'A' + ((manufacturer >> 10) & 0x1F) - 1;
                    monitor->manufacturer.eisa [1] = L'A' + ((manufacturer >>  5) & 0x1F) - 1;
                    monitor->manufacturer.eisa [2] = L'A' + ((manufacturer >>  0) & 0x1F) - 1;
                    monitor->manufacturer.eisa [3] = L'\0';
                    
                    monitor->product.code = (edid [11] << 8) | edid [10];
                    monitor->product.serial = (edid [15] << 24) | (edid [14] << 16) | (edid [13] << 8) | edid [12];
                    
                    monitor->size.cx = ((edid [54+14] >> 4) << 8) | edid [54+12];
                    monitor->size.cy = ((edid [54+14] &0xF) << 8) | edid [54+13];
                    
                    if (monitor->size.cx < 10 * edid [21]) monitor->size.cx = 10 * edid [21];
                    if (monitor->size.cy < 10 * edid [22]) monitor->size.cy = 10 * edid [22];

                    monitor->diagonal = std::sqrt (monitor->size.cx * monitor->size.cx
                                                 + monitor->size.cy * monitor->size.cy)
                                      / 2.54f
                                      + 0.5f;
                    
                    monitor->native.resolution.cx = ((edid [54+4] >> 4) << 8) | edid [54+2];
                    monitor->native.resolution.cy = ((edid [54+7] >> 4) << 8) | edid [54+5];
                    
                    // TODO: ignore string that equal "eisa" manufacturer name
                    // TODO: ignore repeated strings
                    
                    if (!ExtractEdidText (edid, 0xFC, monitor->name.native)) {
                        ExtractEdidText (edid, 0xFE, monitor->name.native);
                    };
                    
                    ExtractEdidText (edid, 0xFF, monitor->name.serial);
                    
                    if (monitor->size.cx || monitor->size.cy) {
                        monitor->current.dpi = std::sqrt (monitor->current.resolution.cx * monitor->current.resolution.cx
                                                        + monitor->current.resolution.cy * monitor->current.resolution.cy)
                                             / std::sqrt (monitor->size.cx * monitor->size.cx
                                                        + monitor->size.cy * monitor->size.cy)
                                             * 2.54f * 10.0f
                                             + 0.5f;
                    } else {
                        monitor->current.dpi = 96;
                    };
                };
                
                RegCloseKey (hKey);
            };
            
            // compute resolution details
            //  - guessing native resolution by maximal width
            
            if (   monitor->native.resolution.cx == 0 
                || monitor->native.resolution.cy == 0) {
                
                DEVMODE dm;
                dm.dmSize = sizeof dm;
                dm.dmDriverExtra = 0;
                
                unsigned int i = 0u;
                while (EnumDisplaySettings (dd.DeviceName, i++, &dm)) {
                    if (monitor->native.resolution.cx < (int) dm.dmPelsWidth) {
                        monitor->native.resolution.cx = dm.dmPelsWidth;
                        monitor->native.resolution.cy = dm.dmPelsHeight;
                    };
                };
            };
            
            if (monitor->size.cx || monitor->size.cy) {
                monitor->native.dpi = std::sqrt (monitor->native.resolution.cx * monitor->native.resolution.cx
                                               + monitor->native.resolution.cy * monitor->native.resolution.cy)
                                    / std::sqrt (monitor->size.cx * monitor->size.cx
                                               + monitor->size.cy * monitor->size.cy)
                                    * 2.54f * 10.0f
                                    + 0.5f;
            } else {
                monitor->native.dpi = 96;
            };
        };

        // effective dpi
        //  - GetDpiForMonitor
        
        HRESULT (WINAPI * pfnGetDpiForMonitor) (HMONITOR, DWORD, UINT *, UINT *);
        if (Windows::Symbol (L"SHCORE.DLL", pfnGetDpiForMonitor, "GetDpiForMonitor")) {
            UINT y;
            if (pfnGetDpiForMonitor (handle, 0, &monitor->current.scaling, &y) != S_OK) {
                Windows::GetDesktopDPI (&monitor->current.scaling, NULL);
            };
        } else {
            Windows::GetDesktopDPI (&monitor->current.scaling, NULL);
        };

        return true;
    };

    bool ExtractMonitorRegPath (const wchar_t * interfaceID,
                                wchar_t * path, unsigned int max) {
        
        wchar_t monitor_id [64]; // usually 7 characters
        monitor_id [0] = 0;

        wchar_t monitor_sub [192]; // usually about 20 characters
        monitor_sub [0] = 0;
        
        if (const wchar_t * p = std::wcschr (interfaceID, L'#')) {
            std::size_t n = sizeof monitor_id / sizeof monitor_id [0] - 1u;
            if (const wchar_t * e = std::wcschr (p + 1, L'#')) {
                std::size_t nn = e - p - 1;
                if (n > nn) {
                    n = nn;
                    std::wcsncpy (monitor_id, p + 1, n);
                    monitor_id [n] = L'\0';
                    
                    // sub
                    n = sizeof monitor_sub / sizeof monitor_sub [0] - 1u;
                    if (const wchar_t * k = std::wcschr (e + 1, L'#')) {
                        nn = k - e - 1;
                        if (n > nn) {
                            n = nn;
                            std::wcsncpy (monitor_sub, e + 1, n);
                            monitor_sub [n] = L'\0';

                            _snwprintf (path, max,
                                        L"SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\%s\\%s\\Device Parameters",
                                        monitor_id, monitor_sub);
                            return true;
                        };
                    };
                };
            };
        };
        return false;
    };

    bool ExtractEdidText (BYTE * edid, BYTE code, wchar_t * out) {
        static const char offsets [] = { /*54,*/ 72, 90, 108 };
        
        bool anything = false;
        bool space = false;
        
        for (int offset : offsets)
            if (   edid [offset + 0] == 0x00
                && edid [offset + 1] == 0x00
                && edid [offset + 2] == 0x00
                && edid [offset + 3] == code
                && edid [offset + 4] == 0x00) {
                
                for (int i = 0; i != 13; ++i) {
                    unsigned char c = edid [offset + 5 + i];
                    if (c <= ' ') {
                        if (space == false) {
                            space = true;
                            *out++ = L' ';
                        };
                    } else {
                        space = false;
                        anything = true;
                        
                        if (c == '_')
                            c = ' ';
                        
                        *out++ = c;
                    };
                };
            };
        
        if (anything) {
            if (out [-1] == L' ')
                out [-1] = L'\0';
            else
                out [0] = L'\0';
        } else
            out [0] = L'\0';
        
        return anything;
    };
};
    
