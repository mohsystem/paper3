/*
 * Instructions to compile (Windows with MinGW-w64 and vcpkg for libcurl):
 * gcc -Wall -Wextra -pedantic -o task.exe your_source_file.c -I"C:/path/to/vcpkg/installed/x64-windows/include" -L"C:/path/to/vcpkg/installed/x64-windows/lib" -lcurl -lws2_32 -lcrypt32 -lwldap32
 *
 * This program is inherently dangerous as it downloads and executes arbitrary code.
 * It should ONLY be used with URLs from a highly trusted and verified source.
 * The security of this operation depends entirely on the integrity of the source
 * and the security of the transport layer (TLS), which is strictly enforced.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific includes
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
// This functionality is Windows-specific.
#endif

// libcurl include
#include <curl/curl.h>

#define MAX_URL_LENGTH 2048
#define MAX_REDIRS 5
#define REQUEST_TIMEOUT_SECONDS 60L

// Callback function for libcurl to write received data to a file
static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (stream == NULL) {
        return 0;
    }
    size_t written = fwrite(ptr, size, nmemb, stream);
    if (written < nmemb) {
        fprintf(stderr, "Error: fwrite failed to write complete data.\n");
        return 0; // Returning a value different from size*nmemb signals an error to libcurl
    }
    return written;
}

/**
 * @brief Downloads a file from a URL and executes it.
 *
 * This function downloads an executable file from a given HTTPS URL, saves it to a
 * temporary location, executes it, waits for it to finish, and then cleans up
 * the temporary file.
 *
 * @param url The HTTPS URL of the executable file to download.
 * @return 0 on success, -1 on failure.
 */
int downloadAndExecute(const char* url) {
#ifdef _WIN32
    CURL* curl = NULL;
    FILE* fp = NULL;
    CURLcode res = CURLE_OK;
    int result = -1; // Default to failure

    wchar_t tempPath[MAX_PATH] = {0};
    wchar_t tempFileName[MAX_PATH] = {0};
    HANDLE hProcess = NULL;
    HANDLE hThread = NULL;

    // Rule #3: Ensure all input is validated
    if (url == NULL) {
        fprintf(stderr, "Error: URL is NULL.\n");
        return -1;
    }
    if (strlen(url) > MAX_URL_LENGTH) {
        fprintf(stderr, "Error: URL exceeds maximum length of %d.\n", MAX_URL_LENGTH);
        return -1;
    }
    if (strncmp(url, "https://", 8) != 0) {
        fprintf(stderr, "Error: URL must use 'https'.\n");
        return -1;
    }

    // Rules #7 & #8: Create a secure temporary file to avoid path traversal and TOCTOU issues.
    DWORD pathLen = GetTempPathW(MAX_PATH, tempPath);
    if (pathLen == 0 || pathLen > MAX_PATH) {
        fprintf(stderr, "Error: GetTempPathW failed.\n");
        return -1;
    }

    if (GetTempFileNameW(tempPath, L"DNE", 0, tempFileName) == 0) {
        fprintf(stderr, "Error: GetTempFileNameW failed.\n");
        return -1;
    }

    // Open the temporary file for writing in binary mode
    if (_wfopen_s(&fp, tempFileName, L"wb") != 0 || fp == NULL) {
        fprintf(stderr, "Error: Could not open temporary file for writing.\n");
        // Manually delete the file if opening failed, as GetTempFileNameW created it.
        DeleteFileW(tempFileName);
        return -1;
    }

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: curl_easy_init() failed.\n");
        result = -1;
        goto cleanup_file;
    }

    // Configure libcurl for a secure download
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // Rules #1, #2, #9: Enforce secure TLS v1.2+ with peer and host verification.
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    // Set other security and robustness options
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, (long)MAX_REDIRS);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, REQUEST_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "secure-downloader/1.0");

    // Perform the download
    res = curl_easy_perform(curl);
    
    // Close the file handle before attempting to execute the file
    fclose(fp);
    fp = NULL;

    if (res != CURLE_OK) {
        fprintf(stderr, "Error: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        result = -1;
        goto cleanup_curl;
    }
    
    // Execute the downloaded file
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // lpCommandLine must be a writable string
    wchar_t commandLine[MAX_PATH];
    if (wcscpy_s(commandLine, MAX_PATH, tempFileName) != 0) {
        fprintf(stderr, "Error: Failed to copy filename to command line buffer.\n");
        result = -1;
        goto cleanup_curl;
    }

    // Rule #4: Use CreateProcessW for safer execution than system().
    if (!CreateProcessW(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Error: CreateProcessW failed (%lu).\n", GetLastError());
        result = -1;
        goto cleanup_curl;
    }
    
    hProcess = pi.hProcess;
    hThread = pi.hThread;

    printf("Successfully executed '%ls'. Waiting for completion...\n", tempFileName);
    WaitForSingleObject(hProcess, INFINITE);

    DWORD exitCode;
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        printf("Process exited with code: %lu\n", exitCode);
    }

    result = 0; // Success

    CloseHandle(hProcess);
    CloseHandle(hThread);

cleanup_curl:
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    
    if (!DeleteFileW(tempFileName)) {
        fprintf(stderr, "Warning: Failed to delete temporary file '%ls'.\n", tempFileName);
    } else {
        printf("Successfully cleaned up temporary file '%ls'.\n", tempFileName);
    }

    return result;

cleanup_file:
    if (fp) {
        fclose(fp);
    }
    // Delete the file if we created it but failed before the main cleanup block
    if (wcslen(tempFileName) > 0) {
        DeleteFileW(tempFileName);
    }
    return result;

#else
    // Non-Windows implementation stub
    fprintf(stderr, "This function is only implemented for Windows.\n");
    if (url == NULL || strncmp(url, "https://", 8) != 0) {
        return -1;
    }
    return -1; // Indicate failure on non-supported platforms
#endif
}

int main() {
    printf("--- Test Case 1: Valid URL (placeholder - will fail download) ---\n");
    // NOTE: This URL is a placeholder. For a real test, replace it with a URL
    // to a known-safe, small executable file from a trusted source.
    // The current URL will likely fail the download step, which is expected.
    int result1 = downloadAndExecute("https://live.sysinternals.com/nonexistent-dummy-tool.exe");
    printf("Test Case 1 %s.\n\n", (result1 == 0) ? "succeeded" : "failed");

    printf("--- Test Case 2: Invalid protocol (http) ---\n");
    int result2 = downloadAndExecute("http://example.com/program.exe");
    printf("Test Case 2 %s (expected failure).\n\n", (result2 == 0) ? "succeeded" : "failed");

    printf("--- Test Case 3: NULL URL ---\n");
    int result3 = downloadAndExecute(NULL);
    printf("Test Case 3 %s (expected failure).\n\n", (result3 == 0) ? "succeeded" : "failed");

    printf("--- Test Case 4: Non-existent domain ---\n");
    int result4 = downloadAndExecute("https://domain-that-does-not-exist-12345.com/file.exe");
    printf("Test Case 4 %s (expected failure).\n\n", (result4 == 0) ? "succeeded" : "failed");

    printf("--- Test Case 5: URL to a non-executable (text file) ---\n");
    // This should download successfully but the created process might exit immediately
    // or fail to launch visibly depending on the OS's handling of text files.
    // This tests the full download and cleanup path.
    int result5 = downloadAndExecute("https://raw.githubusercontent.com/curl/curl/master/README.md");
    printf("Test Case 5 %s (execution behavior depends on OS).\n\n", (result5 == 0) ? "succeeded" : "failed");

    return 0;
}