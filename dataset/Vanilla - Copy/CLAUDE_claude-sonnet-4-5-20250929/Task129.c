
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("Error: Not enough memory\\n");
        return 0;
    }
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

void sanitizeFileName(const char *input, char *output, size_t outputSize) {
    if (input == NULL || strlen(input) == 0) {
        strncpy(output, "default.txt", outputSize - 1);
        output[outputSize - 1] = '\\0';
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < outputSize - 1; i++) {
        char c = input[i];
        
        // Skip path traversal and path separators
        if ((c == '.' && i + 1 < strlen(input) && input[i + 1] == '.') ||
            c == '/' || c == '\\\\') {\n            if (c == '.' && i + 1 < strlen(input) && input[i + 1] == '.') {\n                i++; // Skip the next dot too\n            }\n            continue;\n        }\n        \n        // Keep only alphanumeric, dots, hyphens, and underscores\n        if (isalnum(c) || c == '.' || c == '-' || c == '_') {\n            output[j++] = c;\n        }\n    }\n    output[j] = '\\0';
}

char* retrieveFileFromServer(const char *serverUrl, const char *fileName) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    // Sanitize filename
    char sanitizedFileName[256];
    sanitizeFileName(fileName, sanitizedFileName, sizeof(sanitizedFileName));
    
    // Construct full URL
    char fullUrl[512];
    snprintf(fullUrl, sizeof(fullUrl), "%s/%s", serverUrl, sanitizedFileName);
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fullUrl);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            free(chunk.memory);
            char *errorMsg = malloc(256);
            snprintf(errorMsg, 256, "Error: %s", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return errorMsg;
        }
        
        curl_easy_cleanup(curl);
    } else {
        free(chunk.memory);
        char *errorMsg = malloc(256);
        snprintf(errorMsg, 256, "Error: Failed to initialize CURL");
        return errorMsg;
    }
    
    return chunk.memory;
}

int main() {
    // Test cases
    const char *serverUrl = "http://example.com/files";
    
    printf("Test Case 1: Valid file name\\n");
    char *result1 = retrieveFileFromServer(serverUrl, "document.txt");
    printf("%s\\n\\n", result1);
    free(result1);
    
    printf("Test Case 2: File with numbers\\n");
    char *result2 = retrieveFileFromServer(serverUrl, "report_2024.pdf");
    printf("%s\\n\\n", result2);
    free(result2);
    
    printf("Test Case 3: Attempting path traversal (sanitized)\\n");
    char *result3 = retrieveFileFromServer(serverUrl, "../../../etc/passwd");
    printf("%s\\n\\n", result3);
    free(result3);
    
    printf("Test Case 4: File with special characters\\n");
    char *result4 = retrieveFileFromServer(serverUrl, "data-file_01.json");
    printf("%s\\n\\n", result4);
    free(result4);
    
    printf("Test Case 5: Empty filename\\n");
    char *result5 = retrieveFileFromServer(serverUrl, "");
    printf("%s\\n\\n", result5);
    free(result5);
    
    return 0;
}
