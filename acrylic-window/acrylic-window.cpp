#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <versionhelpers.h>
#include <winreg.h>

#pragma comment(lib, "dwmapi.lib")

enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD        AccentFlags;
    DWORD        GradientColor; 
    DWORD        AnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    DWORD Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWA_BORDER_COLOR       
#define DWMWA_BORDER_COLOR 34
#endif

void PrintUsage()
{
    std::wcout
        << L"Usage:\n"
        << L"  acrylic_window.exe -type \"blur|acrylic\"\n"
        << L"                     -corner \"none|round|roundsmall\"\n"
        << L"                     -title \"Window Title\"\n"
        << L"                     [-opacity 0-255]\n"
        << L"                     [-tintColor RRGGBB]\n"
        << L"                     [-borderVisible true|false]\n"
        << L"                     [-borderColor RRGGBB]\n\n"
        << L"Examples:\n"
        << L"  acrylic_window.exe -type blur -corner none -title Notepad \\\n"
        L"                     -opacity 128 -tintColor FF0000 \\\n"
        L"                     -borderVisible false\n"
        << L"  acrylic_window.exe -type acrylic -corner roundsmall -title MyApp \\\n"
        L"                     -borderColor 00FF88\n";
}

bool ParseHexColor(const std::wstring& s, DWORD& outRGB)
{
    std::wistringstream iss(s);
    iss >> std::hex >> outRGB;
    return !iss.fail();
}

bool ParseBool(const std::wstring& s, bool& out)
{
    if (s == L"true" || s == L"1") { out = true;  return true; }
    if (s == L"false" || s == L"0") { out = false; return true; }
    return false;
}

DWORD GetWindowsBuild()
{
    wchar_t buf[64] = {};
    DWORD bufSize = sizeof(buf);
    if (RegGetValueW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        L"CurrentBuildNumber",
        RRF_RT_REG_SZ,
        nullptr,
        buf,
        &bufSize) == ERROR_SUCCESS)
    {
        return std::stoi(buf);
    }
    return 0;
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 7) {
        PrintUsage();
        return 1;
    }

    std::wstring type, corner, title;
    int     opacity = 204;     
    DWORD   tintRGB = 0xFFFFFF;
    bool    borderVis = true;    
    DWORD   borderRGB = 0xFFFFFF; 

    for (int i = 1; i < argc; i += 2) {
        std::wstring opt = argv[i];
        std::wstring val = (i + 1 < argc) ? argv[i + 1] : L"";

        if (_wcsicmp(opt.c_str(), L"-type") == 0) {
            type = val;
        }
        else if (_wcsicmp(opt.c_str(), L"-corner") == 0) {
            corner = val;
        }
        else if (_wcsicmp(opt.c_str(), L"-title") == 0) {
            title = val;
        }
        else if (_wcsicmp(opt.c_str(), L"-opacity") == 0) {
            opacity = _wtoi(val.c_str());
            if (opacity < 0 || opacity > 255) {
                std::wcerr << L"Error: -opacity must be 0–255\n";
                return 1;
            }
        }
        else if (_wcsicmp(opt.c_str(), L"-tintColor") == 0) {
            if (!ParseHexColor(val, tintRGB) || tintRGB > 0xFFFFFF) {
                std::wcerr << L"Error: -tintColor must be hex RRGGBB\n";
                return 1;
            }
        }
        else if (_wcsicmp(opt.c_str(), L"-borderVisible") == 0) {
            if (!ParseBool(val, borderVis)) {
                std::wcerr << L"Error: -borderVisible must be true or false\n";
                return 1;
            }
        }
        else if (_wcsicmp(opt.c_str(), L"-borderColor") == 0) {
            if (!ParseHexColor(val, borderRGB) || borderRGB > 0xFFFFFF) {
                std::wcerr << L"Error: -borderColor must be hex RRGGBB\n";
                return 1;
            }
        }
        else {
            PrintUsage();
            return 1;
        }
    }

    if (type != L"blur" && type != L"acrylic") {
        std::wcerr << L"Error: -type must be \"blur\" or \"acrylic\"\n";
        return 1;
    }
    if (corner != L"none" && corner != L"round" && corner != L"roundsmall") {
        std::wcerr << L"Error: -corner must be \"none\", \"round\" or \"roundsmall\"\n";
        return 1;
    }
    if (title.empty()) {
        std::wcerr << L"Error: -title is required\n";
        return 1;
    }

    HWND hwnd = FindWindowW(nullptr, title.c_str());
    if (!hwnd) {
        std::wcerr << L"Error: Could not find window titled \""
            << title << L"\"\n";
        return 1;
    }

    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (!hUser) {
        std::wcerr << L"Error: GetModuleHandleW failed\n";
        return 1;
    }
    auto SetWCA = reinterpret_cast<pSetWindowCompositionAttribute>(
        GetProcAddress(hUser, "SetWindowCompositionAttribute"));
    if (!SetWCA) {
        std::wcerr << L"Error: SetWindowCompositionAttribute unavailable\n";
        return 1;
    }

    ACCENT_POLICY policy = {};
    policy.AccentState = (type == L"blur")
        ? ACCENT_ENABLE_BLURBEHIND
        : ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.AccentFlags = 0;
    policy.AnimationId = 0;
    policy.GradientColor = (static_cast<DWORD>(opacity) << 24) | tintRGB;

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = 19; 
    data.pvData = &policy;
    data.cbData = sizeof(policy);

    if (!SetWCA(hwnd, &data)) {
        std::wcerr << L"Error: Failed to apply accent policy\n";
        return 1;
    }

    int pref = (corner == L"none") ? 1  
        : (corner == L"round") ? 2 
        : 3;

    HRESULT hr = DwmSetWindowAttribute(
        hwnd,
        DWMWA_WINDOW_CORNER_PREFERENCE,
        &pref,
        sizeof(pref)
    );
    if (corner != L"none" && !SUCCEEDED(hr)) {
        std::wcerr << L"Warning: Corner style not supported on this OS\n";
    }

    DWORD build = GetWindowsBuild();
    if (IsWindowsVersionOrGreater(10, 0, 22000) || build >= 22000) {
        DWORD cr = ((borderVis ? 0xFF : 0x00) << 24)
            | ((borderRGB & 0x0000FF) << 16)
            | (borderRGB & 0x00FF00)
            | ((borderRGB & 0xFF0000) >> 16);

        hr = DwmSetWindowAttribute(
            hwnd,
            DWMWA_BORDER_COLOR,
            &cr,
            sizeof(cr)
        );
        if (!SUCCEEDED(hr)) {
            std::wcerr << L"Warning: Failed to set border color on Win11\n";
        }
    }

    std::wcout
        << L"Applied `" << type
        << L"` (opacity=" << opacity
        << L", tint=#" << std::hex << std::setw(6)
        << std::setfill(L'0') << tintRGB << std::dec
        << L"), corner=" << corner
        << L", borderVisible=" << (borderVis ? L"true" : L"false")
        << L", borderColor=#" << std::hex << std::setw(6)
        << std::setfill(L'0') << borderRGB << std::dec
        << L" to \"" << title << L"\"\n";

    return 0;
}
