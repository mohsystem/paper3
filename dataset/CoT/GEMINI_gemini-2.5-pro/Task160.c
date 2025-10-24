#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// Required for libcurl
#include <curl/curl.h>
// Required for network address resolution and checking
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// Note: This code requires libcurl. Compile with:
// gcc your_file.c -o your_program -lcurl

const long CONNECT_TIMEOUT = 5L; // 5 seconds
const long TOTAL_TIMEOUT = 10L;  // 10 seconds
const size_t MAX_RESPONSE_SIZE = 4096; // 4KB limit

// Struct to hold response data from libcurl
struct MemoryStruct {
    char* memory;
    size_t size;
};

// Callback function for libcurl to write received data into our struct
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    size_t new_size = mem->size + realsize;
    if (new_size > MAX_RESPONSE_SIZE) {
        realsize = MAX_RESPONSE_SIZE - mem->size;
        if (realsize == 0) return 0; // Prevent further writing if limit reached
        new_size = MAX_RESPONSE_SIZE;
    }

    char* ptr = (char*)realloc(mem->memory, new_size + 1);
    if (!ptr) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size = new_size;
    mem->memory[mem->size] = 0;

    return size * nmemb; // Return original size to avoid curl error
}

// Function to check if an IP address is in a private or loopback range
bool is_private_or_loopback_ip(const char* ip_str) {
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

char* make_http_request(const char* url_string) {
    // 1. Security Check: Protocol validation
    if (strncmp(url_string, "http://", 7) != 0 && strncmp(url_string, "https://", 8) != 0) {
        return strdup("Error: Invalid protocol. Only HTTP and HTTPS are allowed.");
    }

    // 2. Security Check: SSRF Prevention via DNS resolution
    const char* host_start = strstr(url_string, "://");
    if (!host_start) return strdup("Error: Could not parse URL.");
    host_start += 3;

    const char* host_end = strchr(host_start, '/');
    size_t host_len = (host_end) ? (host_end - host_start) : strlen(host_start);
    
    char* hostname = (char*)malloc(host_len + 1);
    if (!hostname) return strdup("Error: Memory allocation failed.");
    strncpy(hostname, host_start, host_len);
    hostname[host_len] = '\0';
    
    // Remove port if present
    char* port_pos = strchr(hostname, ':');
    if (port_pos) *port_pos = '\0';

    struct addrinfo hints, *addrs;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname, NULL, &hints, &addrs) != 0) {
        free(hostname);
        return strdup("Error: Host could not be resolved.");
    }
    free(hostname);

    bool is_unsafe = false;
    for (struct addrinfo* rp = addrs; rp != NULL; rp = rp->ai_next) {
        char ip_str[INET6_ADDRSTRLEN];
        void* addr;
        if (rp->ai_family == AF_INET) {
            addr = &((struct sockaddr_in*)rp->ai_addr)->sin_addr;
        } else {
            addr = &((struct sockaddr_in6*)rp->ai_addr)->sin6_addr;
        }
        inet_ntop(rp->ai_family, addr, ip_str, sizeof(ip_str));
        if (is_private_or_loopback_ip(ip_str)) {
            is_unsafe = true;
            break;
        }
    }
    freeaddrinfo(addrs);
    if (is_unsafe) {
        return strdup("Error: SSRF attempt detected. Requests to local/private networks are not allowed.");
    }

    // 3. Make the Connection using libcurl
    CURL* curl = curl_easy_init();
    if (!curl) return strdup("Error: Failed to initialize libcurl.");

    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1); // will be grown by realloc
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TOTAL_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // Disable redirects

    CURLcode res = curl_easy_perform(curl);
    
    char* final_result;
    if (res != CURLE_OK) {
        size_t len = snprintf(NULL, 0, "Error: curl_easy_perform() failed: %s", curl_easy_strerror(res));
        final_result = (char*)malloc(len + 1);
        snprintf(final_result, len + 1, "Error: curl_easy_perform() failed: %s", curl_easy_strerror(res));
    } else {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        size_t len = snprintf(NULL, 0, "Status Code: %ld\nResponse Body (first %zu bytes):\n%s", response_code, MAX_RESPONSE_SIZE, chunk.memory);
        final_result = (char*)malloc(len + 1);
        snprintf(final_result, len + 1, "Status Code: %ld\nResponse Body (first %zu bytes):\n%s", response_code, MAX_RESPONSE_SIZE, chunk.memory);
    }
    
    free(chunk.memory);
    curl_easy_cleanup(curl);
    return final_result;
}

int main() {
    const char* test_urls[] = {
        "https://api.ipify.org", // Valid public URL
        "http://google.com",       // Valid public URL that redirects
        "http://127.0.0.1",        // SSRF test (loopback)
        "http://192.168.1.1",      // SSRF test (private network)
        "https://thissitedoesnotexist.invalidtld", // Invalid host
        "file:///etc/passwd",      // Invalid protocol test
        NULL
    };

    curl_global_init(CURL_GLOBAL_ALL);

    for (int i = 0; test_urls[i] != NULL; ++i) {
        printf("=========================================\n");
        printf("Requesting URL: %s\n", test_urls[i]);
        printf("-----------------------------------------\n");
        char* response = make_http_request(test_urls[i]);
        printf("%s\n", response);
        free(response);
        printf("=========================================\n\n");
    }

    curl_global_cleanup();
    return 0;
}