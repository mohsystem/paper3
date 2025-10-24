#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Define a maximum response size to prevent excessive memory usage.
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024) // 10 MB

// Struct to hold memory buffer for libcurl's write callback
typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

// Callback function for libcurl to write received data into a memory buffer
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    // Check for integer overflow
    if (size > 0 && nmemb > (size_t)-1 / size) {
        fprintf(stderr, "Error: Integer overflow in callback size calculation.\n");
        return 0; // Returning 0 will cause libcurl to abort
    }

    // Check against max size limit
    if (mem->size + realsize > MAX_RESPONSE_SIZE) {
        fprintf(stderr, "Error: Exceeded maximum response size of %d bytes.\n", MAX_RESPONSE_SIZE);
        return 0;
    }

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Error: Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate the buffer

    return realsize;
}

/**
 * @brief Fetches content from a URL and parses it as a JSON object.
 *
 * This function performs an HTTPS GET request to the specified URL. It uses libcurl
 * for the network request with strict SSL/TLS verification. The response body is
 * then parsed using the jansson library.
 *
 * @param url The URL to fetch the JSON data from.
 * @return A pointer to a json_t object on success, or NULL on failure. The caller
 *         is responsible for decrementing the reference count of the returned object
 *         using json_decref().
 */
json_t *fetch_and_parse_json(const char *url) {
    CURL *curl_handle = NULL;
    MemoryStruct chunk;
    json_t *root = NULL;
    
    // Initialize the memory chunk
    chunk.memory = malloc(1); // Will be grown by realloc
    if (chunk.memory == NULL) {
        fprintf(stderr, "Error: malloc failed to allocate initial memory.\n");
        return NULL;
    }
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: curl_easy_init() failed.\n");
        free(chunk.memory);
        curl_global_cleanup();
        return NULL;
    }

    // Set curl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Enforce security best practices for TLS
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L); // Verify the peer's certificate
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L); // Verify the host name in the certificate
    curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2); // Use TLSv1.2 or later
    
    // Set a reasonable timeout
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15L);
    
    // Allow following redirects
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // Perform the request
    CURLcode res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    // Check HTTP response code to ensure we got a successful response
    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        fprintf(stderr, "Error: Server returned HTTP code %ld\n", http_code);
        goto cleanup;
    }

    // Parse the JSON data
    json_error_t error;
    root = json_loadb(chunk.memory, chunk.size, 0, &error);
    if (!root) {
        fprintf(stderr, "JSON error on line %d: %s\n", error.line, error.text);
        // root is already NULL
    }

cleanup:
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
    
    return root; // Will be NULL on failure
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        
        printf("\n--- Running Built-in Test Cases ---\n");
        const char *test_urls[] = {
            // Test Case 1: Valid JSON object from a known API
            "https://jsonplaceholder.typicode.com/posts/1",
            // Test Case 2: Valid JSON array from a known API
            "https://api.github.com/users/octocat/repos",
            // Test Case 3: A URL that returns HTML (should fail parsing)
            "https://www.google.com",
            // Test Case 4: A non-existent URL (should fail with HTTP 404)
            "https://jsonplaceholder.typicode.com/posts/99999999",
            // Test Case 5: An invalid URL format (should fail curl perform)
            "htp:/invalid-url-format.com"
        };
        int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

        for (int i = 0; i < num_tests; ++i) {
            printf("\n--- Test Case %d: Fetching %s ---\n", i + 1, test_urls[i]);
            json_t *json_result = fetch_and_parse_json(test_urls[i]);
            if (json_result) {
                char *json_string = json_dumps(json_result, JSON_INDENT(2));
                if (json_string) {
                    printf("Successfully fetched and parsed JSON:\n%s\n", json_string);
                    free(json_string);
                }
                json_decref(json_result);
            } else {
                printf("Failed to fetch or parse JSON.\n");
            }
            printf("--- End Test Case %d ---\n", i + 1);
        }
        return 1;
    }
    
    const char *url = argv[1];
    printf("Fetching from command line URL: %s\n", url);
    json_t *json_result = fetch_and_parse_json(url);

    if (json_result) {
        char *json_string = json_dumps(json_result, JSON_INDENT(2));
        if (json_string) {
            printf("Successfully fetched and parsed JSON:\n%s\n", json_string);
            free(json_string);
        } else {
            fprintf(stderr, "Failed to dump JSON to string.\n");
        }
        json_decref(json_result);
        return 0;
    } else {
        fprintf(stderr, "Failed to fetch or parse JSON from URL.\n");
        return 1;
    }
}