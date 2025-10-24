#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
// Required for libcurl
#include <curl/curl.h>
// Required for network address resolution and checking
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring> // For strncmp

// Note: This code requires libcurl. Compile with:
// g++ your_file.cpp -o your_program -lcurl

const long CONNECT_TIMEOUT = 5L; // 5 seconds
const long TOTAL_TIMEOUT = 10L;  // 10 seconds
const size_t MAX_RESPONSE_SIZE = 4096; // 4KB limit

// Callback function for libcurl to write received data into a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::string* response = static_cast<std::string*>(userp);
    size_t total_size = size * nmemb;
    if (response->length() + total_size > MAX_RESPONSE_SIZE) {
        total_size = MAX_RESPONSE_SIZE - response->length();
        if (total_size == 0) return 0; // Prevent further writing if limit reached
    }
    response->append(static_cast<char*>(contents), total_size);
    return size * nmemb; // Return original size to avoid curl error
}

// Function to check if an IP address is in a private or loopback range
bool isPrivateOrLoopbackIP(const char* ip_str) {
    if (strcmp(ip_str, "127.0.0.1") == 0 || strcmp(ip_str, "::1") == 0) return true;

    // Check private ranges
    if (strncmp(ip_str, "10.", 3) == 0) return true;
    if (strncmp(ip_str, "192.168.", 8) == 0) return true;
    if (strncmp(ip_str, "172.", 4) == 0) {
        int second_octet = atoi(ip_str + 4);
        if (second_octet >= 16 && second_octet <= 31) return true;
    }
    // Check link-local
    if (strncmp(ip_str, "169.254.", 8) == 0) return true;
    
    return false;
}

std::string makeHttpRequest(const std::string& url_string) {
    // 1. Security Check: Protocol validation
    if (url_string.rfind("http://", 0) != 0 && url_string.rfind("https://", 0) != 0) {
        return "Error: Invalid protocol. Only HTTP and HTTPS are allowed.";
    }

    // 2. Security Check: SSRF Prevention via DNS resolution
    try {
        size_t host_start = url_string.find("://") + 3;
        size_t host_end = url_string.find("/", host_start);
        std::string hostname = url_string.substr(host_start, host_end - host_start);
        
        // Remove port if present
        size_t port_pos = hostname.find(":");
        if (port_pos != std::string::npos) {
            hostname = hostname.substr(0, port_pos);
        }

        addrinfo hints = {}, *addrs;
        hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM;
        
        if (getaddrinfo(hostname.c_str(), NULL, &hints, &addrs) != 0) {
            return "Error: Host could not be resolved.";
        }
        
        // Ensure we free the addrinfo struct
        std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> addrs_ptr(addrs, &freeaddrinfo);

        for (addrinfo* rp = addrs; rp != NULL; rp = rp->ai_next) {
            char ip_str[INET6_ADDRSTRLEN];
            void* addr;
            if (rp->ai_family == AF_INET) { // IPv4
                addr = &((sockaddr_in*)rp->ai_addr)->sin_addr;
            } else { // IPv6
                addr = &((sockaddr_in6*)rp->ai_addr)->sin6_addr;
            }
            inet_ntop(rp->ai_family, addr, ip_str, sizeof(ip_str));
            if (isPrivateOrLoopbackIP(ip_str)) {
                return "Error: SSRF attempt detected. Requests to local/private networks are not allowed.";
            }
        }
    } catch (const std::out_of_range&) {
        return "Error: Could not parse hostname from URL.";
    }

    // 3. Make the Connection using libcurl
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize libcurl.";
    }
    
    std::string response_string;
    std::string header_string;
    
    curl_easy_setopt(curl, CURLOPT_URL, url_string.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TOTAL_TIMEOUT);
    // Security: Disable redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    
    std::string result;
    if (res != CURLE_OK) {
        result = "Error: curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        result = "Status Code: " + std::to_string(response_code) + "\n";
        result += "Response Body (first " + std::to_string(MAX_RESPONSE_SIZE) + " bytes):\n";
        result += response_string;
    }
    
    curl_easy_cleanup(curl);
    return result;
}


int main() {
    std::vector<std::string> testUrls = {
        "https://api.ipify.org", // Valid public URL
        "http://google.com",       // Valid public URL that redirects
        "http://127.0.0.1",        // SSRF test (loopback)
        "http://192.168.1.1",      // SSRF test (private network)
        "https://thissitedoesnotexist.invalidtld", // Invalid host
        "file:///etc/passwd"       // Invalid protocol test
    };

    curl_global_init(CURL_GLOBAL_ALL);

    for (const auto& url : testUrls) {
        std::cout << "=========================================" << std::endl;
        std::cout << "Requesting URL: " << url << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        std::string response = makeHttpRequest(url);
        std::cout << response << std::endl;
        std::cout << "=========================================\n" << std::endl;
    }

    curl_global_cleanup();
    return 0;
}