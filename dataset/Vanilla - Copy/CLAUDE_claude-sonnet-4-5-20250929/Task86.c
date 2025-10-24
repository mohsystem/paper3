
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <urlmon.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
#endif

char* downloadAndExecute(const char* url) {
    static char result[512];
    char fileName[256];
    char tempPath[512];
    
    // Generate temporary file name
    sprintf(fileName, "downloaded_%ld.exe", (long)time(NULL));
    
    #ifdef _WIN32
        char* temp = getenv("TEMP");
        sprintf(tempPath, "%s\\\\%s", temp, fileName);
    #else
        sprintf(tempPath, "/tmp/%s", fileName);
    #endif
    
    // Download file
    #ifdef _WIN32
        HRESULT hr = URLDownloadToFileA(NULL, url, tempPath, 0, NULL);
        if (FAILED(hr)) {
            strcpy(result, "Error: Failed to download file");
            return result;
        }
    #else
        char curlCmd[1024];
        sprintf(curlCmd, "curl -s -o %s %s", tempPath, url);
        int downloadResult = system(curlCmd);
        if (downloadResult != 0) {
            strcpy(result, "Error: Failed to download file");
            return result;
        }
        
        // Make executable
        chmod(tempPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    #endif
    
    // Execute file
    #ifdef _WIN32
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        if (CreateProcessA(tempPath, NULL, NULL, NULL, FALSE, 0, 
                          NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 30000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    #else
        pid_t pid = fork();
        if (pid == 0) {
            execl(tempPath, tempPath, NULL);
            exit(0);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    #endif
    
    sprintf(result, "File downloaded to: %s\\nExecution completed", tempPath);
    return result;
}

int main() {
    printf("=== SECURITY WARNING: This program downloads and executes files ===\\n\\n");
    
    // Test cases - Using safe/mock URLs
    const char* testUrls[] = {
        "https://example.com/safe-program.exe",
        "https://example.com/test-app.exe",
        "https://example.com/utility.exe",
        "https://example.com/installer.exe",
        "https://example.com/application.exe"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        printf("URL: %s\\n", testUrls[i]);
        printf("Result: %s\\n", downloadAndExecute(testUrls[i]));
        printf("---\\n");
    }
    
    return 0;
}
