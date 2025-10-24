
// WARNING: This code demonstrates a dangerous pattern and should NOT be used in production.
// Downloading and executing arbitrary executables is a critical security vulnerability.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

#define MAX_DOWNLOAD_SIZE (500L * 1024 * 1024)  // 500MB limit
#define BUFFER_SIZE 8192

typedef struct {
    FILE* file;
    size_t downloaded;
} DownloadContext;

// Secure write callback with size limits per Rules #6
static size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    DownloadContext* ctx = (DownloadContext*)userdata;
    size_t total_size = size * nmemb;
    
    if (ctx == NULL || ctx->file == NULL) {
        return 0;
    }
    
    // Check size limit per Rules #6
    if (ctx->downloaded + total_size > MAX_DOWNLOAD_SIZE) {
        fprintf(stderr, "Error: File exceeds maximum size\\n");
        return 0;  // Signal error to libcurl
    }
    
    size_t written = fwrite(ptr, 1, total_size, ctx->file);
    if (written > 0) {
        ctx->downloaded += written;
    }
    
    return written;
}

// SECURITY WARNING: This function downloads and executes arbitrary code.
// In production, you MUST verify cryptographic signatures and implement sandboxing.
int download_and_execute(const char* url) {
    CURL* curl = NULL;
    CURLcode res;
    FILE* temp_file = NULL;
    char temp_path[256];
    int result = 0;
    DownloadContext ctx = {NULL, 0};
    
    // Input validation per Rules #5
    if (url == NULL || strlen(url) == 0) {
        fprintf(stderr, "Error: Invalid URL provided\\n");
        return 0;
    }
    
    // Check URL scheme (only HTTPS allowed) per Rules #1, #13
    if (strncmp(url, "https://", 8) != 0) {
        fprintf(stderr, "Error: Only HTTPS URLs are permitted\\n");
        return 0;
    }
    
    // Create temporary file with secure name
    snprintf(temp_path, sizeof(temp_path), "%s/download_XXXXXX.exe", 
             getenv("TEMP") ? getenv("TEMP") : "/tmp");
    
#ifdef _WIN32
    // Windows: Create with restrictive permissions
    if (tmpnam_s(temp_path, sizeof(temp_path)) != 0) {
        fprintf(stderr, "Error: Failed to create temp file name\\n");
        return 0;
    }
    
    temp_file = fopen(temp_path, "wb");
#else
    // POSIX: Create with O_EXCL and restrictive permissions (0600) per Rules #9
    int fd = mkstemp(temp_path);
    if (fd == -1) {
        fprintf(stderr, "Error: Failed to create temporary file\\n");
        return 0;
    }
    
    fchmod(fd, 0600);  // Owner read/write only
    temp_file = fdopen(fd, "wb");
#endif
    
    if (temp_file == NULL) {
        fprintf(stderr, "Error: Failed to open temporary file\\n");
        return 0;
    }
    
    ctx.file = temp_file;
    ctx.downloaded = 0;
    
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl == NULL) {
        fprintf(stderr, "Error: Failed to initialize CURL\\n");
        goto cleanup;
    }
    
    printf("Downloading from: %s\\n", url);
    printf("WARNING: Executing downloaded executables is dangerous!\\n");
    
    // Configure secure HTTPS connection per Rules #3, #4, #13
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    
    // Enforce TLS 1.2+ and certificate validation per Rules #13
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    // Security settings per Rules #1
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureDownloader/1.0");
    
    // Perform download
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: Download failed - %s\\n", curl_easy_strerror(res));
        goto cleanup;
    }
    
    fflush(temp_file);
    
    printf("Download complete: %zu bytes\\n", ctx.downloaded);
    printf("Saved to: %s\\n", temp_path);
    
    // CRITICAL SECURITY ISSUE: No signature verification!
    printf("\\nWARNING: No signature verification performed!\\n");
    printf("This executable could be malicious!\\n");
    
    fclose(temp_file);
    temp_file = NULL;
    
    // Execute the downloaded file (SECURITY ISSUE: Running untrusted code)
    printf("\\nExecuting: %s\\n", temp_path);
    
#ifdef _WIN32
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};
    
    if (CreateProcessA(temp_path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, 300000);  // 5 minute timeout
        
        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        printf("Exit code: %lu\\n", exitCode);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        result = 1;
    } else {
        fprintf(stderr, "Error: Failed to execute\\n");
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        execl(temp_path, temp_path, NULL);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        printf("Exit code: %d\\n", WEXITSTATUS(status));
        result = 1;
    } else {
        fprintf(stderr, "Error: Failed to fork\\n");
    }
#endif
    
cleanup:
    if (temp_file != NULL) {
        fclose(temp_file);
    }
    
    if (curl != NULL) {
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    
    // Cleanup temporary file
    remove(temp_path);
    
    return result;
}

int main(void) {
    printf("======================================================================\\n");
    printf("SECURITY WARNING: This program downloads and executes code!\\n");
    printf("This is a DANGEROUS operation and should only be used for demonstration.\\n");
    printf("======================================================================\\n\\n");
    
    // Test cases (DO NOT RUN with real executables)
    const char* test_urls[] = {
        "https://example.com/test.exe",     // Would fail - not a real file
        "http://insecure.com/test.exe",     // Rejected - not HTTPS
        "",                                  // Rejected - empty URL
        "https://",                         // Rejected - invalid format
        "https://example.com/large.exe"     // Would test size limits
    };
    
    printf("Test case validation (not executing):\\n");
    for (int i = 0; i < 5; i++) {
        printf("\\nTest %d: %s\\n", i + 1, test_urls[i][0] ? test_urls[i] : "(empty)");
        
        if (test_urls[i][0] == '\\0' || strncmp(test_urls[i], "https://", 8) != 0 || 
            strcmp(test_urls[i], "https://") == 0) {
            printf("  -> Would be rejected by validation\\n");
        }
    }
    
    return 0;
}
