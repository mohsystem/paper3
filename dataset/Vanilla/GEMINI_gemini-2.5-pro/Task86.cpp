#include <iostream>
#include <string>
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
void downloadAndExecute(const std::string& url, const std::string& fileName) {
    // Step 1: Download the file using URLDownloadToFileA
    std::cout << "Downloading " << url << " to " << fileName << "..." << std::endl;
    HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), fileName.c_str(), 0, NULL);

    if (SUCCEEDED(hr)) {
        std::cout << "Download complete." << std::endl;

        // Step 2: Execute the downloaded file using ShellExecuteExA
        std::cout << "Executing " << fileName << "..." << std::endl;
        
        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.fMask = SEE_MASK_NOCLOSEPROCESS; // Get a handle to the new process
        sei.lpVerb = "open";
        sei.lpFile = fileName.c_str();
        sei.nShow = SW_SHOWNORMAL;
        
        if (ShellExecuteExA(&sei)) {
            if (sei.hProcess != NULL) {
                // Wait for the process to terminate
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD exitCode;
                GetExitCodeProcess(sei.hProcess, &exitCode);
                std::cout << "'" << fileName << "' finished with exit code: " << exitCode << std::endl;
                CloseHandle(sei.hProcess);
            }
        } else {
            DWORD error = GetLastError();
            std::cerr << "Failed to execute file. ShellExecuteExA failed with error: " << error << std::endl;
        }

    } else {
        std::cerr << "Failed to download file. HRESULT: 0x" << std::hex << hr << std::endl;
    }
}

int main() {
    // Using a URL for a safe, well-known executable (PuTTY)
    std::string fileUrl = "https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe";
    
    std::cout << "--- Starting 5 Test Cases ---" << std::endl;

    std::cout << "\n--- Test Case 1 ---" << std::endl;
    downloadAndExecute(fileUrl, "putty_cpp_1.exe");

    std::cout << "\n--- Test Case 2 ---" << std::endl;
    downloadAndExecute(fileUrl, "putty_cpp_2.exe");
    
    std::cout << "\n--- Test Case 3 ---" << std::endl;
    downloadAndExecute(fileUrl, "putty_cpp_3.exe");
    
    std::cout << "\n--- Test Case 4 ---" << std::endl;
    downloadAndExecute(fileUrl, "putty_cpp_4.exe");

    std::cout << "\n--- Test Case 5 ---" << std::endl;
    downloadAndExecute(fileUrl, "putty_cpp_5.exe");

    std::cout << "\n--- All tests completed ---" << std::endl;

    return 0;
}