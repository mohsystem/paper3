#include <iostream>
#include <string>
#include <vector>
#include <memory>

// curl is a third-party library. On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
// On macOS (with Homebrew): brew install curl
// Compile with: g++ your_file.cpp -o your_app -lcurl
#include <curl/curl.h>

// For socket/DNS resolution
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring> // For memset

const int CONNECT_TIMEOUT_S = 5;
const int TOTAL_TIMEOUT_S = 10;
const size_t MAX_RESPONSE_BYTES = 1024;

/**
 * Checks if a socket address is unsafe (private, loopback, etc.).
 * Handles both IPv4 and IPv6.
 */
bool isAddressUnsafe(const struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        const auto* addr_v4 = reinterpret_cast<const struct sockaddr_in*>(sa);
        const uint32_t ip = ntohl(addr_v4->sin_addr.s_addr);
        
        // Check for private, loopback, and link-local ranges
        if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) || // 10.0.0.0/8
            (ip >= 0xAC100000 && ip <= 0xAC1FFFFF) || // 172.16.0.0/12
            (ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) || // 192.168.0.0/16
            (ip >= 0x7F000000 && ip <= 0x7FFFFFFF) || // 127.0.0.0/8 (loopback)
            (ip >= 0xA9FE0000 && ip <= 0xA9FEFFFF)) { // 169.254.0.0/16 (link-local)
            return true;
        }
    } else if (sa->sa_family == AF_INET6) {
        const auto* addr_v6 = reinterpret_cast<const struct sockaddr_in6*>(sa);
        const unsigned char* bytes = addr_v6->sin6_addr.s6_addr;

        // Check for loopback (::1)
        if (IN6_IS_ADDR_LOOPBACK(&addr_v6->sin6_addr)) return true;
        // Check for link-local (fe80::/10)
        if (IN6_IS_ADDR_LINKLOCAL(&addr_v6->sin6_addr)) return true;
        // Check for unique local (fc00::/7)
        if ((bytes[0] & 0xFE) == 0xFC) return true;
    }
    return false;
}

/**
 * libcurl write callback function to limit response size.
 */
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    size_t total_size = size * nmemb;
    size_t space_left = MAX_RESPONSE_BYTES - response->length();
    size_t to_append = std::min(total_size, space_left);
    
    if (to_append > 0) {
        response->append(ptr, to_append);
    }
    // We "accept" all data to prevent curl from erroring, but only store up to the limit.
    return total_size;
}

std::string makeHttpRequest(const std::string& urlString) {
    // Using libcurl's URL API for robust parsing
    CURLU *h = curl_url();
    if (!h) return "Error: Could not create CURLU handle.";

    // RAII for curl_url_cleanup
    std::unique_ptr<CURLU, decltype(&curl_url_cleanup)> url_handle(h, &curl_url_cleanup);
    
    if (curl_url_set(h, CURLUPART_URL, urlString.c_str(), 0) != CURLUE_OK) {
        return "Error: Invalid URL format.";
    }

    // 1. Validate scheme
    char *scheme = nullptr;
    curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
    std::unique_ptr<char, decltype(&curl_free)> scheme_ptr(scheme, &curl_free);
    if (!scheme || (std::string(scheme) != "http" && std::string(scheme) != "httpsor")) {
        return "Error: Invalid or disallowed scheme. Only HTTP and HTTPS are allowed.";
    }

    // 2. Resolve host and validate IP
    char *host = nullptr;
    curl_url_get(h, CURLUPART_HOST, &host, 0);
    std::unique_ptr<char, decltype(&curl_free)> host_ptr(host, &curl_free);
    if (!host) {
        return "Error: Could not determine host from URL.";
    }

    struct addrinfo hints{}, *result = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, nullptr, &hints, &result) != 0) {
        return std::string("Error: Could not resolve hostname: ") + host;
    }
    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> res_ptr(result, &freeaddrinfo);

    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        if (isAddressUnsafe(rp->ai_addr)) {
            char ip_str[INET6_ADDRSTRLEN];
            void* addr = (rp->ai_family == AF_INET) ?
                (void*)&((struct sockaddr_in*)rp->ai_addr)->sin_addr :
                (void*)&((struct sockaddr_in6*)rp->ai_addr)->sin6_addr;
            inet_ntop(rp->ai_family, addr, ip_str, sizeof(ip_str));
            return std::string("Error: SSRF attempt detected. Host resolves to a blocked IP address: ") + ip_str;
        }
    }

    // 3. Make the HTTP request
    CURL *curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize curl.";
    
    std::string response_body;
    curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // Do not follow redirects
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)CONNECT_TIMEOUT_S);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)TOTAL_TIMEOUT_S);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "cpp-secure-http-client/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return std::string("Error: curl_easy_perform() failed: ") + curl_easy_strerror(res);
    }
    
    return "Success (first " + std::to_string(MAX_RESPONSE_BYTES) + " bytes): " + response_body;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::vector<std::string> testUrls = {
        "https://www.example.com",     // Valid public URL
        "http://127.0.0.1/status",    // SSRF attempt (loopback IP)
        "http://localhost/admin",       // SSRF attempt (loopback name)
        "https://192.168.1.1",        // SSRF attempt (private IP)
        "file:///etc/passwd",           // SSRF attempt (disallowed scheme)
        "http://[::1]/",               // SSRF attempt (IPv6 loopback)
        "ftp://ftp.example.com"        // SSRF attempt (disallowed scheme)
    };
    
    for (const auto& url : testUrls) {
        std::cout << "Testing URL: " << url << std::endl;
        std::string result = makeHttpRequest(url);
        std::cout << "Result: " << result << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    curl_global_cleanup();
    return 0;
}