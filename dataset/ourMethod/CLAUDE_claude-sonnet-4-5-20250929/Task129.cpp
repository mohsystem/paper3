
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <curl/curl.h>

// Security: Callback for libcurl to write received data
// Uses std::vector for safe memory management
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::vector<char>* buffer = static_cast<std::vector<char>*>(userp);
    
    // Security: Check for integer overflow before allocation
    if (realsize > SIZE_MAX - buffer->size()) {
        std::cerr << "Buffer size overflow prevented" << std::endl;
        return 0; // Signal error to libcurl
    }
    
    // Security: Limit maximum buffer size to prevent memory exhaustion (10MB)
    const size_t MAX_BUFFER_SIZE = 10 * 1024 * 1024;
    if (buffer->size() + realsize > MAX_BUFFER_SIZE) {
        std::cerr << "Maximum buffer size exceeded" << std::endl;
        return 0;
    }
    
    try {
        buffer->insert(buffer->end(), static_cast<char*>(contents), 
                      static_cast<char*>(contents) + realsize);
    } catch (const std::bad_alloc&) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 0;
    }
    
    return realsize;
}

// Security: Validate filename to prevent directory traversal and injection attacks
bool validateFilename(const std::string& filename) {
    // Security: Reject empty filenames
    if (filename.empty() || filename.length() > 255) {
        return false;
    }
    
    // Security: Reject path traversal sequences
    if (filename.find("..") != std::string::npos) {
        return false;
    }
    
    // Security: Reject absolute paths and path separators
    if (filename.find('/') != std::string::npos || 
        filename.find('\\\\') != std::string::npos ||\n        filename[0] == '.' ||\n        filename.find(':') != std::string::npos) {\n        return false;\n    }\n    \n    // Security: Whitelist allowed characters (alphanumeric, dash, underscore, dot)\n    for (char c : filename) {\n        if (!std::isalnum(static_cast<unsigned char>(c)) && \n            c != '-' && c != '_' && c != '.') {\n            return false;\n        }\n    }\n    \n    // Security: Reject files starting or ending with dot\n    if (filename[0] == '.' || filename[filename.length() - 1] == '.') {
        return false;
    }
    
    return true;
}

// Security: Validate URL to ensure HTTPS and proper format
bool validateURL(const std::string& url) {
    // Security: Must use HTTPS
    if (url.substr(0, 8) != "https://") {
        return false;
    }
    
    // Security: Check length bounds
    if (url.length() < 10 || url.length() > 2048) {
        return false;
    }
    
    return true;
}

// Security: Retrieve file from server with strict validation
std::string retrieveFile(const std::string& baseURL, const std::string& filename) {
    // Security: Validate filename before use
    if (!validateFilename(filename)) {
        return "Error: Invalid filename";
    }
    
    // Security: Construct URL safely
    std::string url = baseURL + "/" + filename;
    
    // Security: Validate complete URL
    if (!validateURL(url)) {
        return "Error: Invalid URL";
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize CURL";
    }
    
    std::vector<char> buffer;
    buffer.reserve(4096); // Pre-allocate reasonable size
    
    // Security: Configure CURL with secure options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    
    // Security: Enable SSL/TLS certificate verification (CWE-295)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Security: Enable hostname verification (CWE-297)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Security: Set timeout to prevent indefinite hanging
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    
    // Security: Follow redirects with limit
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    
    // Security: Disable insecure protocols
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    
    CURLcode res = curl_easy_perform(curl);
    
    std::string result;
    if (res != CURLE_OK) {
        result = "Error: " + std::string(curl_easy_strerror(res));
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        if (http_code == 200) {
            // Security: Safe conversion with bounds
            result = std::string(buffer.begin(), buffer.end());
        } else {
            result = "Error: HTTP " + std::to_string(http_code);
        }
    }
    
    curl_easy_cleanup(curl);
    return result;
}

int main() {
    // Security: Use HTTPS base URL only
    const std::string baseURL = "https://example.com/files";
    
    // Test case 1: Valid filename
    std::cout << "Test 1 - Valid filename:" << std::endl;
    std::string result1 = retrieveFile(baseURL, "document.txt");
    std::cout << (result1.substr(0, 6) == "Error:" ? result1 : "Success") << std::endl << std::endl;
    
    // Test case 2: Attempt directory traversal (should be rejected)
    std::cout << "Test 2 - Directory traversal attempt:" << std::endl;
    std::string result2 = retrieveFile(baseURL, "../etc/passwd");
    std::cout << result2 << std::endl << std::endl;
    
    // Test case 3: Absolute path (should be rejected)
    std::cout << "Test 3 - Absolute path attempt:" << std::endl;
    std::string result3 = retrieveFile(baseURL, "/etc/passwd");
    std::cout << result3 << std::endl << std::endl;
    
    // Test case 4: Invalid characters (should be rejected)
    std::cout << "Test 4 - Invalid characters:" << std::endl;
    std::string result4 = retrieveFile(baseURL, "file;rm -rf");
    std::cout << result4 << std::endl << std::endl;
    
    // Test case 5: Valid alphanumeric filename
    std::cout << "Test 5 - Valid alphanumeric filename:" << std::endl;
    std::string result5 = retrieveFile(baseURL, "data_file-123.json");
    std::cout << (result5.substr(0, 6) == "Error:" ? result5 : "Success") << std::endl;
    
    return 0;
}
