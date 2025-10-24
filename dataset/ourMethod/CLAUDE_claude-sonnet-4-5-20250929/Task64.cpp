
#include <iostream>
#include <string>
#include <memory>
#include <curl/curl.h>
#include <cstring>

// Maximum allowed response size to prevent memory exhaustion (10 MB)
constexpr size_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024;

// RAII wrapper for CURL handle
class CurlHandle {
private:
    CURL* handle;
    
public:
    CurlHandle() : handle(curl_easy_init()) {}
    
    ~CurlHandle() {
        if (handle) {
            curl_easy_cleanup(handle);
        }
    }
    
    // Prevent copying
    CurlHandle(const CurlHandle&) = delete;
    CurlHandle& operator=(const CurlHandle&) = delete;
    
    CURL* get() { return handle; }
    explicit operator bool() const { return handle != nullptr; }
};

// Secure string buffer with bounds checking
class ResponseBuffer {
private:
    std::string data;
    
public:
    bool append(const char* content, size_t size) {
        // Check for size overflow
        if (data.size() > MAX_RESPONSE_SIZE || size > MAX_RESPONSE_SIZE - data.size()) {
            return false;
        }
        
        try {
            data.append(content, size);
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    const std::string& get() const { return data; }
    size_t size() const { return data.size(); }
};

// Write callback for libcurl with bounds checking
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    if (!contents || !userp) return 0;
    
    // Check for integer overflow
    if (nmemb > 0 && size > SIZE_MAX / nmemb) {
        return 0;
    }
    
    size_t realsize = size * nmemb;
    auto* buffer = static_cast<ResponseBuffer*>(userp);
    
    if (!buffer->append(static_cast<const char*>(contents), realsize)) {
        return 0; // Signal error to libcurl
    }
    
    return realsize;
}

// Validate URL - ensure HTTPS only
static bool validate_url(const std::string& url) {
    if (url.empty() || url.length() < 8 || url.length() > 2048) {
        return false;
    }
    
    // Enforce HTTPS only for security
    return url.compare(0, 8, "https://") == 0;
}

// Basic JSON structure validation
static bool validate_json_structure(const std::string& json) {
    if (json.empty()) return false;
    
    int brace_count = 0;
    int bracket_count = 0;
    bool in_string = false;
    bool escape_next = false;
    
    for (char c : json) {
        if (escape_next) {
            escape_next = false;
            continue;
        }
        
        if (c == '\\\\') {\n            escape_next = true;\n            continue;\n        }\n        \n        if (c == '"') {\n            in_string = !in_string;\n            continue;\n        }\n        \n        if (in_string) continue;\n        \n        if (c == '{') brace_count++;\n        else if (c == '}') brace_count--;\n        else if (c == '[') bracket_count++;\n        else if (c == ']') bracket_count--;\n        \n        if (brace_count < 0 || bracket_count < 0) return false;\n    }\n    \n    return (brace_count == 0 && bracket_count == 0 && !in_string);\n}\n\n// Fetch JSON from URL with secure HTTPS settings\nstatic std::string fetch_json_from_url(const std::string& url) {\n    // Validate URL before any network operation\n    if (!validate_url(url)) {\n        std::cerr << "Error: Invalid or insecure URL. Only HTTPS URLs are allowed." << std::endl;\n        return "";\n    }\n    \n    CurlHandle curl;\n    if (!curl) {\n        std::cerr << "Error: Failed to initialize CURL." << std::endl;\n        return "";\n    }\n    \n    ResponseBuffer buffer;\n    \n    // Set URL\n    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());\n    \n    // Security: Enable SSL/TLS certificate verification\n    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);\n    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);\n    \n    // Security: Enforce TLS 1.2 minimum\n    curl_easy_setopt(curl.get(), CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);\n    \n    // Security: Restrict to HTTPS protocol only\n    curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);\n    \n    // Set write callback\n    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_callback);\n    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);\n    \n    // Security: Set reasonable timeouts\n    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);\n    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);\n    \n    // Security: Limit redirects\n    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);\n    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 3L);\n    curl_easy_setopt(curl.get(), CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);\n    \n    // Perform request\n    CURLcode res = curl_easy_perform(curl.get());\n    \n    if (res != CURLE_OK) {\n        std::cerr << "Error: Failed to fetch URL: " << curl_easy_strerror(res) << std::endl;\n        return "";\n    }\n    \n    // Check HTTP response code\n    long response_code = 0;\n    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response_code);\n    if (response_code != 200) {\n        std::cerr << "Error: HTTP request failed with code: " << response_code << std::endl;\n        return "";\n    }\n    \n    // Validate JSON structure\n    if (!validate_json_structure(buffer.get())) {\n        std::cerr << "Error: Invalid JSON structure." << std::endl;\n        return "";\n    }\n    \n    return buffer.get();\n}\n\nint main(int argc, char* argv[]) {\n    // Test cases when no URL provided\n    if (argc < 2) {\n        std::cout << "Running test cases...\
" << std::endl;\n        \n        // Test case 1: Valid HTTPS URL\n        std::cout << "Test 1: Valid HTTPS URL" << std::endl;\n        std::string json1 = fetch_json_from_url("https://jsonplaceholder.typicode.com/posts/1");\n        if (!json1.empty()) {\n            std::cout << "Success: Fetched JSON data" << std::endl;\n            std::cout << "Data: " << json1.substr(0, 100) << "...\
" << std::endl;\n        } else {\n            std::cout << "Failed to fetch data\
" << std::endl;\n        }\n        \n        // Test case 2: Invalid URL (HTTP)\n        std::cout << "Test 2: Invalid URL (HTTP not allowed)" << std::endl;\n        std::string json2 = fetch_json_from_url("http://example.com/data.json");\n        if (!json2.empty()) {\n            std::cout << "Unexpected success\
" << std::endl;\n        } else {\n            std::cout << "Correctly rejected HTTP URL\
" << std::endl;\n        }\n        \n        // Test case 3: Empty URL\n        std::cout << "Test 3: Empty URL" << std::endl;\n        std::string json3 = fetch_json_from_url("");\n        if (!json3.empty()) {\n            std::cout << "Unexpected success\
" << std::endl;\n        } else {\n            std::cout << "Correctly rejected empty URL\
" << std::endl;\n        }\n        \n        // Test case 4: Malformed URL\n        std::cout << "Test 4: Malformed URL" << std::endl;\n        std::string json4 = fetch_json_from_url("not-a-url");\n        if (!json4.empty()) {\n            std::cout << "Unexpected success\
" << std::endl;\n        } else {\n            std::cout << "Correctly rejected malformed URL\
" << std::endl;\n        }\n        \n        // Test case 5: Another valid HTTPS URL\n        std::cout << "Test 5: Another valid HTTPS URL" << std::endl;\n        std::string json5 = fetch_json_from_url("https://jsonplaceholder.typicode.com/users/1");\n        if (!json5.empty()) {\n            std::cout << "Success: Fetched JSON data" << std::endl;\n            std::cout << "Data: " << json5.substr(0, 100) << "...\
" << std::endl;\n        } else {\n            std::cout << "Failed to fetch data\
" << std::endl;\n        }\n        \n        return 0;\n    }\n    \n    // Validate command line argument\n    std::string url(argv[1]);\n    if (url.length() > 2048) {\n        std::cerr << "Error: URL too long." << std::endl;\n        return 1;\n    }\n    \n    // Fetch JSON from provided URL\n    std::string json_data = fetch_json_from_url(url);\n    \n    if (!json_data.empty()) {\n        std::cout << "Successfully fetched JSON:\
" << json_data << std::endl;\n        return 0;\n    } else {\n        std::cerr << "Failed to fetch JSON from URL." << std::endl;
        return 1;
    }
}
