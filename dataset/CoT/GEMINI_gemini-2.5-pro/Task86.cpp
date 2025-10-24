#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <urlmon.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Link with the URL Moniker library for URLDownloadToFileW
#pragma comment(lib, "urlmon.lib")

/**
 * @brief Deletes a file specified by path. Used with unique_ptr for RAII cleanup.
 */
struct FileDeleter {
    void operator()(WCHAR* path) const {
        if (path) {
            if (DeleteFileW(path)) {
                std::wcout << L"Cleaned up temporary file: " << path << std::endl;
            } else {
                std::wcerr << L"Failed to delete temporary file: " << path << L" Error: " << GetLastError() << std::endl;
            }
            delete[] path;
        }
    }
};

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
bool downloadAndExecute(const std::wstring& url) {
    WCHAR tempPath[MAX_PATH];
    WCHAR tempFilePath[MAX_PATH];
    std::unique_ptr<WCHAR[], FileDeleter> finalExePath(nullptr);
    
    // 1. Create a secure temporary file path
    DWORD pathLen = GetTempPathW(MAX_PATH, tempPath);
    if (pathLen == 0 || pathLen > MAX_PATH) {
        std::cerr << "Error getting temp path: " << GetLastError() << std::endl;
        return false;
    }

    if (GetTempFileNameW(tempPath, L"DEX", 0, tempFilePath) == 0) {
        std::cerr << "Error creating temp file name: " << GetLastError() << std::endl;
        return false;
    }

    // Rename the file to have a .exe extension.
    std::wstring newPathStr = std::wstring(tempFilePath) + L".exe";
    WCHAR* newPathCStr = new WCHAR[newPathStr.length() + 1];
    wcscpy_s(newPathCStr, newPathStr.length() + 1, newPathStr.c_str());
    finalExePath.reset(newPathCStr);

    if (!MoveFileW(tempFilePath, finalExePath.get())) {
        std::cerr << "Error renaming temp file: " << GetLastError() << std::endl;
        DeleteFileW(tempFilePath);
        return false;
    }
    
    // 2. Download the file
    std::wcout << L"Downloading from " << url << L" to " << finalExePath.get() << std::endl;
    HRESULT hr = URLDownloadToFileW(NULL, url.c_str(), finalExePath.get(), 0, NULL);
    if (FAILED(hr)) {
        std::cerr << "Error downloading file. HRESULT: 0x" << std::hex << hr << std::endl;
        return false;
    }
    
    // 3. Execute the downloaded file
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    std::wcout << L"Executing " << finalExePath.get() << std::endl;
    // Using CreateProcessW is secure as it does not involve the shell.
    if (!CreateProcessW(
        finalExePath.get(), // Application name
        NULL,               // Command line
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        FALSE,              // Set handle inheritance to FALSE
        0,                  // No creation flags
        NULL,               // Use parent's environment block
        NULL,               // Use parent's starting directory
        &si,                // Pointer to STARTUPINFO structure
        &pi)                // Pointer to PROCESS_INFORMATION structure
    ) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    std::wcout << L"Process started successfully with PID: " << pi.dwProcessId << std::endl;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

int main() {
    std::wcout << L"--- Running Test Cases ---" << std::endl;

    // Test Case 1: A well-known, safe 64-bit executable (PuTTY)
    std::wcout << L"\n--- Test Case 1: Valid 64-bit EXE (PuTTY) ---" << std::endl;
    downloadAndExecute(L"https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe");

    // Test Case 2: A well-known, safe portable executable (Process Explorer)
    std::wcout << L"\n--- Test Case 2: Valid portable EXE (Process Explorer) ---" << std::endl;
    downloadAndExecute(L"https://live.sysinternals.com/procexp.exe");

    // Test Case 3: An invalid URL that will fail to download
    std::wcout << L"\n--- Test Case 3: Invalid URL ---" << std::endl;
    downloadAndExecute(L"https://this-is-a-fake-url-12345.com/nonexistent.exe");
    
    // Test Case 4: A valid URL to a non-executable file
    std::wcout << L"\n--- Test Case 4: Valid URL, but not an EXE ---" << std::endl;
    downloadAndExecute(L"https://www.google.com/images/branding/googlelogo/1x/googlelogo_color_272x92dp.png");

    // Test Case 5: A well-known, safe 32-bit executable (PuTTY)
    std::wcout << L"\n--- Test Case 5: Valid 32-bit EXE (PuTTY) ---" << std::endl;
    downloadAndExecute(L"https://the.earth.li/~sgtatham/putty/latest/w32/putty.exe");

    return 0;
}