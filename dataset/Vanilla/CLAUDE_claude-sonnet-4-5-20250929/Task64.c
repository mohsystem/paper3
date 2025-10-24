
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Not enough memory\\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

char* readJsonFromUrl(const char* url) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK) {
            fprintf(stderr, "Error: %s\\n", curl_easy_strerror(res));
            free(chunk.memory);
            return NULL;
        }
    }
    
    curl_global_cleanup();
    return chunk.memory;
}

char* parseJson(const char* url) {
    char* jsonContent = readJsonFromUrl(url);
    return jsonContent;
}

int main(int argc, char* argv[]) {
    const char* testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://jsonplaceholder.typicode.com/users/1",
        "https://jsonplaceholder.typicode.com/comments/1",
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://jsonplaceholder.typicode.com/albums/1"
    };
    
    for(int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        printf("URL: %s\\n", testUrls[i]);
        char* result = parseJson(testUrls[i]);
        if(result != NULL) {
            printf("JSON Content: %s\\n", result);
            free(result);
        }
        printf("\\n");
    }
    
    return 0;
}
