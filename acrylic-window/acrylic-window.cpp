#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <iostream>

#pragma comment(lib, "dwmapi.lib")

// For SetWindowCompositionAttribute (undocumented API)
enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4
};

struct ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
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

void PrintUsage()
{
    std::wcout
        << L"Usage:\n"
        << L"  acrylic_window.exe -type \"blur|acrylic\"\n"
        << L"                     -corner \"none|round|roundsmall\"\n"
        << L"                     -title \"Window Title\"\n"
        << L"                     [-opacity 0-255]\n\n"
        << L"Examples:\n"
        << L"  acrylic_window.exe -type blur -corner none -title \"Notepad\" -opacity 128\n"
        << L"  acrylic_window.exe -type acrylic -corner roundsmall -title \"MyApp\"\n";
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 7) {
        PrintUsage();
        return 1;
    }

    std::wstring type, corner, title;
    int opacity = 204; // default ~80%
    for (int i = 1; i < argc; i += 2) {
        if (_wcsicmp(argv[i], L"-type") == 0 && i + 1 < argc) {
            type = argv[i + 1];
        }
        else if (_wcsicmp(argv[i], L"-corner") == 0 && i + 1 < argc) {
            corner = argv[i + 1];
        }
        else if (_wcsicmp(argv[i], L"-title") == 0 && i + 1 < argc) {
            title = argv[i + 1];
        }
        else if (_wcsicmp(argv[i], L"-opacity") == 0 && i + 1 < argc) {
            opacity = _wtoi(argv[i + 1]);
            if (opacity < 0 || opacity > 255) {
                std::wcerr << L"Error: -opacity must be between 0 and 255\n";
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

    HWND hwnd = FindWindowW(nullptr, title.c_str());
    if (!hwnd) {
        std::wcerr << L"Error: Could not find window titled \"" << title << L"\"\n";
        return 1;
    }

    // Load SetWindowCompositionAttribute
    HMODULE hUser = GetModuleHandleW(L"user32.dll");
    if (!hUser) {
        std::wcerr << L"Error: Failed to get user32.dll handle\n";
        return 1;
    }
    auto SetWindowCompositionAttribute = reinterpret_cast<pSetWindowCompositionAttribute>(
        GetProcAddress(hUser, "SetWindowCompositionAttribute"));
    if (!SetWindowCompositionAttribute) {
        std::wcerr << L"Error: SetWindowCompositionAttribute not available\n";
        return 1;
    }

    // Build accent policy
    ACCENT_POLICY policy = {};
    policy.AccentFlags = 0;
    policy.AnimationId = 0;
    policy.AccentState = (type == L"blur")
        ? ACCENT_ENABLE_BLURBEHIND
        : ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.GradientColor = (static_cast<DWORD>(opacity) << 24) | 0x00FFFFFF;

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attrib = 19; // WCA_ACCENT_POLICY
    data.pvData = &policy;
    data.cbData = sizeof(policy);

    if (!SetWindowCompositionAttribute(hwnd, &data)) {
        std::wcerr << L"Error: Failed to apply accent policy\n";
        return 1;
    }

    // Apply corner preference
    int pref = 0;
    if (corner == L"none")       pref = 1; // DWMWCP_DONOTROUND
    else if (corner == L"round") pref = 2; // DWMWCP_ROUND
    else                         pref = 3; // DWMWCP_ROUNDSMALL

    HRESULT hr = DwmSetWindowAttribute(
        hwnd,
        DWMWA_WINDOW_CORNER_PREFERENCE,
        &pref,
        sizeof(pref)
    );
    if (corner != L"none" && !SUCCEEDED(hr)) {
        std::wcerr << L"Warning: Corner style not supported on this OS\n";
    }

    std::wcout << L"Applied " << type
        << L" (opacity=" << opacity << L")"
        << L", corner=" << corner
        << L" to \"" << title << L"\"\n";

    return 0;
}
