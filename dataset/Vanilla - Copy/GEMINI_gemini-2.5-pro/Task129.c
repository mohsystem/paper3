#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// This code requires libcurl.
// To compile: gcc your_file_name.c -o output -lcurl
#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

// Callback function for libcurl to write received data into our MemoryStruct
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
 * @brief Retrieves the content of a file from a server.
 * @param baseUrl The base URL of the server (e.g., "https://example.com/files").
 * @param fileName The name of the file to retrieve (e.g., "data.txt").
 * @return A dynamically allocated string with the file content or an error message.
 *         The caller is responsible for freeing this string.
 */
char* retrieveFileFromServer(const char* baseUrl, const char* fileName) {
    if (fileName == NULL || fileName[0] == '\0') {
        char* errorMsg = malloc(strlen("Error: File name cannot be empty.") + 1);
        if (errorMsg) strcpy(errorMsg, "Error: File name cannot be empty.");
        return errorMsg;
    }
    
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Build the full URL
    int needs_slash = (baseUrl[strlen(baseUrl) - 1] != '/');
    char* fullUrl = malloc(strlen(baseUrl) + strlen(fileName) + needs_slash + 1);
    strcpy(fullUrl, baseUrl);
    if (needs_slash) {
        strcat(fullUrl, "/");
    }
    strcat(fullUrl, fileName);

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    
    if(!curl_handle) {
        free(fullUrl);
        free(chunk.memory);
        char* errorMsg = malloc(strlen("Error: Could not initialize libcurl.") + 1);
        if (errorMsg) strcpy(errorMsg, "Error: Could not initialize libcurl.");
        return errorMsg;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, fullUrl);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L);

    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        const char* curl_err = curl_easy_strerror(res);
        char* errorMsg = malloc(strlen("Error: curl_easy_perform() failed: ") + strlen(curl_err) + 1);
        if (errorMsg) sprintf(errorMsg, "Error: curl_easy_perform() failed: %s", curl_err);
        
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        free(fullUrl);
        free(chunk.memory);
        return errorMsg;
    } 

    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (http_code < 200 || http_code >= 300) {
        free(chunk.memory);
        char tempMsg[100];
        sprintf(tempMsg, "Error: Failed to retrieve file. HTTP status code: %ld", http_code);
        chunk.memory = malloc(strlen(tempMsg) + 1);
        if(chunk.memory) strcpy(chunk.memory, tempMsg);
    }
    
    curl_easy_cleanup(curl_handle);
    free(fullUrl);
    curl_global_cleanup();

    return chunk.memory;
}

int main(void) {
    const char* baseUrl = "https://jsonplaceholder.typicode.com";
    const char* testFiles[] = {
        "todos/1",
        "posts/10",
        "users/5",
        "nonexistent/path", // This will cause a 404
        "" // Empty file name
    };
    int num_tests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("--- Test Case %d: Retrieving '%s' ---\n", (i + 1), testFiles[i]);
        char* result = retrieveFileFromServer(baseUrl, testFiles[i]);
        if(result) {
            printf("Result:\n%s\n", result);
            free(result);
        } else {
            printf("Result: NULL (error occurred)\n");
        }
        printf("--- End Test Case %d ---\n\n", (i + 1));
    }

    return 0;
}