
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <ctype.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    if (mem->size + realsize > 1000000) {
        return 0;
    }
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

char* getPageTitle(const char* urlString) {
    char* result = (char*)malloc(1000);
    
    if (!urlString || strlen(urlString) == 0) {
        strcpy(result, "Invalid URL");
        return result;
    }
    
    // Validate HTTPS URL
    char lowerUrl[1000];
    strncpy(lowerUrl, urlString, 999);
    lowerUrl[999] = '\\0';
    for (int i = 0; lowerUrl[i]; i++) {
        lowerUrl[i] = tolower(lowerUrl[i]);
    }
    
    if (strncmp(lowerUrl, "https://", 8) != 0) {
        strcpy(result, "Only HTTPS URLs are allowed");
        return result;
    }
    
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        free(chunk.memory);
        curl_global_cleanup();
        strcpy(result, "Failed to initialize CURL");
        return result;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, urlString);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        snprintf(result, 999, "Error: %s", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return result;
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    // Simple title extraction
    char* titleStart = strcasestr(chunk.memory, "<title>");
    char* titleEnd = strcasestr(chunk.memory, "</title>");
    
    if (titleStart && titleEnd && titleEnd > titleStart) {
        titleStart += 7;
        int len = titleEnd - titleStart;
        if (len > 998) len = 998;
        strncpy(result, titleStart, len);
        result[len] = '\\0';
        
        // Trim whitespace
        while (len > 0 && isspace(result[len-1])) result[--len] = '\\0';
        int start = 0;
        while (result[start] && isspace(result[start])) start++;
        if (start > 0) memmove(result, result + start, len - start + 1);
    } else {
        strcpy(result, "No title found");
    }
    
    free(chunk.memory);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        char* result = getPageTitle(argv[1]);
        printf("Title: %s\\n", result);
        free(result);
    } else {
        printf("Test Case 1:\\n");
        char* r1 = getPageTitle("https://www.example.com");
        printf("%s\\n", r1);
        free(r1);
        
        printf("\\nTest Case 2:\\n");
        char* r2 = getPageTitle("https://www.google.com");
        printf("%s\\n", r2);
        free(r2);
        
        printf("\\nTest Case 3:\\n");
        char* r3 = getPageTitle("http://www.example.com");
        printf("%s\\n", r3);
        free(r3);
        
        printf("\\nTest Case 4:\\n");
        char* r4 = getPageTitle("");
        printf("%s\\n", r4);
        free(r4);
        
        printf("\\nTest Case 5:\\n");
        char* r5 = getPageTitle(NULL);
        printf("%s\\n", r5);
        free(r5);
    }
    
    return 0;
}
