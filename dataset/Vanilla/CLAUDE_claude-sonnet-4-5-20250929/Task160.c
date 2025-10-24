
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

// Structure to hold response data
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function to write received data
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("Not enough memory\\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

char* makeHttpRequest(const char* urlString) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    long responseCode;
    static char result[10000];
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, urlString);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            snprintf(result, sizeof(result), "Error: %s", curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            snprintf(result, sizeof(result), "Response Code: %ld\\n%s", responseCode, chunk.memory);
        }
        
        curl_easy_cleanup(curl);
    } else {
        snprintf(result, sizeof(result), "Error: Failed to initialize CURL");
    }
    
    free(chunk.memory);
    curl_global_cleanup();
    
    return result;
}

int main() {
    // Test cases
    const char* testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://api.github.com/users/github",
        "https://httpbin.org/get",
        "https://www.google.com",
        "https://example.com"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d: %s\\n", i + 1, testUrls[i]);
        char* result = makeHttpRequest(testUrls[i]);
        printf("%.200s...\\n\\n", result);
    }
    
    return 0;
}
