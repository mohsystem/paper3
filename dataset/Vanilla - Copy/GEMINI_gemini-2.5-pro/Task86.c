#include <stdio.h>
#include <windows.h>
#include <urlmon.h>
#include <shellapi.h>

// WARNING: Downloading and executing files from the internet can be extremely dangerous.
// This code is Windows-specific and should only be used with trusted URLs.

// Link with necessary libraries
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

/**
 * Downloads an executable file from a URL and runs it.
 * This function is specific to the Windows operating system.
 * @param url The URL of the EXE file to download.
 * @param fileName The local filename to save the EXE as.
 */
void downloadAndExecute(const char* url, const char* fileName) {
    // Step 1: Download the file using URLDownloadToFileA
    printf("Downloading %s to %s...\n", url, fileName);
    HRESULT hr = URLDownloadToFileA(NULL, url, fileName, 0, NULL);

    if (SUCCEEDED(hr)) {
        printf("Download complete.\n");

        // Step 2: Execute the downloaded file using ShellExecuteExA
        printf("Executing %s...\n", fileName);

        SHELLEXECUTEINFOA sei;
        ZeroMemory(&sei, sizeof(sei));
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Get a handle to the new process
        sei.lpVerb = "open";
        sei.lpFile = fileName;
        sei.nShow = SW_SHOWNORMAL;

        if (ShellExecuteExA(&sei)) {
            if (sei.hProcess != NULL) {
                // Wait for the process to terminate
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD exitCode;
                GetExitCodeProcess(sei.hProcess, &exitCode);
                printf("'%s' finished with exit code: %lu\n", fileName, exitCode);
                CloseHandle(sei.hProcess);
            }
        } else {
            DWORD error = GetLastError();
            fprintf(stderr, "Failed to execute file. ShellExecuteExA failed with error: %lu\n", error);
        }

    } else {
        fprintf(stderr, "Failed to download file. HRESULT: 0x%lx\n", hr);
    }
}

int main() {
    // Using a URL for a safe, well-known executable (PuTTY)
    const char* fileUrl = "https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe";
    
    printf("--- Starting 5 Test Cases ---\n");

    printf("\n--- Test Case 1 ---\n");
    downloadAndExecute(fileUrl, "putty_c_1.exe");

    printf("\n--- Test Case 2 ---\n");
    downloadAndExecute(fileUrl, "putty_c_2.exe");

    printf("\n--- Test Case 3 ---\n");
    downloadAndExecute(fileUrl, "putty_c_3.exe");

    printf("\n--- Test Case 4 ---\n");
    downloadAndExecute(fileUrl, "putty_c_4.exe");

    printf("\n--- Test Case 5 ---\n");
    downloadAndExecute(fileUrl, "putty_c_5.exe");

    printf("\n--- All tests completed ---\n");

    return 0;
}