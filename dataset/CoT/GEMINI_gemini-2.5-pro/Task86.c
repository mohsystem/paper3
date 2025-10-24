#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <tchar.h>
#include <stdbool.h>

// Link with the URL Moniker library for URLDownloadToFile
#pragma comment(lib, "urlmon.lib")

/**
 * WARNING: This function downloads and executes a file from the internet.
 * This is an extremely dangerous operation and can expose your system to
 * malware and other security risks. Only use this with URLs from sources
 * you completely trust. The downloaded executable will have the same
 * permissions as the user running this program.
 *
 * This function is Windows-specific.
 *
 * @param url The URL of the EXE file to download and execute.
 * @return true if the process was started successfully, false otherwise.
 */
bool downloadAndExecute(const TCHAR* url) {
    TCHAR tempPath[MAX_PATH];
    TCHAR tempFilePath[MAX_PATH];
    TCHAR finalExePath[MAX_PATH + 5]; // +5 for ".exe" and null terminator
    bool success = false;

    // 1. Create a secure temporary file path
    DWORD pathLen = GetTempPath(MAX_PATH, tempPath);
    if (pathLen == 0 || pathLen > MAX_PATH) {
        _ftprintf(stderr, _T("Error getting temp path: %lu\n"), GetLastError());
        return false;
    }

    if (GetTempFileName(tempPath, _T("DEX"), 0, tempFilePath) == 0) {
        _ftprintf(stderr, _T("Error creating temp file name: %lu\n"), GetLastError());
        return false;
    }

    // Rename the file to have a .exe extension
    _stprintf_s(finalExePath, MAX_PATH + 5, _T("%s.exe"), tempFilePath);
    if (!MoveFile(tempFilePath, finalExePath)) {
        _ftprintf(stderr, _T("Error renaming temp file: %lu\n"), GetLastError());
        DeleteFile(tempFilePath); // Cleanup original temp file
        return false;
    }

    // 2. Download the file
    _tprintf(_T("Downloading from %s to %s\n"), url, finalExePath);
    HRESULT hr = URLDownloadToFile(NULL, url, finalExePath, 0, NULL);
    if (FAILED(hr)) {
        _ftprintf(stderr, _T("Error downloading file. HRESULT: 0x%08lX\n"), hr);
        goto cleanup; // Go to cleanup section
    }

    // 3. Execute the downloaded file
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    _tprintf(_T("Executing %s\n"), finalExePath);
    // Using CreateProcess is secure as it does not involve the shell.
    if (!CreateProcess(
            finalExePath, // Application name
            NULL,         // Command line
            NULL,         // Process handle not inheritable
            NULL,         // Thread handle not inheritable
            FALSE,        // Set handle inheritance to FALSE
            0,            // No creation flags
            NULL,         // Use parent's environment block
            NULL,         // Use parent's starting directory
            &si,          // Pointer to STARTUPINFO structure
            &pi)          // Pointer to PROCESS_INFORMATION structure
    ) {
        _ftprintf(stderr, _T("CreateProcess failed. Error: %lu\n"), GetLastError());
        goto cleanup;
    }

    _tprintf(_T("Process started successfully with PID: %lu\n"), pi.dwProcessId);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    success = true;

cleanup:
    // 4. Cleanup: Delete the temporary file
    if (DeleteFile(finalExePath)) {
        _tprintf(_T("Cleaned up temporary file: %s\n"), finalExePath);
    } else {
        // May fail if the process is still running, but we should try
        _ftprintf(stderr, _T("Failed to delete temporary file: %s (Error: %lu)\n"), finalExePath, GetLastError());
    }

    return success;
}

int _tmain(int argc, TCHAR* argv[]) {
    _tprintf(_T("--- Running Test Cases ---\n"));

    // Test Case 1: A well-known, safe 64-bit executable (PuTTY)
    _tprintf(_T("\n--- Test Case 1: Valid 64-bit EXE (PuTTY) ---\n"));
    downloadAndExecute(_T("https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe"));

    // Test Case 2: A well-known, safe portable executable (Process Explorer)
    _tprintf(_T("\n--- Test Case 2: Valid portable EXE (Process Explorer) ---\n"));
    downloadAndExecute(_T("https://live.sysinternals.com/procexp.exe"));

    // Test Case 3: An invalid URL that will fail to download
    _tprintf(_T("\n--- Test Case 3: Invalid URL ---\n"));
    downloadAndExecute(_T("https://this-is-a-fake-url-12345.com/nonexistent.exe"));

    // Test Case 4: A valid URL to a non-executable file
    _tprintf(_T("\n--- Test Case 4: Valid URL, but not an EXE ---\n"));
    downloadAndExecute(_T("https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png"));

    // Test Case 5: A well-known, safe 32-bit executable (PuTTY)
    _tprintf(_T("\n--- Test Case 5: Valid 32-bit EXE (PuTTY) ---\n"));
    downloadAndExecute(_T("https://the.earth.li/~sgtatham/putty/latest/w32/putty.exe"));

    return 0;
}