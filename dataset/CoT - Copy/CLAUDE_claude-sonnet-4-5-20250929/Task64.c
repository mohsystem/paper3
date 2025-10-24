
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>

#define MAX_SIZE (10 * 1024 * 1024)
#define TIMEOUT 10L
#define MAX_URL_LENGTH 2048

typedef struct {
    char* data;
    size_t size;
} ResponseBuffer;

// Callback function for CURL
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    ResponseBuffer* buffer = (ResponseBuffer*)userp;
    
    if (buffer->size + realsize > MAX_SIZE) {
        return 0;
    }
    
    char* ptr = realloc(buffer->data, buffer->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    buffer->data = ptr;
    memcpy(&(buffer->data[buffer->size]), contents, realsize);
    buffer->size += realsize;
    buffer->data[buffer->size] = 0;
    
    return realsize;
}

void to_lower(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int is_private_address(const char* host) {
    char hostLower[256];
    strncpy(hostLower, host, sizeof(hostLower) - 1);
    hostLower[sizeof(hostLower) - 1] = '\\0';
    to_lower(hostLower);
    
    return (strcmp(hostLower, "localhost") == 0 ||
            strcmp(hostLower, "127.0.0.1") == 0 ||
            strcmp(hostLower, "0.0.0.0") == 0 ||
            strncmp(hostLower, "192.168.", 8) == 0 ||
            strncmp(hostLower, "10.", 3) == 0 ||
            strncmp(hostLower, "172.16.", 7) == 0);
}

char* fetch_and_parse_json(const char* url_string) {
    if (!url_string || strlen(url_string) == 0) {
        fprintf(stderr, "Error: URL cannot be empty\\n");
        return NULL;
    }
    
    if (strlen(url_string) > MAX_URL_LENGTH) {
        fprintf(stderr, "Error: URL too long\\n");
        return NULL;
    }
    
    // Protocol validation
    char urlLower[MAX_URL_LENGTH];
    strncpy(urlLower, url_string, sizeof(urlLower) - 1);
    urlLower[sizeof(urlLower) - 1] = '\\0';
    to_lower(urlLower);
    
    if (strncmp(urlLower, "http://", 7) != 0 && strncmp(urlLower, "https://", 8) != 0) {
        fprintf(stderr, "Error: Only HTTP and HTTPS protocols are allowed\\n");
        return NULL;
    }
    
    // Extract hostname
    const char* hostStart = strstr(url_string, "://");
    if (hostStart) {
        hostStart += 3;
        char host[256] = {0};
        const char* hostEnd = strchr(hostStart, '/');
        const char* portPos = strchr(hostStart, ':');
        
        size_t hostLen;
        if (portPos && (!hostEnd || portPos < hostEnd)) {
            hostLen = portPos - hostStart;
        } else if (hostEnd) {
            hostLen = hostEnd - hostStart;
        } else {
            hostLen = strlen(hostStart);
        }
        
        if (hostLen >= sizeof(host)) {
            hostLen = sizeof(host) - 1;
        }
        
        strncpy(host, hostStart, hostLen);
        host[hostLen] = '\\0';
        
        if (is_private_address(host)) {
            fprintf(stderr, "Error: Access to private network addresses is not allowed\\n");
            return NULL;
        }
    }
    
    CURL* curl;
    CURLcode res;
    ResponseBuffer buffer = {NULL, 0};
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL\\n");
        curl_global_cleanup();
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureJSONFetcher/1.0");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: CURL error: %s\\n", curl_easy_strerror(res));
        free(buffer.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    if (response_code != 200) {
        fprintf(stderr, "Error: HTTP error code: %ld\\n", response_code);
        free(buffer.data);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    return buffer.data;
}

int main(int argc, char* argv[]) {
    const char* test_urls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://api.github.com/users/github",
        "https://jsonplaceholder.typicode.com/users/1",
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://jsonplaceholder.typicode.com/comments/1"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("\\nTest Case %d:\\n", i + 1);
        printf("URL: %s\\n", test_urls[i]);
        
        char* result = fetch_and_parse_json(test_urls[i]);
        if (result) {
            printf("Result: %s\\n", result);
            free(result);
        } else {
            printf("Failed to fetch or parse JSON\\n");
        }
    }
    
    return 0;
}
