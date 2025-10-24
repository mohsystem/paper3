// Note: This code requires two third-party libraries:
// 1. libcurl: For making HTTP requests.
// 2. cJSON: For parsing JSON.
//
// On Debian/Ubuntu, install them with:
// sudo apt-get install libcurl4-openssl-dev libcjson-dev
//
// Compilation command:
// gcc your_file_name.c -o Task64 -lcurl -lcjson

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

// A structure to hold the data received from libcurl
struct MemoryStruct {
  char *memory;
  size_t size;
};

// Callback function for libcurl to append data to our MemoryStruct
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
 * Fetches content from a URL, parses it as JSON, and returns the object.
 * @param url The URL to fetch JSON from.
 * @return A cJSON object pointer, or NULL on failure. The caller is
 *         responsible for freeing this object using cJSON_Delete().
 */
cJSON* readJsonFromUrl(const char* url) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    cJSON *json = NULL;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    
    if(curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L); // 5 second timeout

        res = curl_easy_perform(curl_handle);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code >= 200 && http_code < 300) {
                json = cJSON_Parse(chunk.memory);
                if (json == NULL) {
                    const char *error_ptr = cJSON_GetErrorPtr();
                    if (error_ptr != NULL) {
                        fprintf(stderr, "JSON Parse Error before: %s\n", error_ptr);
                    }
                }
            } else {
                fprintf(stderr, "HTTP request failed with code %ld\n", http_code);
            }
        }
        curl_easy_cleanup(curl_handle);
    }
    
    free(chunk.memory);
    curl_global_cleanup();

    return json;
}


int main(void) {
    const char* testUrls[] = {
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://jsonplaceholder.typicode.com/users/1",
        "https://jsonplaceholder.typicode.com/posts", // This is a JSON array
        "https://jsonplaceholder.typicode.com/nonexistent" // This will cause a 404 error
    };
    int num_urls = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_urls; ++i) {
        printf("Fetching from: %s\n", testUrls[i]);
        cJSON* jsonData = readJsonFromUrl(testUrls[i]);

        if (jsonData != NULL) {
            char* jsonString = cJSON_Print(jsonData);
            if (jsonString != NULL) {
                printf("%s\n", jsonString);
                free(jsonString);
            }
            cJSON_Delete(jsonData);
        } else {
            printf("Failed to retrieve or parse JSON.\n");
        }
        printf("----------------------------------------\n");
    }

    return 0;
}