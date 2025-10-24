
#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <regex>
#include <curl/curl.h>

// Maximum URL length - Security: prevents memory exhaustion
constexpr size_t MAX_URL_LENGTH = 2048;

// Maximum response size (1MB) - Security: DoS prevention
constexpr size_t MAX_RESPONSE_SIZE = 1024 * 1024;

// Connection timeout in seconds - Security: prevents hanging
constexpr long TIMEOUT_SECONDS = 10;

/**
 * Callback function for libcurl to write response data.
 * Security: Enforces size limit to prevent memory exhaustion.
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    
    // Check for potential overflow - Security: prevents integer overflow
    if (realsize > MAX_RESPONSE_SIZE || response->size() > MAX_RESPONSE_SIZE - realsize) {
        return 0; // Stop reading
    }
    
    // Append data safely
    try {
        response->append(static_cast<char*>(contents), realsize);
    } catch (const std::exception&) {
        return 0; // Memory allocation failed
    }
    
    return realsize;
}

/**
 * Validate URL format and security constraints.
 * Security: Prevents SSRF by enforcing HTTPS and blocking private addresses.
 */
bool validateUrl(const std::string& url) {
    // Check null/empty and length - Security: input validation
    if (url.empty() || url.length() > MAX_URL_LENGTH) {
        return false;
    }
    
    // Check for null bytes - Security: prevents injection
    if (url.find('\\0') != std::string::npos) {
        return false;
    }
    
    // Only allow HTTPS protocol - Security: enforces encryption
    if (url.substr(0, 8) != "https://") {
        return false;
    }
    
    // Extract hostname for validation
    size_t host_start = 8; // After "https://"
    size_t host_end = url.find('/', host_start);
    if (host_end == std::string::npos) {
        host_end = url.length();
    }
    
    std::string host = url.substr(host_start, host_end - host_start);
    
    // Remove port if present
    size_t port_pos = host.find(':');
    if (port_pos != std::string::npos) {
        host = host.substr(0, port_pos);
    }
    
    // Block localhost and local hostnames - Security: SSRF prevention
    if (host == "localhost" || host == "127.0.0.1" || 
        host == "0.0.0.0" || host == "::1" || host == "[::1]") {
        return false;
    }
    
    // Block private IP ranges - Security: SSRF prevention
    if (host.substr(0, 3) == "10." || 
        host.substr(0, 8) == "192.168." ||
        host.substr(0, 4) == "172." ||
        host.substr(0, 4) == "127.") {
        return false;
    }
    
    // Basic hostname validation using regex
    std::regex hostname_pattern(R"(^[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?(\\.[a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?)*$)");
    if (!std::regex_match(host, hostname_pattern)) {
        return false;
    }
    
    return true;
}

/**
 * Make secure HTTPS request to validated URL.
 * Security: Uses libcurl with SSL/TLS certificate verification enabled.
 */
std::unique_ptr<std::string> makeHttpRequest(const std::string& url) {
    // Validate URL first - Security: input validation
    if (!validateUrl(url)) {
        return nullptr;
    }
    
    // Initialize curl
    CURL* curl = curl_easy_init();
    if (!curl) {
        return nullptr;
    }
    
    // Response container
    auto response = std::make_unique<std::string>();
    CURLcode res;
    
    // Set URL - Security: already validated
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Security: Enable SSL certificate verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    
    // Security: Enable hostname verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Set callback function for response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response.get());
    
    // Set timeouts - Security: prevents hanging connections
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, TIMEOUT_SECONDS);
    
    // Set User-Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureHTTPClient/1.0");
    
    // Follow redirects with limit - Security: prevents redirect loops
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    
    // Perform request
    res = curl_easy_perform(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return nullptr;
    }
    
    // Check HTTP response code
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    // Cleanup
    curl_easy_cleanup(curl);
    
    if (response_code != 200) {
        return nullptr;
    }
    
    return response;
}

int main() {
    std::cout << "Secure HTTPS Request Program\\n";
    std::cout << std::string(50, '=') << "\\n";
    
    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test cases demonstrating secure URL handling
    const char* testUrls[] = {
        // Valid HTTPS URL
        "https://www.example.com",
        // Invalid: HTTP instead of HTTPS
        "http://www.example.com",
        // Invalid: localhost (SSRF prevention)
        "https://localhost/admin",
        // Invalid: private IP (SSRF prevention)
        "https://192.168.1.1/secret",
        // Invalid: malformed URL
        "not-a-valid-url"
    };
    
    for (int i = 0; i < 5; ++i) {
        std::cout << "\\nTest Case " << (i + 1) << ":\\n";
        std::cout << "URL: " << testUrls[i] << "\\n";
        
        auto result = makeHttpRequest(testUrls[i]);
        
        if (result != nullptr) {
            std::string preview = result->length() > 200 ? 
                result->substr(0, 200) : *result;
            std::cout << "Success: Retrieved " << result->length() << " bytes\\n";
            std::cout << "Preview: " << preview << "...\\n";
        } else {
            std::cout << "Failed: URL validation failed or request error\\n";
        }
    }
    
    std::cout << "\\n" << std::string(50, '=') << "\\n";
    std::cout << "\\nNote: All URLs must use HTTPS and point to public domains\\n";
    
    // Cleanup curl globally
    curl_global_cleanup();
    
    return 0;
}
