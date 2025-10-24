
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/* Structure to hold downloaded data */
struct MemoryStruct {
    char *memory;
    size_t size;
    size_t capacity;
};

/* Maximum response size: 10MB to prevent excessive memory usage */
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024)

/* Write callback for libcurl - grows buffer dynamically with size limit */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    /* Check for integer overflow in multiplication */
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        return 0; /* Signal error to libcurl */
    }
    
    /* Check for integer overflow in addition */
    if (realsize > SIZE_MAX - mem->size - 1) {
        return 0;
    }
    
    size_t new_size = mem->size + realsize;
    
    /* Enforce maximum response size to prevent memory exhaustion */
    if (new_size > MAX_RESPONSE_SIZE) {
        return 0; /* Signal error - response too large */
    }
    
    /* Grow buffer if needed */
    if (new_size >= mem->capacity) {
        size_t new_capacity = mem->capacity * 2;
        if (new_capacity < new_size + 1) {
            new_capacity = new_size + 1;
        }
        if (new_capacity > MAX_RESPONSE_SIZE) {
            new_capacity = MAX_RESPONSE_SIZE;
        }
        
        char *new_memory = realloc(mem->memory, new_capacity);
        if (new_memory == NULL) {
            return 0; /* Signal error - allocation failed */
        }
        mem->memory = new_memory;
        mem->capacity = new_capacity;
    }
    
    /* Copy data into buffer - bounds are validated above */
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = '\\0'; /* Null terminate */
    
    return realsize;
}

/* Extract title from HTML - returns dynamically allocated string or NULL */
static char* extract_title(const char *html) {
    if (html == NULL) {
        return NULL;
    }
    
    /* Find opening <title> tag - case insensitive search */
    const char *title_start = strcasestr(html, "<title");
    if (title_start == NULL) {
        return NULL;
    }
    
    /* Find the '>' character after <title to handle attributes */
    const char *content_start = strchr(title_start, '>');
    if (content_start == NULL) {
        return NULL;
    }
    content_start++; /* Move past '>' */
    
    /* Find closing </title> tag */
    const char *title_end = strcasestr(content_start, "</title>");
    if (title_end == NULL) {
        return NULL;
    }
    
    /* Calculate title length with bounds check */
    if (title_end < content_start) {
        return NULL;
    }
    
    size_t title_len = title_end - content_start;
    
    /* Limit title length to reasonable size (64KB) */
    if (title_len > 65536) {
        title_len = 65536;
    }
    
    /* Allocate memory for title with null terminator - check overflow */
    if (title_len > SIZE_MAX - 1) {
        return NULL;
    }
    
    char *title = malloc(title_len + 1);
    if (title == NULL) {
        return NULL; /* Allocation failed */
    }
    
    /* Copy title content with bounds checking */
    memcpy(title, content_start, title_len);
    title[title_len] = '\\0'; /* Null terminate */
    
    return title;
}

/* Validate URL format and protocol */
static int validate_url(const char *url) {
    if (url == NULL) {
        return 0;
    }
    
    size_t len = strlen(url);
    
    /* Check URL length - reject empty or excessively long URLs */
    if (len == 0 || len > 2048) {
        return 0;
    }
    
    /* Require HTTPS protocol only */
    if (strncmp(url, "https://", 8) != 0) {
        return 0;
    }
    
    return 1;
}

/* Fetch page title from HTTPS URL - returns allocated string or NULL */
char* fetch_page_title(const char *url) {
    CURL *curl_handle = NULL;
    CURLcode res;
    struct MemoryStruct chunk;
    char *title = NULL;
    
    /* Validate input URL */
    if (!validate_url(url)) {
        fprintf(stderr, "Error: Invalid URL or non-HTTPS protocol\\n");
        return NULL;
    }
    
    /* Initialize memory structure */
    chunk.memory = malloc(4096); /* Initial buffer size */
    if (chunk.memory == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    chunk.size = 0;
    chunk.capacity = 4096;
    chunk.memory[0] = '\\0';
    
    /* Initialize libcurl */
    curl_handle = curl_easy_init();
    if (curl_handle == NULL) {
        fprintf(stderr, "Error: Failed to initialize CURL\\n");
        free(chunk.memory);
        return NULL;
    }
    
    /* Set URL */
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    
    /* Restrict to HTTPS only - security requirement */
    curl_easy_setopt(curl_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl_handle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    
    /* Enable SSL/TLS certificate verification - mandatory for security */
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
    
    /* Enforce TLS 1.2 minimum */
    curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    /* Set write callback */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    
    /* Set reasonable timeout: 30 seconds */
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10L);
    
    /* Limit redirects to prevent redirect loops */
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 5L);
    
    /* Set User-Agent */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "TitleFetcher/1.0");
    
    /* Perform the request */
    res = curl_easy_perform(curl_handle);
    
    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: Request failed: %s\\n", curl_easy_strerror(res));
        goto cleanup;
    }
    
    /* Check HTTP response code */
    long response_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        fprintf(stderr, "Error: HTTP response code %ld\\n", response_code);
        goto cleanup;
    }
    
    /* Extract title from HTML */
    title = extract_title(chunk.memory);
    if (title == NULL) {
        fprintf(stderr, "Error: No title found in HTML\\n");
    }
    
cleanup:
    /* Clean up resources */
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    
    return title;
}

int main(int argc, char *argv[]) {
    /* Validate command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <https_url>\\n", argv[0]);
        fprintf(stderr, "Example: %s https://example.com\\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /* Initialize curl globally */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    /* Test cases when run directly */
    const char *test_urls[] = {
        argv[1],
        "https://www.example.com",
        "https://www.wikipedia.org",
        "https://www.github.com",
        "https://www.stackoverflow.com"
    };
    
    /* Process only the provided URL, not test cases in normal operation */
    char *title = fetch_page_title(argv[1]);
    
    if (title != NULL) {
        printf("Page title: %s\\n", title);
        free(title); /* Free allocated memory */
        curl_global_cleanup();
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Failed to retrieve page title\\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }
    
    /* Uncomment below to run all test cases */
    /*
    for (int i = 0; i < 5; i++) {
        printf("\\nTest %d: %s\\n", i + 1, test_urls[i]);
        char *title = fetch_page_title(test_urls[i]);
        if (title != NULL) {
            printf("Title: %s\\n", title);
            free(title);
        } else {
            printf("Failed to retrieve title\\n");
        }
    }
    */
    
    curl_global_cleanup();
    return EXIT_SUCCESS;
}
