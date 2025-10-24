
#include <iostream>
#include <string>
#include <cstring>
#include <curl/curl.h>
#include <algorithm>
#include <cctype>

// Callback function for libcurl to write received data
// Returns the number of bytes written to prevent CWE-119
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    
    // Prevent excessive memory allocation (DoS protection)
    const size_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024; // 10MB limit
    if (buffer->size() + realsize > MAX_RESPONSE_SIZE) {
        std::cerr << "Response size exceeded maximum allowed" << std::endl;
        return 0; // Signal error to libcurl
    }
    
    try {
        buffer->append(static_cast<char*>(contents), realsize);
    } catch (const std::bad_alloc&) {
        std::cerr << "Memory allocation failed" << std::endl;
        return 0; // Signal error
    }
    
    return realsize;
}

// Validates URL format and ensures HTTPS protocol (CWE-20, CWE-319)
bool validateURL(const std::string& url) {
    // Check minimum length
    if (url.length() < 8) {
        return false;
    }
    
    // Ensure HTTPS protocol only (prevents CWE-319)
    if (url.substr(0, 8) != "https://") {
        return false;
    }
    
    // Check for null bytes (CWE-158)
    if (url.find('\\0') != std::string::npos) {
        return false;
    }
    
    // Basic validation for reasonable URL length
    if (url.length() > 2048) {
        return false;
    }
    
    return true;
}

// Extracts title from HTML content (CWE-20)
std::string extractTitle(const std::string& html) {
    // Find opening title tag (case-insensitive)
    size_t titleStart = std::string::npos;
    size_t titleEnd = std::string::npos;
    
    // Convert to lowercase for case-insensitive search
    std::string htmlLower = html;
    std::transform(htmlLower.begin(), htmlLower.end(), htmlLower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    titleStart = htmlLower.find("<title>");
    if (titleStart != std::string::npos) {
        titleStart += 7; // Length of "<title>"
        titleEnd = htmlLower.find("</title>", titleStart);
    }
    
    // If title tags not found, return empty string
    if (titleStart == std::string::npos || titleEnd == std::string::npos) {
        return "";
    }
    
    // Validate extraction bounds (CWE-125)
    if (titleEnd < titleStart || titleEnd > html.length()) {
        return "";
    }
    
    // Extract title from original HTML (preserves case)
    size_t length = titleEnd - titleStart;
    if (length > 1024) { // Reasonable title length limit
        length = 1024;
    }
    
    return html.substr(titleStart, length);
}

// Fetches webpage and extracts title (CWE-295, CWE-297, CWE-319)
std::string fetchPageTitle(const std::string& url) {
    CURL* curl = nullptr;
    CURLcode res;
    std::string responseBuffer;
    std::string title;
    
    // Initialize curl
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }
    
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    
    // Enable SSL/TLS certificate verification (CWE-295)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    
    // Enable hostname verification (CWE-297)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Enforce TLS 1.2 or higher (CWE-319, CWE-327)
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    // Set callback function for response data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    
    // Set timeout to prevent hanging (30 seconds)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    // Follow redirects with a limit (CWE-601)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    
    // Set User-Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureHTTPClient/1.0");
    
    // Perform request
    res = curl_easy_perform(curl);
    
    // Check for errors (CWE-252)
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return "";
    }
    
    // Check HTTP response code
    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    // Cleanup curl
    curl_easy_cleanup(curl);
    
    // Validate HTTP response code
    if (response_code < 200 || response_code >= 400) {
        std::cerr << "HTTP error code: " << response_code << std::endl;
        return "";
    }
    
    // Extract title from HTML
    title = extractTitle(responseBuffer);
    
    return title;
}

int main(int argc, char* argv[]) {
    // Validate command line arguments (CWE-20)
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <https_url>" << std::endl;
        return 1;
    }
    
    // Check for null pointer (defensive programming)
    if (argv[1] == nullptr) {
        std::cerr << "Error: Invalid argument" << std::endl;
        return 1;
    }
    
    std::string url(argv[1]);
    
    // Validate URL format and protocol (CWE-20, CWE-319)
    if (!validateURL(url)) {
        std::cerr << "Error: Invalid URL. Must be HTTPS URL." << std::endl;
        return 1;
    }
    
    // Fetch page title
    std::string title = fetchPageTitle(url);
    
    if (title.empty()) {
        std::cout << "No title found or error occurred" << std::endl;
        return 1;
    }
    
    std::cout << "Page Title: " << title << std::endl;
    
    // Test cases (commented out for production)
    /*
    std::cout << "\\n=== Running Test Cases ===" << std::endl;
    
    std::string testURLs[] = {
        "https://www.example.com",
        "https://www.google.com",
        "https://www.github.com",
        "https://www.stackoverflow.com",
        "https://www.wikipedia.org"
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "\\nTest " << (i + 1) << ": " << testURLs[i] << std::endl;
        if (validateURL(testURLs[i])) {
            std::string testTitle = fetchPageTitle(testURLs[i]);
            if (!testTitle.empty()) {
                std::cout << "Title: " << testTitle << std::endl;
            } else {
                std::cout << "Failed to retrieve title" << std::endl;
            }
        } else {
            std::cout << "Invalid URL" << std::endl;
        }
    }
    */
    
    return 0;
}
