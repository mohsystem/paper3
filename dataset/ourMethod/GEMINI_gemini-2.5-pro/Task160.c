#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <limits.h>

// Constants for security and resource limits
#define MAX_URL_LENGTH 2048
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024) // 10 MB
#define CONNECT_TIMEOUT 10L // seconds
#define REQUEST_TIMEOUT 20L // seconds
#define MAX_REDIRECTS 5L

// Struct to hold response data in memory
typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

// Function prototypes
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
static int is_valid_url_scheme(const char *url);
MemoryStruct* make_http_request(const char *url);
void free_memory_struct(MemoryStruct *mem);

/**
 * @brief Callback function for libcurl to write received data into a memory buffer.
 * This function is designed to be secure by preventing buffer overflows and excessive memory allocation.
 *
 * @param contents Pointer to the data received.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param userp User pointer, expected to be a MemoryStruct*.
 * @return The number of bytes successfully handled. If this differs from size*nmemb,
 *         it will signal an error to libcurl.
 */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Check for multiplication overflow on size * nmemb
    if (nmemb > 0 && size > SIZE_MAX / nmemb) {
        fprintf(stderr, "Error: multiplication overflow in callback\n");
        return 0; // Signal error to curl
    }
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    // Check if the new data will exceed the maximum allowed response size.
    if (realsize > MAX_RESPONSE_SIZE - mem->size) {
        fprintf(stderr, "Error: Response exceeds maximum size of %d bytes\n", MAX_RESPONSE_SIZE);
        return 0; // Signal error to curl
    }
    
    char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Error: realloc() failed, out of memory\n");
        return 0; // Signal error to curl
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate the buffer

    return realsize;
}

/**
 * @brief Performs a basic validation on the URL scheme.
 *
 * @param url The URL string to validate.
 * @return 1 if URL starts with "http://" or "https://", 0 otherwise.
 */
static int is_valid_url_scheme(const char *url) {
    if (url == NULL) {
        return 0;
    }
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        return 1;
    }
    return 0;
}


/**
 * @brief Frees the memory allocated for a MemoryStruct, including its internal buffer.
 *
 * @param mem Pointer to the MemoryStruct to be freed.
 */
void free_memory_struct(MemoryStruct *mem) {
    if (mem != NULL) {
        free(mem->memory);
        mem->memory = NULL;
        free(mem);
    }
}


/**
 * @brief Makes an HTTP GET request to the given URL and returns the response.
 *
 * @param url The URL to fetch.
 * @return A pointer to a newly allocated MemoryStruct containing the response data,
 *         or NULL on failure. The caller is responsible for freeing the returned struct
 *         using free_memory_struct().
 */
MemoryStruct* make_http_request(const char *url) {
    // Input validation
    if (url == NULL) {
        fprintf(stderr, "Error: URL is NULL.\n");
        return NULL;
    }
    if (strlen(url) > MAX_URL_LENGTH) {
        fprintf(stderr, "Error: URL length exceeds maximum of %d characters.\n", MAX_URL_LENGTH);
        return NULL;
    }
    if (!is_valid_url_scheme(url)) {
        fprintf(stderr, "Error: Invalid URL scheme. Only http:// and https:// are supported.\n");
        return NULL;
    }

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: Failed to initialize curl handle.\n");
        return NULL;
    }

    MemoryStruct *chunk = (MemoryStruct *)malloc(sizeof(MemoryStruct));
    if (chunk == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for chunk struct.\n");
        curl_easy_cleanup(curl_handle);
        return NULL;
    }
    chunk->memory = (char *)malloc(1); // Will be grown by realloc in callback
    if (chunk->memory == NULL) {
        fprintf(stderr, "Error: Failed to allocate initial memory for response.\n");
        free(chunk);
        curl_easy_cleanup(curl_handle);
        return NULL;
    }
    chunk->size = 0;
    chunk->memory[0] = '\0';


    CURLcode res;

    // Set curl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // Security options
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

    // Follow redirects
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
    
    // Timeouts
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, REQUEST_TIMEOUT);
    
    // Perform the request
    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free_memory_struct(chunk);
        chunk = NULL;
    }

    curl_easy_cleanup(curl_handle);
    return chunk;
}

int main(void) {
    // curl_global_init() should be called once per program
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return 1;
    }

    const char *test_urls[] = {
        "https://www.example.com",                           // 1. Valid HTTPS URL
        "http://httpbin.org/get",                            // 2. Valid HTTP URL
        "http://google.com",                                 // 3. URL that redirects
        "https://thissitedoesnotexist.invalid",              // 4. Non-existent domain
        "ftp://example.com"                                  // 5. Invalid protocol scheme
    };
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("----------------------------------------\n");
        printf("Testing URL: %s\n", test_urls[i]);
        printf("----------------------------------------\n");

        MemoryStruct *response = make_http_request(test_urls[i]);

        if (response) {
            printf("Request successful. Received %zu bytes.\n", response->size);
            // Print first 400 characters of the response to avoid flooding the console
            printf("Response body (first 400 chars):\n%.400s\n\n", response->memory);
            free_memory_struct(response);
        } else {
            printf("Request failed.\n\n");
        }
    }

    // curl_global_cleanup() should be called once per program
    curl_global_cleanup();

    return 0;
}