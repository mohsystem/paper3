
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>
#include <ctype.h>

#define TIMEOUT 10
#define MAX_CONTENT_LENGTH (5 * 1024 * 1024)

typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    if (mem->size + realsize > MAX_CONTENT_LENGTH) {
        return 0;
    }
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

int is_valid_https_url(const char* url) {
    if (!url || strncmp(url, "https://", 8) != 0) {
        return 0;
    }
    
    const char* host_start = url + 8;
    const char* host_end = strchr(host_start, '/');
    
    if (!host_end) {
        host_end = url + strlen(url);
    }
    
    size_t host_len = host_end - host_start;
    char host[256] = {0};
    
    if (host_len >= sizeof(host)) {
        return 0;
    }
    
    strncpy(host, host_start, host_len);
    
    char* port_pos = strchr(host, ':');
    if (port_pos) {
        *port_pos = '\\0';
    }
    
    if (strcmp(host, "localhost") == 0 || strcmp(host, "127.0.0.1") == 0 ||
        strcmp(host, "::1") == 0 || strncmp(host, "192.168.", 8) == 0 ||
        strncmp(host, "10.", 3) == 0 || strncmp(host, "172.", 4) == 0) {
        return 0;
    }
    
    return 1;
}

char* extract_title_from_html(const char* html) {
    if (!html) {
        return strdup("No title found");
    }
    
    regex_t regex;
    regmatch_t matches[2];
    
    if (regcomp(&regex, "<title[^>]*>[ \\t\\n\\r]*([^<]+)[ \\t\\n\\r]*</title>", REG_ICASE | REG_EXTENDED) != 0) {
        return strdup("No title found");
    }
    
    if (regexec(&regex, html, 2, matches, 0) == 0) {
        size_t len = matches[1].rm_eo - matches[1].rm_so;
        char* title = malloc(len + 1);
        
        if (title) {
            strncpy(title, html + matches[1].rm_so, len);
            title[len] = '\\0';
            
            char* p = title;
            while (*p) {
                if (*p == '\\n' || *p == '\\r') {
                    *p = ' ';
                }
                p++;
            }
            
            regfree(&regex);
            return title;
        }
    }
    
    regfree(&regex);
    return strdup("No title found");
}

char* extract_page_title(const char* url_string) {
    if (!url_string || strlen(url_string) == 0) {
        return strdup("Error: URL cannot be empty");
    }
    
    if (!is_valid_https_url(url_string)) {
        return strdup("Error: Invalid URL or non-HTTPS URL provided");
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return strdup("Error: Failed to initialize CURL");
    }
    
    MemoryStruct chunk = {0};
    chunk.data = malloc(1);
    chunk.size = 0;
    
    curl_easy_setopt(curl, CURLOPT_URL, url_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecurePageTitleExtractor/1.0");
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        char error[256];
        snprintf(error, sizeof(error), "Error: %s", curl_easy_strerror(res));
        free(chunk.data);
        curl_easy_cleanup(curl);
        return strdup(error);
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);
    
    if (http_code != 200) {
        char error[256];
        snprintf(error, sizeof(error), "Error: HTTP %ld", http_code);
        free(chunk.data);
        return strdup(error);
    }
    
    char* title = extract_title_from_html(chunk.data);
    free(chunk.data);
    
    return title;
}

int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    if (argc < 2) {
        printf("Usage: %s <https-url>\\n", argv[0]);
        printf("\\nTest cases:\\n");
        
        const char* test_urls[] = {
            "https://www.example.com",
            "https://www.google.com",
            "http://www.example.com",
            "https://localhost",
            "invalid-url"
        };
        
        for (int i = 0; i < 5; i++) {
            printf("\\nURL: %s\\n", test_urls[i]);
            char* result = extract_page_title(test_urls[i]);
            printf("Title: %s\\n", result);
            free(result);
        }
    } else {
        char* result = extract_page_title(argv[1]);
        printf("%s\\n", result);
        free(result);
    }
    
    curl_global_cleanup();
    return 0;
}
