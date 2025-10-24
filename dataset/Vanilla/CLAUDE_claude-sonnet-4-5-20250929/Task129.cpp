
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <algorithm>
#include <regex>

class Task129 {
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    static std::string sanitizeFileName(const std::string& fileName) {
        if (fileName.empty()) {
            return "default.txt";
        }
        
        std::string sanitized = fileName;
        
        // Remove directory traversal attempts
        size_t pos;
        while ((pos = sanitized.find("..")) != std::string::npos) {
            sanitized.erase(pos, 2);
        }
        
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '/'), sanitized.end());
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\\\\'), sanitized.end());
        
        // Keep only alphanumeric, dots, hyphens, and underscores
        std::regex pattern("[^a-zA-Z0-9._-]");
        sanitized = std::regex_replace(sanitized, pattern, "");
        
        return sanitized;
    }
    
public:
    static std::string retrieveFileFromServer(const std::string& serverUrl, const std::string& fileName) {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;
        
        // Sanitize filename
        std::string sanitizedFileName = sanitizeFileName(fileName);
        
        // Construct full URL
        std::string fullUrl = serverUrl + "/" + sanitizedFileName;
        
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            
            res = curl_easy_perform(curl);
            
            if (res != CURLE_OK) {
                readBuffer = "Error: " + std::string(curl_easy_strerror(res));
            }
            
            curl_easy_cleanup(curl);
        } else {
            readBuffer = "Error: Failed to initialize CURL";
        }
        
        return readBuffer;
    }
};

int main() {
    // Test cases
    std::string serverUrl = "http://example.com/files";
    
    std::cout << "Test Case 1: Valid file name" << std::endl;
    std::cout << Task129::retrieveFileFromServer(serverUrl, "document.txt") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2: File with numbers" << std::endl;
    std::cout << Task129::retrieveFileFromServer(serverUrl, "report_2024.pdf") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Attempting path traversal (sanitized)" << std::endl;
    std::cout << Task129::retrieveFileFromServer(serverUrl, "../../../etc/passwd") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4: File with special characters" << std::endl;
    std::cout << Task129::retrieveFileFromServer(serverUrl, "data-file_01.json") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Empty filename" << std::endl;
    std::cout << Task129::retrieveFileFromServer(serverUrl, "") << std::endl;
    std::cout << std::endl;
    
    return 0;
}
