// INSTRUCTIONS:
// This code requires the 'libcurl' library.
// 1. Install libcurl development libraries (e.g., 'sudo apt-get install libcurl4-openssl-dev' on Debian/Ubuntu).
// 2. Compile with: gcc -std=c11 Task76.c -o task76 -lcurl

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

// Struct to hold the response data from curl
struct MemoryStruct {
    char *memory;
    size_t size;
};

// A portable case-insensitive string search function.
char* strcasestr_portable(const char* haystack, const char* needle) {
    if (!needle || !*needle) return (char*)haystack;
    for (; *haystack; ++haystack) {
        if (toupper((unsigned char)*haystack) == toupper((unsigned char)*needle)) {
            const char* h = haystack;
            const char* n = needle;
            for (;;) {
                h++;
                n++;
                if (!*n) return (char*)haystack; // needle found
                if (!*h) return NULL; // haystack ended
                if (toupper((unsigned char)*h) != toupper((unsigned char)*n)) break;
            }
        }
    }
    return NULL;
}

// Callback function to write data received from curl into our MemoryStruct.
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
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

// NOTE: This function returns a dynamically allocated string that the CALLER MUST FREE.
char* extractPageTitle(const char* url) {
    // 1. Input Validation
    if (url == NULL || strncmp(url, "https://", 8) != 0) {
        // strdup is POSIX, not C standard, so use malloc+strcpy for portability
        char* error_msg = malloc(strlen("Error: Invalid or insecure URL provided. Please use HTTPS.") + 1);
        if(error_msg) strcpy(error_msg, "Error: Invalid or insecure URL provided. Please use HTTPS.");
        return error_msg;
    }

    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk = { .memory = malloc(1), .size = 0 };
    if (!chunk.memory) return NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    char* result_message = NULL;

    if (curl_handle) {
        // 2. Set curl options: URL, callbacks, security settings.
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-c-app/1.0");
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
        // Certificate validation is ON by default and should not be disabled.

        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            const char* err_str = curl_easy_strerror(res);
            size_t len = snprintf(NULL, 0, "Error: curl failed: %s", err_str);
            result_message = malloc(len + 1);
            if(result_message) snprintf(result_message, len + 1, "Error: curl failed: %s", err_str);
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
            if (http_code != 200) {
                 size_t len = snprintf(NULL, 0, "Error: Received non-200 status code: %ld", http_code);
                 result_message = malloc(len + 1);
                 if(result_message) snprintf(result_message, len + 1, "Error: Received non-200 status code: %ld", http_code);
            } else {
                // 3. Extract title
                char* title_start_tag = strcasestr_portable(chunk.memory, "<title>");
                if (title_start_tag) {
                    title_start_tag += strlen("<title>");
                    char* title_end_tag = strcasestr_portable(title_start_tag, "</title>");
                    if (title_end_tag) {
                        size_t title_len = title_end_tag - title_start_tag;
                        result_message = malloc(title_len + 1);
                        if (result_message) {
                            strncpy(result_message, title_start_tag, title_len);
                            result_message[title_len] = '\0';
                            
                            // Trim whitespace
                            char* start = result_message;
                            while(isspace((unsigned char)*start)) start++;
                            char* end = result_message + strlen(result_message) - 1;
                            while(end > start && isspace((unsigned char)*end)) end--;
                            *(end + 1) = '\0';
                            memmove(result_message, start, strlen(start) + 1);
                        }
                    } else {
                        result_message = malloc(50);
                        if(result_message) strcpy(result_message, "Error: Closing title tag not found.");
                    }
                } else {
                    result_message = malloc(50);
                    if(result_message) strcpy(result_message, "Error: Title tag not found.");
                }
            }
        }
        curl_easy_cleanup(curl_handle);
    } else {
        result_message = malloc(50);
        if(result_message) strcpy(result_message, "Error: Failed to initialize curl.");
    }
    
    free(chunk.memory);
    curl_global_cleanup();
    return result_message;
}

int main(void) {
    const char* testUrls[] = {
        "https://www.google.com",
        "https://github.com",
        "https://github.com/nonexistent/repo",  // 404 test
        "https://thissitedoesnotexist12345.com",  // DNS error
        "http://www.google.com"  // Insecure URL test
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("Testing URL: %s\n", testUrls[i]);
        char *title = extractPageTitle(testUrls[i]);
        if (title) {
            printf("Page Title: %s\n", title);
            free(title); // IMPORTANT: Free the memory returned by the function
        } else {
            printf("Page Title: Error - Function returned NULL\n");
        }
        printf("--------------------\n");
    }

    return 0;
}