/*
 * COMPILE & RUN INSTRUCTIONS for a Linux-based system:
 * 1. Install libcurl development package:
 *    - On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
 *    - On Fedora/CentOS: sudo dnf install libcurl-devel
 * 2. Compile the code:
 *    gcc Task76.c -o Task76 -lcurl
 * 3. Run the program with test cases:
 *    ./Task76
 * 4. Run with a command-line argument:
 *    ./Task76 https://some.url/
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Struct to hold memory buffer from libcurl
struct MemoryStruct {
    char* memory;
    size_t size;
};

// Case-insensitive strstr
char* stristr(const char* haystack, const char* needle) {
    if (!needle || !*needle) return (char*)haystack;
    for (; *haystack; ++haystack) {
        if (tolower((unsigned char)*haystack) == tolower((unsigned char)*needle)) {
            const char* h = haystack;
            const char* n = needle;
            for (; *h && *n; ++h, ++n) {
                if (tolower((unsigned char)*h) != tolower((unsigned char)*n)) break;
            }
            if (!*n) return (char*)haystack;
        }
    }
    return NULL;
}

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;
    
    // Prevent DoS by limiting total size
    const size_t MAX_SIZE = 1024 * 512; // 512 KB limit
    if (mem->size + realsize > MAX_SIZE) {
        fprintf(stderr, "error: response body too large\n");
        return 0; // Stops the transfer
    }

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "error: not enough memory (realloc returned NULL)\n");
        return 0; // Out of memory
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate

    return realsize;
}

// The caller of this function is responsible for freeing the returned string.
char* getPageTitle(const char* url) {
    // 1. Input validation
    if (strncmp(url, "https://", 8) != 0) {
        char* error_msg = strdup("Error: URL must use HTTPS.");
        return error_msg;
    }

    CURL* curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1); // will be grown by realloc
    chunk.size = 0;

    if(chunk.memory == NULL) {
        return strdup("Error: Failed to allocate memory.");
    }
    
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        free(chunk.memory);
        return strdup("Error: Failed to initialize libcurl.");
    }
    
    // 2. Set secure curl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 15L);

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Error: curl_easy_perform() failed: %s", curl_easy_strerror(res));
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return strdup(buffer);
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    if(http_code != 200) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Error: Received non-200 status code: %ld", http_code);
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return strdup(buffer);
    }

    // 3. Simple and secure parsing (case-insensitive)
    char* title_start = stristr(chunk.memory, "<title>");
    if (!title_start) {
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return strdup("Error: Title tag not found.");
    }
    title_start += 7; // Move pointer past "<title>"

    char* title_end = stristr(title_start, "</title>");
    if (!title_end) {
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return strdup("Error: Closing title tag not found.");
    }

    size_t title_len = title_end - title_start;
    char* title = malloc(title_len + 1);
    if (!title) {
        free(chunk.memory);
        curl_easy_cleanup(curl_handle);
        return strdup("Error: Failed to allocate memory for title.");
    }

    memcpy(title, title_start, title_len);
    title[title_len] = '\0';

    // Cleanup
    free(chunk.memory);
    curl_easy_cleanup(curl_handle);

    return title;
}

int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char* testUrls[] = {
        "https://www.google.com",
        "https://github.com",
        "https://www.microsoft.com/en-us/",
        "https://www.w3.org/",
        "https://example.com"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_tests; i++) {
        printf("URL: %s\n", testUrls[i]);
        char* title = getPageTitle(testUrls[i]);
        printf("Title: %s\n", title);
        free(title); // Free the memory returned by the function
        printf("--------------------\n");
    }

    if (argc > 1) {
        printf("\n--- Running with Command Line Argument ---\n");
        const char* urlFromArg = argv[1];
        printf("URL: %s\n", urlFromArg);
        char* title = getPageTitle(urlFromArg);
        printf("Title: %s\n", title);
        free(title);
        printf("--------------------\n");
    } else {
        printf("\nNote: You can also provide a URL as a command-line argument.\n");
    }
    
    curl_global_cleanup();
    return 0;
}