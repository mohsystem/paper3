
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>

typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

char* getPageTitle(const char* urlString) {
    CURL* curl;
    CURLcode res;
    MemoryStruct chunk;
    chunk.data = malloc(1);
    chunk.size = 0;
    
    static char result[1024];
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, urlString);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            snprintf(result, sizeof(result), "Error: %s", curl_easy_strerror(res));
            free(chunk.data);
            return result;
        }
        
        regex_t regex;
        regmatch_t matches[2];
        
        if (regcomp(&regex, "<title>([^<]*)</title>", REG_EXTENDED | REG_ICASE) == 0) {
            if (regexec(&regex, chunk.data, 2, matches, 0) == 0) {
                int len = matches[1].rm_eo - matches[1].rm_so;
                if (len < sizeof(result)) {
                    strncpy(result, chunk.data + matches[1].rm_so, len);
                    result[len] = '\\0';
                    regfree(&regex);
                    free(chunk.data);
                    return result;
                }
            }
            regfree(&regex);
        }
        
        free(chunk.data);
        strcpy(result, "No title found");
        return result;
    }
    
    strcpy(result, "Error: Could not initialize CURL");
    return result;
}

int main(int argc, char* argv[]) {
    char* testUrls[] = {
        "https://www.example.com",
        "https://www.google.com",
        "https://www.github.com",
        "https://www.stackoverflow.com",
        "https://www.wikipedia.org"
    };
    
    if (argc > 1) {
        printf("URL: %s\\n", argv[1]);
        printf("Title: %s\\n", getPageTitle(argv[1]));
    } else {
        printf("Running test cases:\\n");
        for (int i = 0; i < 5; i++) {
            printf("\\nURL: %s\\n", testUrls[i]);
            printf("Title: %s\\n", getPageTitle(testUrls[i]));
        }
    }
    
    return 0;
}
