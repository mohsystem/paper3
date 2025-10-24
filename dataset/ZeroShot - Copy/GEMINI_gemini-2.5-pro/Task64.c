/*
 * IMPORTANT: This code requires third-party libraries for HTTP requests and JSON parsing.
 * - libcurl: For handling HTTP/HTTPS requests.
 * - cJSON: For parsing and handling JSON data.
 *
 * To compile this code, you need to have libcurl and cJSON development libraries installed.
 *
 * Example compilation command on Linux (with libraries installed via package manager):
 * gcc your_file_name.c -o your_executable -lcurl -lcjson -lm
 *
 * You must have cJSON.h in your include path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

const long CONNECT_TIMEOUT = 5L; // 5 seconds
const long REQUEST_TIMEOUT = 10L; // 10 seconds
const size_t MAX_RESPONSE_SIZE = 1024 * 1024; // 1 MB

// A struct to hold the state for our curl write callback
typedef struct {
    char *buffer;
    size_t size;
} MemoryStruct;

// Callback function for libcurl to write received data into our MemoryStruct
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    // 3. Security: Check if the new data will exceed the max size
    if (mem->size + realsize > MAX_RESPONSE_SIZE) {
        fprintf(stderr, "Error: Response size exceeds the limit of %zu bytes.\n", MAX_RESPONSE_SIZE);
        return 0; // Returning 0 signals an error to libcurl
    }

    char *ptr = realloc(mem->buffer, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Error: not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->buffer = ptr;
    memcpy(&(mem->buffer[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->buffer[mem->size] = 0;

    return realsize;
}

cJSON* fetchAndParseJson(const char* url_string) {
    // 1. Security: Validate URL protocol to prevent SSRF
    if (strncmp(url_string, "http://", 7) != 0 && strncmp(url_string, "https://", 8) != 0) {
        fprintf(stderr, "Error: Invalid protocol. Only HTTP and HTTPS are allowed.\n");
        return NULL;
    }

    CURL *curl_handle;
    MemoryStruct chunk = { .buffer = NULL, .size = 0 };
    chunk.buffer = malloc(1); // will be grown by realloc
    if (!chunk.buffer) {
        fprintf(stderr, "Error: Failed to allocate memory.\n");
        return NULL;
    }
    
    cJSON *json = NULL;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: Failed to initialize libcurl.\n");
        free(chunk.buffer);
        curl_global_cleanup();
        return NULL;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url_string);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // 2. Security: Set timeouts to prevent DoS attacks
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, REQUEST_TIMEOUT);

    CURLcode res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            fprintf(stderr, "HTTP request failed with response code: %ld\n", http_code);
        } else {
            // 4. Parse JSON
            json = cJSON_Parse(chunk.buffer);
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if (error_ptr != NULL) {
                    fprintf(stderr, "JSON Parse Error before: %s\n", error_ptr);
                }
            }
        }
    }

    // Cleanup
    curl_easy_cleanup(curl_handle);
    free(chunk.buffer);
    curl_global_cleanup();

    return json;
}

int main(void) {
    const char* testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",      // Test 1: Success, single object
        "https://jsonplaceholder.typicode.com/posts",         // Test 2: Success, array of objects
        "https://jsonplaceholder.typicode.com/posts/999999",  // Test 3: Client Error (404)
        "https://nonexistent-domain-for-testing123.com",     // Test 4: Network Error
        "https://www.google.com",                             // Test 5: JSON Parse Error
        "file:///etc/hosts"                                   // Test 6: Security Error (Invalid Protocol)
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d: %s ---\n", (i + 1), testUrls[i]);
        cJSON *json_obj = fetchAndParseJson(testUrls[i]);
        if (json_obj != NULL) {
            printf("Success! Parsed JSON object (first 200 chars):\n");
            char* json_string = cJSON_Print(json_obj);
            if (json_string != NULL) {
                printf("%.200s%s\n", json_string, strlen(json_string) > 200 ? "..." : "");
                free(json_string);
            }
            cJSON_Delete(json_obj);
        } else {
            fprintf(stderr, "Failed to fetch or parse JSON.\n");
        }
        printf("\n");
    }

    return 0;
}