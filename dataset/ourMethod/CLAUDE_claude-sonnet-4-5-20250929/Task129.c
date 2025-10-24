
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>

#define MAX_FILENAME_LEN 255
#define MAX_URL_LEN 512
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024) /* 10 MB limit */
#define BASE_URL "https://example.com/files/"

/* Structure to hold response data */
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ResponseBuffer;

/* Secure memory clearing function that compiler cannot optimize away */
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Initialize response buffer */
static int init_response_buffer(ResponseBuffer *buf) {
    if (buf == NULL) return 0;
    
    buf->capacity = 4096;
    buf->data = (char *)calloc(1, buf->capacity);
    if (buf->data == NULL) {
        return 0;
    }
    buf->size = 0;
    return 1;
}

/* Free response buffer with secure clearing */
static void free_response_buffer(ResponseBuffer *buf) {
    if (buf == NULL) return;
    if (buf->data != NULL) {
        secure_zero(buf->data, buf->capacity);
        free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0;
    buf->capacity = 0;
}

/* Callback for curl write operation with bounds checking and growth limit */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize;
    ResponseBuffer *buf = (ResponseBuffer *)userp;
    
    if (buf == NULL || contents == NULL) return 0;
    
    /* Check for integer overflow in multiplication */
    if (size > 0 && nmemb > SIZE_MAX / size) {
        return 0;
    }
    realsize = size * nmemb;
    
    /* Check for integer overflow in addition */
    if (buf->size > SIZE_MAX - realsize) {
        return 0;
    }
    
    /* Enforce maximum response size to prevent excessive memory usage */
    if (buf->size + realsize > MAX_RESPONSE_SIZE) {
        return 0;
    }
    
    /* Grow buffer if needed */
    if (buf->size + realsize + 1 > buf->capacity) {
        size_t new_capacity = buf->capacity * 2;
        
        /* Ensure new capacity is large enough */
        while (new_capacity < buf->size + realsize + 1) {
            if (new_capacity > SIZE_MAX / 2) {
                return 0;
            }
            new_capacity *= 2;
        }
        
        /* Check against maximum allowed size */
        if (new_capacity > MAX_RESPONSE_SIZE) {
            new_capacity = MAX_RESPONSE_SIZE;
            if (buf->size + realsize + 1 > new_capacity) {
                return 0;
            }
        }
        
        char *new_data = (char *)realloc(buf->data, new_capacity);
        if (new_data == NULL) {
            return 0;
        }
        
        buf->data = new_data;
        buf->capacity = new_capacity;
    }
    
    /* Copy data with bounds checking */
    memcpy(buf->data + buf->size, contents, realsize);
    buf->size += realsize;
    buf->data[buf->size] = '\\0'; /* Ensure null termination */
    
    return realsize;
}

/* Validate filename to prevent path traversal attacks */
static int validate_filename(const char *filename) {
    size_t len;
    size_t i;
    
    if (filename == NULL) return 0;
    
    len = strlen(filename);
    
    /* Check length bounds */
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return 0;
    }
    
    /* Reject paths with traversal patterns */
    if (strstr(filename, "..") != NULL) {
        return 0;
    }
    if (strstr(filename, "./") != NULL || strstr(filename, "/.") != NULL) {
        return 0;
    }
    if (filename[0] == '/' || filename[0] == '\\\\') {\n        return 0;\n    }\n    \n    /* Check for path separators and other dangerous characters */\n    for (i = 0; i < len; i++) {\n        char c = filename[i];\n        if (c == '/' || c == '\\\\' || c == ':' || c == '*' || \n            c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {\n            return 0;\n        }\n        /* Reject control characters */\n        if (iscntrl((unsigned char)c)) {\n            return 0;\n        }\n    }\n    \n    return 1;\n}\n\n/* URL encode a string to prevent injection attacks */\nstatic char* url_encode(const char *str) {\n    size_t len;\n    size_t i;\n    size_t encoded_len;\n    char *encoded;\n    size_t pos;\n    \n    if (str == NULL) return NULL;\n    \n    len = strlen(str);\n    \n    /* Check for integer overflow: each char can expand to %XX (3 bytes) */\n    if (len > SIZE_MAX / 3) {\n        return NULL;\n    }\n    \n    encoded_len = len * 3 + 1;\n    encoded = (char *)calloc(1, encoded_len);\n    if (encoded == NULL) {\n        return NULL;\n    }\n    \n    pos = 0;\n    for (i = 0; i < len; i++) {\n        unsigned char c = (unsigned char)str[i];\n        \n        /* Check bounds before writing */\n        if (pos + 4 > encoded_len) {\n            secure_zero(encoded, encoded_len);\n            free(encoded);\n            return NULL;\n        }\n        \n        /* Encode special characters */\n        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {\n            encoded[pos++] = c;\n        } else {\n            int written = snprintf(encoded + pos, encoded_len - pos, "%%%02X", c);\n            if (written < 0 || written >= (int)(encoded_len - pos)) {\n                secure_zero(encoded, encoded_len);\n                free(encoded);\n                return NULL;\n            }\n            pos += written;\n        }\n    }\n    \n    encoded[pos] = '\\0';\n    return encoded;\n}\n\n/* Retrieve file from server using HTTPS with full security settings */\nstatic int retrieve_file(const char *filename, ResponseBuffer *response) {\n    CURL *curl = NULL;\n    CURLcode res;\n    char url[MAX_URL_LEN];\n    char *encoded_filename = NULL;\n    int result = 0;\n    \n    if (filename == NULL || response == NULL) {\n        return 0;\n    }\n    \n    /* Validate filename to prevent path traversal */\n    if (!validate_filename(filename)) {\n        fprintf(stderr, "Error: Invalid filename\
");\n        return 0;\n    }\n    \n    /* URL encode the filename to prevent injection */\n    encoded_filename = url_encode(filename);\n    if (encoded_filename == NULL) {\n        fprintf(stderr, "Error: Failed to encode filename\
");\n        return 0;\n    }\n    \n    /* Build URL with bounds checking */\n    if (snprintf(url, sizeof(url), "%s%s", BASE_URL, encoded_filename) >= (int)sizeof(url)) {\n        fprintf(stderr, "Error: URL too long\
");\n        secure_zero(encoded_filename, strlen(encoded_filename));\n        free(encoded_filename);\n        return 0;\n    }\n    \n    /* Clean up encoded filename */\n    secure_zero(encoded_filename, strlen(encoded_filename));\n    free(encoded_filename);\n    encoded_filename = NULL;\n    \n    curl = curl_easy_init();\n    if (curl == NULL) {\n        fprintf(stderr, "Error: Failed to initialize curl\
");\n        return 0;\n    }\n    \n    /* Set URL */\n    res = curl_easy_setopt(curl, CURLOPT_URL, url);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Restrict to HTTPS only */\n    res = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Enable SSL certificate verification */\n    res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Enable hostname verification */\n    res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Set minimum TLS version to 1.2 */\n    res = curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Set timeouts */\n    res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);\n    if (res != CURLE_OK) goto cleanup;\n    \n    res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Limit redirects */\n    res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);\n    if (res != CURLE_OK) goto cleanup;\n    \n    res = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Set write callback and buffer */\n    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);\n    if (res != CURLE_OK) goto cleanup;\n    \n    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);\n    if (res != CURLE_OK) goto cleanup;\n    \n    /* Perform the request */\n    res = curl_easy_perform(curl);\n    if (res != CURLE_OK) {\n        fprintf(stderr, "Error: Request failed\
");\n        goto cleanup;\n    }\n    \n    /* Check HTTP response code */\n    long response_code = 0;\n    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);\n    if (res != CURLE_OK || response_code != 200) {\n        fprintf(stderr, "Error: Invalid response\
");\n        goto cleanup;\n    }\n    \n    result = 1;\n    \ncleanup:\n    if (curl != NULL) {\n        curl_easy_cleanup(curl);\n    }\n    \n    return result;\n}\n\nint main(void) {\n    const char *test_files[] = {\n        "document.txt",\n        "data.json",\n        "report.pdf",\n        "image.png",\n        "config.xml"\n    };\n    int i;\n    CURLcode res;\n    \n    /* Initialize curl globally */\n    res = curl_global_init(CURL_GLOBAL_DEFAULT);\n    if (res != CURLE_OK) {\n        fprintf(stderr, "Error: Failed to initialize curl library\
");\n        return EXIT_FAILURE;\n    }\n    \n    printf("File Retrieval System - Test Cases\
");\n    printf("===================================\
\
");\n    \n    /* Test with 5 different filenames */\n    for (i = 0; i < 5; i++) {\n        ResponseBuffer response;\n        \n        printf("Test %d: Retrieving file '%s'\
", i + 1, test_files[i]);\n        \n        /* Initialize response buffer */\n        if (!init_response_buffer(&response)) {\n            fprintf(stderr, "Error: Failed to initialize buffer\
");\n            continue;\n        }\n        \n        /* Retrieve file */\n        if (retrieve_file(test_files[i], &response)) {\n            printf("Success: Retrieved %zu bytes\
", response.size);\n            \n            /* Display first 100 bytes or less */\n            size_t preview_len = response.size < 100 ? response.size : 100;\n            if (preview_len > 0) {\n                printf("Preview: ");\n                fwrite(response.data, 1, preview_len, stdout);\n                if (response.size > 100) {\n                    printf("...");\n                }\n                printf("\
");\n            }\n        } else {\n            printf("Failed to retrieve file\
");\n        }\n        \n        /* Clean up response buffer */\n        free_response_buffer(&response);\n        printf("\
");\n    }\n    \n    /* Test path traversal attempt (should fail) */\n    printf("Test 6: Security test - path traversal attempt\
");\n    ResponseBuffer response;\n    if (!init_response_buffer(&response)) {\n        fprintf(stderr, "Error: Failed to initialize buffer\
");\n    } else {\n        if (retrieve_file("../../../etc/passwd", &response)) {\n            printf("SECURITY FAILURE: Path traversal was not blocked\
");\n        } else {\n            printf("Success: Path traversal attack blocked\
");
        }
        free_response_buffer(&response);
    }
    
    /* Cleanup curl globally */
    curl_global_cleanup();
    
    return EXIT_SUCCESS;
}
