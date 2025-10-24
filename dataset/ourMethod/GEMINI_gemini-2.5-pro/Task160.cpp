#include <iostream>
#include <string>
#include <vector>
#include <cstring> // For memset
#include <curl/curl.h>

// For SSRF check
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// Callback function for libcurl to write received data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    try {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Not enough memory (bad_alloc)." << std::endl;
        return 0;
    }
    return size * nmemb;
}

// Function to check for private/reserved IP addresses
bool isAddressPrivate(const struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        const auto* sin = (const struct sockaddr_in*)sa;
        const uint32_t addr = ntohl(sin->sin_addr.s_addr);
        if ((addr & 0xFF000000) == 0x0A000000 || // 10.0.0.0/8
            (addr & 0xFFF00000) == 0xAC100000 || // 172.16.0.0/12
            (addr & 0xFFFF0000) == 0xC0A80000 || // 192.168.0.0/16
            (addr & 0xFF000000) == 0x7F000000 || // 127.0.0.0/8
            (addr & 0xFFFF0000) == 0xA9FE0000) { // 169.254.0.0/16
            return true;
        }
    } else if (sa->sa_family == AF_INET6) {
        const auto* sin6 = (const struct sockaddr_in6*)sa;
        if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr) ||
            IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr) ||
            (sin6->sin6_addr.s6_addr[0] & 0xFE) == 0xFC) { // fc00::/7
            return true;
        }
    }
    return false;
}

// Validates host to prevent SSRF
bool isHostPrivate(const std::string& host) {
    struct addrinfo hints{}, *res = nullptr, *p = nullptr;
    
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) {
        return true; // Fail closed if host cannot be resolved
    }

    bool is_private = false;
    for (p = res; p != nullptr; p = p->ai_next) {
        if (isAddressPrivate(p->ai_addr)) {
            is_private = true;
            break;
        }
    }

    freeaddrinfo(res);
    return is_private;
}

std::string makeHttpRequest(const std::string& urlString) {
    if (urlString.rfind("http://", 0) != 0 && urlString.rfind("https://", 0) != 0) {
        return "Error: Invalid protocol. Only HTTP and HTTPS are allowed.";
    }

    size_t host_start = urlString.find("://") + 3;
    if (host_start == std::string::npos + 3) return "Error: Malformed URL.";
    
    size_t host_end = urlString.find_first_of(":/", host_start);
    std::string host = (host_end == std::string::npos) ? 
                       urlString.substr(host_start) : 
                       urlString.substr(host_start, host_end - host_start);

    if (host.empty()) return "Error: Could not extract hostname from URL.";

    if (isHostPrivate(host)) {
        return "Error: SSRF attempt detected. Requests to private networks are not allowed.";
    }

    CURL* curl = curl_easy_init();
    if (!curl) return "Error: Failed to initialize libcurl.";

    std::string readBuffer;
    
    curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

    std::string result;
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        result = "Error: curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code >= 200 && response_code < 300) {
            result = readBuffer;
        } else {
            result = "Error: HTTP request failed with code: " + std::to_string(response_code);
        }
    }

    curl_easy_cleanup(curl);
    return result;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const std::vector<std::string> testUrls = {
        "https://api.github.com", "http://example.com", "ftp://example.com",
        "https://127.0.0.1", "https://localhost", "http://192.168.1.1", "not-a-url"
    };

    for (size_t i = 0; i < testUrls.size(); ++i) {
        std::cout << "---- Test Case " << (i + 1) << ": " << testUrls[i] << " ----" << std::endl;
        std::string response = makeHttpRequest(testUrls[i]);
        if (response.length() > 300) {
            std::cout << response.substr(0, 300) << "..." << std::endl;
        } else {
            std::cout << response << std::endl;
        }
        std::cout << "-------------------------------------------\n" << std::endl;
    }

    curl_global_cleanup();
    return 0;
}