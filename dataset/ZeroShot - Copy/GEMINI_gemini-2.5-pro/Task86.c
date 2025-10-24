#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This code is Windows-specific due to the use of WinINet and CreateProcess.
#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
// Provide a stub for non-Windows platforms
void downloadAndExecute(const char* urlString) {
    fprintf(stderr, "This function is only implemented for Windows.\n");
}
#endif

#ifdef _WIN32
/**
 * @brief Downloads an EXE file from a URL and executes it.
 * WARNING: This is an inherently dangerous operation. Executing code from
 * untrusted sources can compromise your system. This function should only be
 * used with URLs from trusted and verified sources. A production system
 * should include additional security measures like digital signature verification
 * and sandboxing.
 *
 * @param urlString The URL of the EXE file to download.
 */
void downloadAndExecute(const char* urlString) {
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    FILE* tempFile = NULL;
    char tempPathBuffer[MAX_PATH];
    char tempFileName[MAX_PATH] = {0}; // Initialize to zero

    // Use a do-while(0) loop to easily break out on error and go to cleanup.
    do {
        // 1. Get a temporary file path
        DWORD pathLen = GetTempPathA(MAX_PATH, tempPathBuffer);
        if (pathLen == 0 || pathLen > MAX_PATH) {
            fprintf(stderr, "Error: Failed to get temporary path. (WinError: %lu)\n", GetLastError());
            break;
        }
        UINT fileInt = GetTempFileNameA(tempPathBuffer, "dwn", 0, tempFileName);
        if (fileInt == 0) {
            fprintf(stderr, "Error: Failed to create temporary file name. (WinError: %lu)\n", GetLastError());
            break;
        }

        // 2. Download the file using WinINet
        printf("Downloading from: %s\n", urlString);
        hInternet = InternetOpenA("Downloader/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            fprintf(stderr, "Error: InternetOpenA failed. (WinError: %lu)\n", GetLastError());
            break;
        }

        hConnect = InternetOpenUrlA(hInternet, urlString, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hConnect) {
            fprintf(stderr, "Error: InternetOpenUrlA failed. Check URL and connection. (WinError: %lu)\n", GetLastError());
            break;
        }

        // Use secure fopen_s on modern MSVC, fallback to fopen
        #if defined(_MSC_VER) && _MSC_VER >= 1400
            if (fopen_s(&tempFile, tempFileName, "wb") != 0) {
                tempFile = NULL;
            }
        #else
            tempFile = fopen(tempFileName, "wb");
        #endif
        
        if (!tempFile) {
            fprintf(stderr, "Error: Could not open temporary file for writing.\n");
            break;
        }

        char buffer[4096];
        DWORD bytesRead;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            fwrite(buffer, 1, bytesRead, tempFile);
        }
        fclose(tempFile);
        tempFile = NULL; // Set to NULL after closing

        printf("Downloaded to: %s\n", tempFileName);

        // 3. Execute the file
        printf("Executing: %s\n", tempFileName);
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessA(tempFileName, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            fprintf(stderr, "Error: CreateProcess failed. (WinError: %lu)\n", GetLastError());
            break;
        }

        // Close process and thread handles immediately. We don't wait.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    } while (0);

    // 4. Cleanup
    if (hConnect) InternetCloseHandle(hConnect);
    if (hInternet) InternetCloseHandle(hInternet);
    if (tempFile) fclose(tempFile);

    if (tempFileName[0] != '\0') {
        // On Windows, deletion might fail if the launched process is still running.
        if (remove(tempFileName) == 0) {
            printf("Cleaned up temporary file: %s\n", tempFileName);
        } else {
            fprintf(stderr, "Could not delete temporary file: %s. It may be in use.\n", tempFileName);
        }
    }
}
#endif // _WIN32

// Per instructions, use Task86 in the main method name for C
void Task86_main() {
    #ifdef _WIN32
    printf("--- Test Case 1: Valid Harmless EXE (PuTTY) ---\n");
    // WARNING: This URL points to a legitimate third-party application.
    // Running it is at your own risk. This test is for demonstration only.
    downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe");

    printf("\n--- Test Case 2: URL with 404 Not Found ---\n");
    downloadAndExecute("https://example.com/nonexistent/program.exe");

    printf("\n--- Test Case 3: URL to a non-EXE file (text file) ---\n");
    // This will download a text file and attempt to execute it.
    // The operating system should handle this gracefully (e.g., show an error).
    downloadAndExecute("https://raw.githubusercontent.com/octocat/Spoon-Knife/main/README.md");

    printf("\n--- Test Case 4: Invalid URL format ---\n");
    downloadAndExecute("invalid-url"); // WinINet will fail on this
    
    printf("\n--- Test Case 5: Another valid EXE (PSCP) ---\n");
    // PSCP is a command-line tool, it will run and exit quickly.
    downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/pscp.exe");
    #else
    printf("--- All test cases skipped on non-Windows platform ---\n");
    #endif
}

int main() {
    Task86_main();
    return 0;
}