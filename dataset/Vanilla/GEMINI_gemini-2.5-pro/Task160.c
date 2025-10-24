// To compile and run this code, you need to have libcurl installed.
// On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
// On Fedora/CentOS: sudo dnf install libcurl-devel
//
// Compile command: gcc your_file_name.c -o your_executable_name -lcurl
// Then run: ./your_executable_name

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// A struct to hold the dynamically allocated memory for the response.
struct MemoryStruct {
  char *memory;
  size_t size;
};

/**
 * @brief Callback function for libcurl to write received data into our MemoryStruct.
 *        It dynamically resizes the memory buffer to hold the incoming data.
 */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Error: not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/**
 * @brief Makes an HTTP GET request using libcurl.
 * @param url The URL to request.
 * @return A dynamically allocated string with the response.
 *         The caller is responsible for freeing this memory.
 *         Returns a dynamically allocated error message on failure.
 */
char* makeHttpRequest(const char *url) {
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1); 
    chunk.size = 0;

    curl_handle = curl_easy_init();
    if(curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-c-agent/1.0");
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L);

        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            const char* error_str = curl_easy_strerror(res);
            size_t len = strlen("Error: ") + strlen(error_str) + 1;
            char* error_buffer = malloc(len);
            if (error_buffer) {
                snprintf(error_buffer, len, "Error: %s", error_str);
            }
            free(chunk.memory);
            curl_easy_cleanup(curl_handle);
            return error_buffer; // Caller must free this
        }
        
        curl_easy_cleanup(curl_handle);
        return chunk.memory;
    }

    free(chunk.memory);
    return strdup("Error: Could not initialize cURL."); // Caller must free this
}

int main(void) {
    const char *testUrls[] = {
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://httpbin.org/get",
        "https://api.ipify.org?format=json",
        "https://www.google.com",
        "http://thissitedoesnotexist.invalidtld/"
    };
    int num_urls = sizeof(testUrls) / sizeof(testUrls[0]);
    
    curl_global_init(CURL_GLOBAL_ALL);

    for (int i = 0; i < num_urls; ++i) {
        printf("Testing URL: %s\n", testUrls[i]);
        char* result = makeHttpRequest(testUrls[i]);

        if (result) {
            if (strlen(result) > 300) {
                printf("Result (first 300 chars):\n%.300s...\n\n", result);
            } else {
                printf("Result:\n%s\n\n", result);
            }
            free(result); // Free the memory allocated by makeHttpRequest
        } else {
            printf("Result: NULL (An error occurred)\n\n");
        }
        printf("----------------------------------------\n");
    }
    
    curl_global_cleanup();
    return 0;
}