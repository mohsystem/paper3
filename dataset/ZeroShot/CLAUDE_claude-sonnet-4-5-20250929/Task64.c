
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Note: C does not have built-in JSON parsing. 
// You would need to use a library like cJSON or json-c
// This implementation shows the HTTP fetching part only

struct MemoryStruct {
    char *memory;
    size_t size;
};

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

char* fetchAndParseJSON(const char* url_string) {
    if (url_string == NULL || strlen(url_string) == 0) {
        fprintf(stderr, "URL cannot be NULL or empty\\n");
        return NULL;
    }
    
    // Validate protocol
    if (strncmp(url_string, "http://", 7) != 0 && strncmp(url_string, "https://", 8) != 0) {
        fprintf(stderr, "Only HTTP and HTTPS protocols are allowed\\n");
        return NULL;
    }
    
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\\n");
        free(chunk.memory);
        return NULL;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        free(chunk.memory);
        return NULL;
    }
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    if (response_code != 200) {
        fprintf(stderr, "HTTP Error: %ld\\n", response_code);
        free(chunk.memory);
        return NULL;
    }
    
    // Note: In a real implementation, you would parse the JSON here using a library like cJSON
    // For now, we just return the raw JSON string
    return chunk.memory;
}

int main(int argc, char *argv[]) {
    printf("Test Case 1: Valid JSON URL\\n");
    char *result1 = fetchAndParseJSON("https://jsonplaceholder.typicode.com/todos/1");
    if (result1 != NULL) {
        printf("Result: %s\\n", result1);
        free(result1);
    }
    
    printf("\\nTest Case 2: Another valid JSON URL\\n");
    char *result2 = fetchAndParseJSON("https://jsonplaceholder.typicode.com/users/1");
    if (result2 != NULL) {
        printf("Result: %s\\n", result2);
        free(result2);
    }
    
    printf("\\nTest Case 3: Invalid URL format\\n");
    char *result3 = fetchAndParseJSON("not-a-valid-url");
    if (result3 != NULL) {
        free(result3);
    }
    
    printf("\\nTest Case 4: Empty URL\\n");
    char *result4 = fetchAndParseJSON("");
    if (result4 != NULL) {
        free(result4);
    }
    
    printf("\\nTest Case 5: NULL URL\\n");
    char *result5 = fetchAndParseJSON(NULL);
    if (result5 != NULL) {
        free(result5);
    }
    
    // Command line argument support
    if (argc > 1) {
        printf("\\nCommand line URL: %s\\n", argv[1]);
        char *result = fetchAndParseJSON(argv[1]);
        if (result != NULL) {
            printf("Result: %s\\n", result);
            free(result);
        }
    }
    
    return 0;
}
