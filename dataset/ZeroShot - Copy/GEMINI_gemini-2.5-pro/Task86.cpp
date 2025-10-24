#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

// This code is Windows-specific due to the use of WinINet and CreateProcess.
#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
#else
// Provide a stub for non-Windows platforms
void downloadAndExecute(const std::string& urlString) {
    std::cerr << "This function is only implemented for Windows." << std::endl;
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
void downloadAndExecute(const std::string& urlString) {
    char tempPathBuffer[MAX_PATH];
    char tempFileName[MAX_PATH];
    tempFileName[0] = '\0'; // Ensure it's an empty string initially
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;

    // RAII wrapper for handles to ensure they are closed.
    auto internetHandleCloser = [](HINTERNET h) { if (h) InternetCloseHandle(h); };
    std::unique_ptr<void, decltype(internetHandleCloser)> hInternetManager(nullptr, internetHandleCloser);
    std::unique_ptr<void, decltype(internetHandleCloser)> hConnectManager(nullptr, internetHandleCloser);

    try {
        // 1. Get a temporary file path
        DWORD pathLen = GetTempPathA(MAX_PATH, tempPathBuffer);
        if (pathLen == 0 || pathLen > MAX_PATH) {
            throw std::runtime_error("Failed to get temporary path.");
        }
        UINT fileInt = GetTempFileNameA(tempPathBuffer, "dwn", 0, tempFileName);
        if (fileInt == 0) {
            throw std::runtime_error("Failed to create temporary file name.");
        }
        
        // 2. Download the file using WinINet
        std::cout << "Downloading from: " << urlString << std::endl;
        hInternet = InternetOpenA("Downloader/1.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) {
            throw std::runtime_error("InternetOpenA failed.");
        }
        hInternetManager.reset(hInternet);

        hConnect = InternetOpenUrlA(hInternet, urlString.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hConnect) {
            throw std::runtime_error("InternetOpenUrlA failed. Check URL and connection.");
        }
        hConnectManager.reset(hConnect);
        
        std::ofstream outFile(tempFileName, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Could not open temporary file for writing.");
        }

        char buffer[4096];
        DWORD bytesRead;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            outFile.write(buffer, bytesRead);
        }
        outFile.close();

        std::cout << "Downloaded to: " << tempFileName << std::endl;

        // 3. Execute the file
        std::cout << "Executing: " << tempFileName << std::endl;
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // CreateProcess is safer than system() as it avoids command injection.
        if (!CreateProcessA(tempFileName, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            throw std::runtime_error("CreateProcess failed.");
        }

        // Close process and thread handles. We don't wait for the process to exit.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << " (Windows Error Code: " << GetLastError() << ")" << std::endl;
    }

    // 4. Cleanup
    if (tempFileName[0] != '\0') {
        // On Windows, deletion might fail if the launched process is still running.
        if (remove(tempFileName) == 0) {
            std::cout << "Cleaned up temporary file: " << tempFileName << std::endl;
        } else {
             std::cerr << "Could not delete temporary file: " << tempFileName << ". It may be in use." << std::endl;
        }
    }
}
#endif // _WIN32

class Task86 {
public:
    static void main() {
        #ifdef _WIN32
        std::cout << "--- Test Case 1: Valid Harmless EXE (PuTTY) ---" << std::endl;
        // WARNING: This URL points to a legitimate third-party application.
        // Running it is at your own risk. This test is for demonstration only.
        downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe");

        std::cout << "\n--- Test Case 2: URL with 404 Not Found ---" << std::endl;
        downloadAndExecute("https://example.com/nonexistent/program.exe");

        std::cout << "\n--- Test Case 3: URL to a non-EXE file (text file) ---" << std::endl;
        // This will download a text file and attempt to execute it.
        // The operating system should handle this gracefully (e.g., show an error).
        downloadAndExecute("https://raw.githubusercontent.com/octocat/Spoon-Knife/main/README.md");

        std::cout << "\n--- Test Case 4: Invalid URL format ---" << std::endl;
        downloadAndExecute("invalid-url"); // WinINet will fail on this
        
        std::cout << "\n--- Test Case 5: Another valid EXE (PSCP) ---" << std::endl;
        // PSCP is a command-line tool, it will run and exit quickly.
        downloadAndExecute("https://the.earth.li/~sgtatham/putty/latest/w64/pscp.exe");
        #else
        std::cout << "--- All test cases skipped on non-Windows platform ---" << std::endl;
        #endif
    }
};

int main() {
    Task86::main();
    return 0;
}