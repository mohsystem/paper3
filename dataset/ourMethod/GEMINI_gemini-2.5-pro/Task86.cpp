// This code requires the libcurl library.
// On Windows with MSVC, link: libcurl.lib;Ws2_32.lib;Wldap32.lib;Crypt32.lib;Normaliz.lib;
// On Linux/macOS, link with: -lcurl
// This implementation is Windows-specific for file execution (.exe).
// Compile with C++17 or later for std::filesystem.
#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <curl/curl.h>

// Callback function for libcurl to write received data into a file
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    // Rule #5: fwrite handles writing to the buffer (file stream) safely.
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/**
 * Downloads an executable file from a given URL and executes it.
 * WARNING: This function is inherently dangerous as it downloads and executes
 * code from the internet. Only use with URLs from trusted sources.
 *
 * @param url The URL of the .exe file to download.
 * @return 0 on success, non-zero on failure.
 */
int downloadAndExecute(const std::string& url) {
    // Rule #3: Ensure all input is validated
    if (url.empty()) {
        std::cerr << "Error: URL is empty." << std::endl;
        return 1;
    }
    if (url.rfind("https://", 0) != 0) {
        std::cerr << "Error: URL must use HTTPS." << std::endl;
        return 2;
    }
    if (url.length() < 4) {
        std::cerr << "Error: URL must point to an .exe file." << std::endl;
        return 3;
    }
    std::string extension = url.substr(url.length() - 4);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c){ return std::tolower(c); });
    if (extension != ".exe") {
        std::cerr << "Error: URL must point to an .exe file." << std::endl;
        return 3;
    }

    std::filesystem::path tempFilePath;
    FILE* fp = nullptr;
    CURL* curl = curl_easy_init();
    
    if (!curl) {
        std::cerr << "Error: Failed to initialize libcurl." << std::endl;
        return 4;
    }
    // Using a unique_ptr for automatic cleanup of curl handle (RAII)
    auto curl_deleter = [](CURL* c){ curl_easy_cleanup(c); };
    std::unique_ptr<CURL, decltype(curl_deleter)> curl_ptr(curl, curl_deleter);

    try {
        // Rule #7: Use a safe, temporary directory.
        tempFilePath = std::filesystem::temp_directory_path() / "downloaded_program.exe";
        
        #ifdef _WIN32
            if (fopen_s(&fp, tempFilePath.string().c_str(), "wb") != 0) fp = nullptr;
        #else
            fp = fopen(tempFilePath.string().c_str(), "wb");
        #endif

        if (!fp) throw std::runtime_error("Failed to open temporary file for writing.");
        
        std::cout << "Downloading from " << url << " to " << tempFilePath << std::endl;

        // Rules #1 & #2: libcurl by default performs SSL/TLS validation.
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
        
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) throw std::runtime_error(std::string("Failed to download file: ") + curl_easy_strerror(res));
        
        fclose(fp);
        fp = nullptr;

        #ifndef _WIN32
            chmod(tempFilePath.c_str(), S_IRWXU); // rwx for owner
        #endif

        std::cout << "Download complete. Executing " << tempFilePath << std::endl;
        
        // Rule #4: Execute the downloaded file
        #ifdef _WIN32
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            std::wstring widePath = tempFilePath.wstring();
            
            if (!CreateProcessW(NULL, &widePath[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                throw std::runtime_error("Failed to execute process. Error code: " + std::to_string(GetLastError()));
            }
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            std::cout << "Program finished with exit code: " << exitCode << std::endl;
        #else
            std::cerr << "Execution of .exe is not supported on this platform." << std::endl;
        #endif

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        if (fp) fclose(fp);
        if (!tempFilePath.empty()) std::filesystem::remove(tempFilePath);
        return 5;
    }

    // Rule #8: Cleanup
    if (!tempFilePath.empty()) {
        std::filesystem::remove(tempFilePath);
        std::cout << "Cleaned up temporary file: " << tempFilePath << std::endl;
    }
    return 0; // Success
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::cout << "--- Running Test Cases ---" << std::endl;

    std::cout << "\n[Test Case 1: Valid URL]" << std::endl;
    int r1 = downloadAndExecute("https://www.chiark.greenend.org.uk/~sgtatham/putty/latest/w64/putty.exe");
    std::cout << "Test Case 1 Result: " << (r1 == 0 ? "Success (as expected)" : "Failure") << std::endl;

    std::cout << "\n[Test Case 2: HTTP URL]" << std::endl;
    int r2 = downloadAndExecute("http://www.chiark.greenend.org.uk/~sgtatham/putty/latest/w64/putty.exe");
    std::cout << "Test Case 2 Result: " << (r2 != 0 ? "Failure (as expected)" : "Success (unexpected)") << std::endl;

    std::cout << "\n[Test Case 3: Non-EXE URL]" << std::endl;
    int r3 = downloadAndExecute("https://www.google.com/index.html");
    std::cout << "Test Case 3 Result: " << (r3 != 0 ? "Failure (as expected)" : "Success (unexpected)") << std::endl;

    std::cout << "\n[Test Case 4: Not Found URL]" << std::endl;
    int r4 = downloadAndExecute("https://example.com/nonexistent/program.exe");
    std::cout << "Test Case 4 Result: " << (r4 != 0 ? "Failure (as expected)" : "Success (unexpected)") << std::endl;

    std::cout << "\n[Test Case 5: Empty URL]" << std::endl;
    int r5 = downloadAndExecute("");
    std::cout << "Test Case 5 Result: " << (r5 != 0 ? "Failure (as expected)" : "Success (unexpected)") << std::endl;

    std::cout << "\n--- Test Cases Finished ---" << std::endl;

    curl_global_cleanup();
    return 0;
}