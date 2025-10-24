// Note: This C implementation targets Windows.
// It uses URLDownloadToFile from Urlmon and CreateProcess to execute the downloaded EXE.

#include <windows.h>
#include <urlmon.h>
#include <stdio.h>

#pragma comment(lib, "Urlmon.lib")

int download_and_execute(const char* url, const char* destDirectory) {
    // Returns process exit code, or negative value on error.
    char dirBuffer[MAX_PATH] = {0};
    if (destDirectory == NULL || destDirectory[0] == '\0') {
        DWORD len = GetTempPathA(MAX_PATH, dirBuffer);
        if (len == 0 || len > MAX_PATH) return -1;
    } else {
        strncpy_s(dirBuffer, sizeof(dirBuffer), destDirectory, _TRUNCATE);
        // Ensure directory exists (best-effort)
        CreateDirectoryA(dirBuffer, NULL);
    }

    char targetPath[MAX_PATH] = {0};
    strncpy_s(targetPath, sizeof(targetPath), dirBuffer, _TRUNCATE);
    size_t n = strlen(targetPath);
    if (n > 0 && targetPath[n - 1] != '\\' && targetPath[n - 1] != '/') {
        strncat_s(targetPath, sizeof(targetPath), "\\", _TRUNCATE);
    }
    strncat_s(targetPath, sizeof(targetPath), "downloaded_program.exe", _TRUNCATE);

    HRESULT hr = URLDownloadToFileA(NULL, url, targetPath, 0, NULL);
    if (FAILED(hr)) {
        return -1;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    if (!CreateProcessA(
            targetPath,    // lpApplicationName (app name or command line; here as app name)
            NULL,          // lpCommandLine
            NULL,          // lpProcessAttributes
            NULL,          // lpThreadAttributes
            FALSE,         // bInheritHandles
            0,             // dwCreationFlags
            NULL,          // lpEnvironment
            NULL,          // lpCurrentDirectory
            &si,           // lpStartupInfo
            &pi            // lpProcessInformation
        )) {
        return -2;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return (int)exitCode;
}

int main() {
    // Five test cases (placeholders). Replace with actual reachable EXE URLs in real use.
    const char* tests[5] = {
        "https://example.com/program1.exe",
        "https://example.com/program2.exe",
        "https://example.org/tools/setup.exe",
        "https://example.net/app/latest.exe",
        "https://example.com/download/myapp.exe"
    };

    for (int i = 0; i < 5; ++i) {
        int rc = download_and_execute(tests[i], NULL);
        printf("URL: %s -> Exit Code: %d\n", tests[i], rc);
    }
    return 0;
}