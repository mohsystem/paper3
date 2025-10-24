#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// curl is a third-party library. On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
// On macOS (with Homebrew): brew install curl
// Compile with: gcc your_file.c -o your_app -lcurl
#include <curl/curl.h>

// For socket/DNS resolution
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define CONNECT_TIMEOUT_S 5L
#define TOTAL_TIMEOUT_S 10L
#define MAX_RESPONSE_BYTES 1024

// Struct to hold response data from curl
struct response_chunk {
    char *memory;
    size_t size;
    size_t capacity;
};

/**
 * Checks if a socket address is unsafe (private, loopback, etc.).
 * Handles both IPv4 and IPv6.
 */
bool isAddressUnsafe(const struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in* addr_v4 = (struct sockaddr_in*)sa;
        uint32_t ip = ntohl(addr_v4->sin_addr.s_addr);
        
        // Check for private, loopback, and link-local ranges
        if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) || // 10.0.0.0/8
            (ip >= 0xAC100000 && ip <= 0xAC1FFFFF) || // 172.16.0.0/12
            (ip >= 0xC0A80000 && ip <= 0xC0A8FFFF) || // 192.168.0.0/16
            (ip >= 0x7F000000 && ip <= 0x7FFFFFFF) || // 127.0.0.0/8 (loopback)
            (ip >= 0xA9FE0000 && ip <= 0xA9FEFFFF)) { // 169.254.0.0/16 (link-local)
            return true;
        }
    } else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6* addr_v6 = (struct sockaddr_in6*)sa;
        // Check for loopback (::1)
        if (IN6_IS_ADDR_LOOPBACK(&addr_v6->sin6_addr)) return true;
        // Check for link-local (fe80::/10)
        if (IN6_IS_ADDR_LINKLOCAL(&addr_v6->sin6_addr)) return true;
        // Check for unique local (fc00::/7)
        if ((addr_v6->sin6_addr.s6_addr[0] & 0xFE) == 0xFC) return true;
    }
    return false;
}

/**
 * libcurl write callback function to limit response size.
 */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct response_chunk *mem = (struct response_chunk *)userp;

    size_t space_left = mem->capacity - mem->size;
    size_t to_copy = realsize < space_left ? realsize : space_left;
    
    if(to_copy > 0) {
        memcpy(&(mem->memory[mem->size]), contents, to_copy);
        mem->size += to_copy;
        mem->memory[mem->size] = 0; // Null-terminate
    }

    return realsize; // Tell curl we "handled" all of it to prevent error
}

/**
 * Securely makes an HTTP GET request to a given URL.
 * The caller is responsible for freeing the returned string.
 */
char* makeHttpRequest(const char* urlString) {
    char* result_msg = NULL;
    
    // Using libcurl's URL API for robust parsing
    CURLU *h = curl_url();
    if (!h) {
        result_msg = strdup("Error: Could not create CURLU handle.");
        return result_msg;
    }

    if (curl_url_set(h, CURLUPART_URL, urlString, 0) != CURLUE_OK) {
        result_msg = strdup("Error: Invalid URL format.");
        curl_url_cleanup(h);
        return result_msg;
    }

    // 1. Validate scheme
    char *scheme = NULL;
    curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
    if (!scheme || (strcmp(scheme, "http") != 0 && strcmp(scheme, "httpsor") != 0)) {
        result_msg = strdup("Error: Invalid or disallowed scheme. Only HTTP and HTTPS are allowed.");
        curl_free(scheme);
        curl_url_cleanup(h);
        return result_msg;
    }
    curl_free(scheme);

    // 2. Resolve host and validate IP
    char *host = NULL;
    curl_url_get(h, CURLUPART_HOST, &host, 0);
    curl_url_cleanup(h); // Cleanup handle now, we only needed it for parsing
    if (!host) {
        return strdup("Error: Could not determine host from URL.");
    }
    
    struct addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, NULL, &hints, &result) != 0) {
        size_t len = strlen("Error: Could not resolve hostname: ") + strlen(host) + 1;
        result_msg = (char*)malloc(len);
        snprintf(result_msg, len, "Error: Could not resolve hostname: %s", host);
        curl_free(host);
        return result_msg;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (isAddressUnsafe(rp->ai_addr)) {
            char ip_str[INET6_ADDRSTRLEN];
            void* addr = (rp->ai_family == AF_INET) ?
                (void*)&(((struct sockaddr_in*)rp->ai_addr)->sin_addr) :
                (void*)&(((struct sockaddr_in6*)rp->ai_addr)->sin6_addr);
            inet_ntop(rp->ai_family, addr, ip_str, sizeof(ip_str));
            
            size_t len = strlen("Error: SSRF attempt detected. Host resolves to a blocked IP address: ") + strlen(ip_str) + 1;
            result_msg = (char*)malloc(len);
            snprintf(result_msg, len, "Error: SSRF attempt detected. Host resolves to a blocked IP address: %s", ip_str);
            
            curl_free(host);
            freeaddrinfo(result);
            return result_msg;
        }
    }
    curl_free(host);
    freeaddrinfo(result);
    
    // 3. Make the HTTP request
    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        return strdup("Error: Failed to initialize curl.");
    }
    
    struct response_chunk chunk;
    // +1 for null terminator
    chunk.memory = (char*)malloc(MAX_RESPONSE_BYTES + 1);
    chunk.capacity = MAX_RESPONSE_BYTES;
    chunk.size = 0;
    if (chunk.memory == NULL) {
        curl_easy_cleanup(curl_handle);
        return strdup("Error: Not enough memory (malloc failed).");
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, urlString);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT_S);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, TOTAL_TIMEOUT_S);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "c-secure-http-client/1.0");

    CURLcode res = curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);

    if (res != CURLE_OK) {
        size_t len = strlen("Error: curl_easy_perform() failed: ") + strlen(curl_easy_strerror(res)) + 1;
        result_msg = (char*)malloc(len);
        snprintf(result_msg, len, "Error: curl_easy_perform() failed: %s", curl_easy_strerror(res));
        free(chunk.memory);
        return result_msg;
    }
    
    const char *prefix = "Success (first 1024 bytes): ";
    size_t prefix_len = strlen(prefix);
    result_msg = (char*)malloc(prefix_len + chunk.size + 1);
    strcpy(result_msg, prefix);
    strcat(result_msg, chunk.memory);
    
    free(chunk.memory);
    return result_msg;
}


int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char* test_urls[] = {
        "https://www.example.com",     // Valid public URL
        "http://127.0.0.1/status",    // SSRF attempt (loopback IP)
        "http://localhost/admin",       // SSRF attempt (loopback name)
        "https://192.168.1.1",        // SSRF attempt (private IP)
        "file:///etc/passwd",           // SSRF attempt (disallowed scheme)
        "http://[::1]/",               // SSRF attempt (IPv6 loopback)
        "ftp://ftp.example.com",        // SSRF attempt (disallowed scheme)
        NULL
    };
    
    for (int i = 0; test_urls[i] != NULL; i++) {
        printf("Testing URL: %s\n", test_urls[i]);
        char* result = makeHttpRequest(test_urls[i]);
        if (result) {
            printf("Result: %s\n", result);
            free(result); // Free the returned string
        }
        printf("---------------------------------\n");
    }

    curl_global_cleanup();
    return 0;
}