#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Define a maximum response size to prevent excessive memory usage (5MB)
#define MAX_RESPONSE_SIZE (5 * 1024 * 1024)

/**
 * @brief A structure to hold the dynamically growing memory buffer for the HTTP response.
 */
struct MemoryStruct {
    char *memory;
    size_t size;
};

/**
 * @brief Callback function for libcurl to write received data into a MemoryStruct.
 *
 * This function is called by libcurl as soon as there is data received that needs to be saved.
 * It reallocates the memory buffer to fit the new data and copies the data into the buffer.
 * It also checks against MAX_RESPONSE_SIZE to prevent DoS attacks via memory exhaustion.
 *
 * @param contents Pointer to the received data.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param userp User pointer, pointing to a MemoryStruct.
 * @return The number of bytes successfully handled. If it differs from size * nmemb,
 *         it will signal an error to libcurl.
 */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Rule #5: Check if the new data will exceed the maximum allowed size.
    if (mem->size + realsize > MAX_RESPONSE_SIZE) {
        fprintf(stderr, "Error: Maximum response size exceeded.\n");
        return 0; // Returning 0 signals an error to libcurl
    }

    char *ptr = (char*) realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        // Out of memory!
        fprintf(stderr, "Error: Not enough memory (realloc returned NULL).\n");
        return 0; // Returning 0 signals an error to libcurl
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate the buffer

    return realsize;
}

/**
 * @brief Extracts the content of the <title> tag from an HTML string.
 *
 * This function performs a simple search for "<title>" and "</title>" tags.
 * It does not handle variations in capitalization or attributes within the tag.
 *
 * @param html_content A null-terminated string containing the HTML content.
 * @return A newly allocated string containing the page title, or NULL if the title
 *         cannot be found or if memory allocation fails. The caller is responsible
 *         for freeing the returned string.
 */
char* extract_title(const char* html_content) {
    if (!html_content) {
        return NULL;
    }

    const char *title_start_tag = "<title>";
    const char *title_end_tag = "</title>";
    char *title = NULL;

    const char *start_ptr = strstr(html_content, title_start_tag);
    if (start_ptr) {
        start_ptr += strlen(title_start_tag); // Move pointer to the beginning of the title text
        const char *end_ptr = strstr(start_ptr, title_end_tag);

        if (end_ptr) {
            size_t title_len = end_ptr - start_ptr;
            // Rule #5: Ensure buffer is correctly sized.
            title = (char *)malloc(title_len + 1);
            if (title) {
                memcpy(title, start_ptr, title_len);
                title[title_len] = '\0'; // Null-terminate the new string
            } else {
                fprintf(stderr, "Error: Failed to allocate memory for title.\n");
            }
        }
    }
    return title;
}

/**
 * @brief Fetches an HTTPS URL and extracts the page title.
 *
 * This function uses libcurl to perform an HTTPS GET request. It enforces security
 * best practices like certificate verification, hostname verification, and TLS 1.2+.
 *
 * @param url The HTTPS URL to fetch.
 * @return A newly allocated string with the page title, or NULL on failure.
 *         The caller is responsible for freeing the returned string.
 */
char* get_page_title(const char* url) {
    CURL *curl_handle = NULL;
    CURLcode res;
    struct MemoryStruct chunk;
    char *title = NULL;

    // Initialize the memory chunk
    // Rule #13: Initialize pointers
    chunk.memory = (char*) malloc(1); 
    if (chunk.memory == NULL) {
        fprintf(stderr, "Error: Failed to allocate initial memory.\n");
        return NULL;
    }
    chunk.size = 0;

    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "curl_easy_init() failed.\n");
        free(chunk.memory);
        return NULL;
    }

    // Set libcurl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);
    // Rule #3: Restrict protocols to only HTTPS to be safe.
    curl_easy_setopt(curl_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

    // Rules #1, #2, #7, #8, #13: Enforce strict SSL/TLS security
    // Verify the server's SSL certificate.
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    // Verify the certificate's name against the host.
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
    // Enforce minimum TLS 1.2
    curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    // Perform the request
    res = curl_easy_perform(curl_handle);

    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code >= 200 && response_code < 300) {
            title = extract_title(chunk.memory);
        } else {
            fprintf(stderr, "Request failed with HTTP status code: %ld\n", response_code);
        }
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Cleanup
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);

    return title;
}

int main(int argc, char *argv[]) {
    // Initialize libcurl globally
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
        fprintf(stderr, "curl_global_init() failed.\n");
        return 1;
    }

    if (argc > 1) {
        // Process command line argument
        const char *url = argv[1];

        // Rule #3: Perform basic input validation.
        if (strncmp(url, "https://", 8) != 0) {
            fprintf(stderr, "Error: URL must start with 'https://'\n");
            curl_global_cleanup();
            return 1;
        }

        printf("Fetching title for: %s\n", url);
        char *title = get_page_title(url);
        if (title) {
            printf("Title: %s\n", title);
            free(title); // Free the allocated title string
        } else {
            printf("Could not retrieve or find a title.\n");
        }
    } else {
        // Run hardcoded test cases if no command line argument is provided
        printf("No URL provided. Running built-in test cases...\n\n");
        const char* test_urls[] = {
            "https://www.google.com",
            "https://example.com",
            "https://github.com",
            "https://www.ietf.org/rfc/rfc2616.txt", // A text file with no <title> tag
            "https://invalid.domain.for.testing.xyz" // A non-existent domain
        };
        int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

        for (int i = 0; i < num_tests; ++i) {
            printf("--- Test Case %d: %s ---\n", i + 1, test_urls[i]);
            char *title = get_page_title(test_urls[i]);
            if (title) {
                printf("Title: %s\n", title);
                free(title); // Free the allocated title string
            } else {
                printf("Could not retrieve or find a title.\n");
            }
            printf("\n");
        }
    }

    // Cleanup libcurl globally
    curl_global_cleanup();
    return 0;
}