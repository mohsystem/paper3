
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>

#ifdef _WIN32
    #include <windows.h>
    #include <urlmon.h>
    #pragma comment(lib, "urlmon.lib")
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
#endif

using namespace std;

string downloadAndExecute(const string& url) {
    try {
        // Generate temporary file name
        string fileName = "downloaded_" + to_string(time(nullptr)) + ".exe";
        
        #ifdef _WIN32
            string tempPath = string(getenv("TEMP")) + "\\\\" + fileName;\n        #else\n            string tempPath = "/tmp/" + fileName;\n        #endif\n        \n        // Download file\n        #ifdef _WIN32\n            HRESULT hr = URLDownloadToFileA(nullptr, url.c_str(), tempPath.c_str(), 0, nullptr);\n            if (FAILED(hr)) {\n                return "Error: Failed to download file";\n            }\n        #else\n            string curlCmd = "curl -s -o " + tempPath + " " + url;\n            int downloadResult = system(curlCmd.c_str());\n            if (downloadResult != 0) {\n                return "Error: Failed to download file";\n            }\n            \n            // Make executable\n            chmod(tempPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);\n        #endif\n        \n        // Execute file\n        #ifdef _WIN32\n            STARTUPINFOA si;\n            PROCESS_INFORMATION pi;\n            ZeroMemory(&si, sizeof(si));\n            si.cb = sizeof(si);\n            ZeroMemory(&pi, sizeof(pi));\n            \n            if (CreateProcessA(tempPath.c_str(), nullptr, nullptr, nullptr, FALSE, 0, \n                              nullptr, nullptr, &si, &pi)) {\n                WaitForSingleObject(pi.hProcess, 30000);\n                CloseHandle(pi.hProcess);\n                CloseHandle(pi.hThread);\n            }\n        #else\n            pid_t pid = fork();\n            if (pid == 0) {\n                execl(tempPath.c_str(), tempPath.c_str(), nullptr);\n                exit(0);\n            } else if (pid > 0) {\n                int status;\n                waitpid(pid, &status, 0);\n            }\n        #endif\n        \n        return "File downloaded to: " + tempPath + "\
Execution completed";\n        \n    } catch (const exception& e) {\n        return string("Error: ") + e.what();\n    }\n}\n\nint main() {\n    cout << "=== SECURITY WARNING: This program downloads and executes files ===" << endl << endl;\n    \n    // Test cases - Using safe/mock URLs\n    string testUrls[] = {\n        "https://example.com/safe-program.exe",\n        "https://example.com/test-app.exe",\n        "https://example.com/utility.exe",\n        "https://example.com/installer.exe",\n        "https://example.com/application.exe"\n    };\n    \n    for (int i = 0; i < 5; i++) {\n        cout << "Test Case " << (i + 1) << ":" << endl;\n        cout << "URL: " << testUrls[i] << endl;\n        cout << "Result: " << downloadAndExecute(testUrls[i]) << endl;\n        cout << "---" << endl;
    }
    
    return 0;
}
