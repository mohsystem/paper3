/*
 * This program requires the following third-party libraries:
 * 1. libcurl: For making HTTP requests.
 * 2. jansson: For parsing JSON.
 *
 * Installation (Ubuntu/Debian):
 * sudo apt-get install libcurl4-openssl-dev libjansson-dev
 *
 * Compilation command:
 * gcc your_source_file.c -o your_executable -lcurl -ljansson
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Struct to hold memory buffer for libcurl response
struct MemoryStruct {
    char* memory;
    size_t size;
};

// Callback function for libcurl to write received data into our MemoryStruct
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Error: not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/**
 * @brief Fetches content from a URL and parses it as a jansson JSON object.
 *
 * @param url_string The URL to fetch the JSON from.
 * @return A pointer to a json_t object, or NULL on failure.
 *         The caller is responsible for calling json_decref() on the returned object.
 */
json_t* fetch_json_from_url(const char* url_string) {
    if (url_string == NULL || *url_string == '\0') {
        fprintf(stderr, "Error: URL string is null or empty.\n");
        return NULL;
    }
    
    CURL* curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: Failed to initialize libcurl.\n");
        free(chunk.memory);
        return NULL;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url_string);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L);

    res = curl_easy_perform(curl_handle);

    json_t* root = NULL;
    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            json_error_t error;
            root = json_loadb(chunk.memory, chunk.size, 0, &error);
            if (!root) {
                fprintf(stderr, "Error: Jansson failed to parse JSON on line %d: %s\n", error.line, error.text);
            }
        } else {
            fprintf(stderr, "Error: HTTP GET request failed with response code: %ld\n", response_code);
        }
    } else {
        fprintf(stderr, "Error: libcurl failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    return root;
}

int main(int argc, char* argv[]) {
    // It's good practice to initialize libcurl globally.
    curl_global_init(CURL_GLOBAL_ALL);

    // Command-line argument handling
    if (argc > 1) {
        printf("Fetching JSON from command-line argument: %s\n", argv[1]);
        json_t* result = fetch_json_from_url(argv[1]);
        if (result) {
            char* json_str = json_dumps(result, JSON_INDENT(4));
            printf("Successfully fetched and parsed JSON:\n%s\n", json_str);
            free(json_str);
            json_decref(result); // Important: free the jansson object
        } else {
            printf("Failed to fetch or parse JSON.\n");
        }
        printf("\n--- Running built-in test cases ---\n\n");
    }

    const char* test_urls[] = {
        // 1. Valid JSON URL
        "https://jsonplaceholder.typicode.com/posts/1",
        // 2. URL pointing to non-JSON content (HTML)
        "http://example.com",
        // 3. URL that results in a 404 Not Found error
        "https://jsonplaceholder.typicode.com/posts/99999",
        // 4. Malformed URL
        "htp:/invalid-url",
        // 5. Non-existent domain
        "http://domain.that.does.not.exist"
    };
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d: %s ---\n", i + 1, test_urls[i]);
        json_t* result = fetch_json_from_url(test_urls[i]);
        if (result) {
            printf("Success! Fetched JSON object. Type: %d (2=Array, 5=Object)\n", json_typeof(result));
            // Free the memory allocated by jansson
            json_decref(result);
        } else {
            printf("Failure! As expected or due to an error.\n");
        }
        printf("\n");
    }

    curl_global_cleanup();
    return 0;
}