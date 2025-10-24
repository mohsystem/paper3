// Note: This C++ implementation targets Windows.
// It uses URLDownloadToFile from Urlmon and CreateProcess to execute the downloaded EXE.

#include <windows.h>
#include <urlmon.h>
#include <cstdio>
#include <string>

#pragma comment(lib, "Urlmon.lib")

int download_and_execute(const wchar_t* url, const wchar_t* destDirectory) {
    // Returns process exit code, or negative value on error.
    wchar_t dirBuffer[MAX_PATH] = {0};
    if (destDirectory == nullptr || wcslen(destDirectory) == 0) {
        DWORD len = GetTempPathW(MAX_PATH, dirBuffer);
        if (len == 0 || len > MAX_PATH) return -1;
    } else {
        wcsncpy_s(dirBuffer, destDirectory, _TRUNCATE);
        // Ensure directory exists
        CreateDirectoryW(dirBuffer, nullptr);
    }

    std::wstring targetPath = std::wstring(dirBuffer);
    if (!targetPath.empty() && targetPath.back() != L'\\' && targetPath.back() != L'/') {
        targetPath.push_back(L'\\');
    }
    targetPath += L"downloaded_program.exe";

    HRESULT hr = URLDownloadToFileW(nullptr, url, targetPath.c_str(), 0, nullptr);
    if (FAILED(hr)) {
        return -1;
    }

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    // CreateProcess with application name. Command line can be nullptr.
    if (!CreateProcessW(
            targetPath.c_str(),    // lpApplicationName
            nullptr,               // lpCommandLine
            nullptr,               // lpProcessAttributes
            nullptr,               // lpThreadAttributes
            FALSE,                 // bInheritHandles
            0,                     // dwCreationFlags
            nullptr,               // lpEnvironment
            nullptr,               // lpCurrentDirectory
            &si,                   // lpStartupInfo
            &pi                    // lpProcessInformation
        )) {
        return -2;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return static_cast<int>(exitCode);
}

int main() {
    // Five test cases (placeholders). Replace with actual reachable EXE URLs in real use.
    const wchar_t* tests[5] = {
        L"https://example.com/program1.exe",
        L"https://example.com/program2.exe",
        L"https://example.org/tools/setup.exe",
        L"https://example.net/app/latest.exe",
        L"https://example.com/download/myapp.exe"
    };

    for (int i = 0; i < 5; ++i) {
        int rc = download_and_execute(tests[i], nullptr);
        wprintf(L"URL: %ls -> Exit Code: %d\n", tests[i], rc);
    }
    return 0;
}