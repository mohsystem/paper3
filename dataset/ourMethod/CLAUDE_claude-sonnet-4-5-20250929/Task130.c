
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_ORIGIN_LENGTH 2048
#define MAX_METHOD_LENGTH 16
#define MAX_HEADER_LENGTH 256
#define MAX_ORIGINS 50
#define MAX_METHODS 20
#define MAX_HEADERS 50
#define MAX_RESPONSE_HEADERS 30

/* CWE-20: Structure to hold validated strings with length limits */
typedef struct {
    char data[MAX_ORIGIN_LENGTH + 1];
    size_t length;
} SafeString;

/* CORS configuration structure */
typedef struct {
    SafeString allowedOrigins[MAX_ORIGINS];
    int originCount;
    SafeString allowedMethods[MAX_METHODS];
    int methodCount;
    SafeString allowedHeaders[MAX_HEADERS];
    int headerCount;
    SafeString exposedHeaders[MAX_HEADERS];
    int exposedHeaderCount;
    bool allowCredentials;
    int maxAge;
} CORSConfig;

/* HTTP Response header structure */
typedef struct {
    char name[MAX_HEADER_LENGTH + 1];
    char value[MAX_ORIGIN_LENGTH + 1];
} HTTPHeader;

/* HTTP Response structure */
typedef struct {
    int statusCode;
    HTTPHeader headers[MAX_RESPONSE_HEADERS];
    int headerCount;
    char body[1024];
} HTTPResponse;

/* Initialize CORS configuration with safe defaults */
void initCORSConfig(CORSConfig* config) {
    if (config == NULL) { /* CWE-476: NULL pointer check */
        return;
    }
    
    memset(config, 0, sizeof(CORSConfig)); /* CWE-665: Initialize all memory */
    
    config->originCount = 0;
    config->methodCount = 0;
    config->headerCount = 0;
    config->exposedHeaderCount = 0;
    config->allowCredentials = false;
    config->maxAge = 86400;
    
    /* Add default safe methods */
    strncpy(config->allowedMethods[0].data, "GET", MAX_METHOD_LENGTH);
    config->allowedMethods[0].data[MAX_METHOD_LENGTH] = '\\0'; /* CWE-120: Ensure null termination */
    config->allowedMethods[0].length = strlen(config->allowedMethods[0].data);
    
    strncpy(config->allowedMethods[1].data, "POST", MAX_METHOD_LENGTH);
    config->allowedMethods[1].data[MAX_METHOD_LENGTH] = '\\0';
    config->allowedMethods[1].length = strlen(config->allowedMethods[1].data);
    
    strncpy(config->allowedMethods[2].data, "HEAD", MAX_METHOD_LENGTH);
    config->allowedMethods[2].data[MAX_METHOD_LENGTH] = '\\0';
    config->allowedMethods[2].length = strlen(config->allowedMethods[2].data);
    
    config->methodCount = 3;
}

/* CWE-20: Validate origin format */
bool isValidOrigin(const char* origin, size_t length) {
    if (origin == NULL || length == 0 || length > MAX_ORIGIN_LENGTH) {
        return false;
    }
    
    /* Check for null bytes in the string - CWE-20 */
    for (size_t i = 0; i < length; i++) {
        if (origin[i] == '\\0' && i < length - 1) {
            return false;
        }
    }
    
    /* Must contain :// */
    const char* scheme_end = strstr(origin, "://");
    if (scheme_end == NULL || scheme_end == origin) {
        return false;
    }
    
    /* Extract scheme */
    size_t scheme_len = scheme_end - origin;
    if (scheme_len > 10) { /* Reasonable scheme length */
        return false;
    }
    
    /* Check scheme is http or https - CWE-20 */
    if ((scheme_len == 4 && strncmp(origin, "http", 4) == 0) ||
        (scheme_len == 5 && strncmp(origin, "https", 5) == 0)) {
        /* Valid scheme */
    } else {
        return false;
    }
    
    /* Check for control characters and spaces - CWE-20 */
    for (size_t i = 0; i < length; i++) {
        if (iscntrl((unsigned char)origin[i]) || isspace((unsigned char)origin[i])) {
            return false;
        }
    }
    
    return true;
}

/* CWE-20: Validate HTTP method */
bool isValidMethod(const char* method, size_t length) {
    if (method == NULL || length == 0 || length > MAX_METHOD_LENGTH) {
        return false;
    }
    
    /* Check for control characters and spaces - CWE-20 */
    for (size_t i = 0; i < length; i++) {
        if (iscntrl((unsigned char)method[i]) || isspace((unsigned char)method[i])) {
            return false;
        }
    }
    
    return true;
}

/* CWE-20: Validate header name */
bool isValidHeaderName(const char* header, size_t length) {
    if (header == NULL || length == 0 || length > MAX_HEADER_LENGTH) {
        return false;
    }
    
    /* Check for control characters - CWE-20 */
    for (size_t i = 0; i < length; i++) {
        if (iscntrl((unsigned char)header[i])) {
            return false;
        }
    }
    
    return true;
}

/* Add allowed origin with validation */
bool addAllowedOrigin(CORSConfig* config, const char* origin) {
    if (config == NULL || origin == NULL) { /* CWE-476: NULL pointer check */
        return false;
    }
    
    size_t len = strnlen(origin, MAX_ORIGIN_LENGTH + 1);
    
    if (!isValidOrigin(origin, len)) { /* CWE-20: Validate input */
        return false;
    }
    
    if (config->originCount >= MAX_ORIGINS) { /* CWE-119: Bounds check */
        return false;
    }
    
    strncpy(config->allowedOrigins[config->originCount].data, origin, MAX_ORIGIN_LENGTH);
    config->allowedOrigins[config->originCount].data[MAX_ORIGIN_LENGTH] = '\\0'; /* CWE-120: Null terminate */
    config->allowedOrigins[config->originCount].length = len;
    config->originCount++;
    
    return true;
}

/* Add allowed method with validation */
bool addAllowedMethod(CORSConfig* config, const char* method) {
    if (config == NULL || method == NULL) { /* CWE-476: NULL pointer check */
        return false;
    }
    
    size_t len = strnlen(method, MAX_METHOD_LENGTH + 1);
    
    if (!isValidMethod(method, len)) { /* CWE-20: Validate input */
        return false;
    }
    
    if (config->methodCount >= MAX_METHODS) { /* CWE-119: Bounds check */
        return false;
    }
    
    strncpy(config->allowedMethods[config->methodCount].data, method, MAX_METHOD_LENGTH);
    config->allowedMethods[config->methodCount].data[MAX_METHOD_LENGTH] = '\\0';
    config->allowedMethods[config->methodCount].length = len;
    config->methodCount++;
    
    return true;
}

/* Add allowed header with validation */
bool addAllowedHeader(CORSConfig* config, const char* header) {
    if (config == NULL || header == NULL) { /* CWE-476: NULL pointer check */
        return false;
    }
    
    size_t len = strnlen(header, MAX_HEADER_LENGTH + 1);
    
    if (!isValidHeaderName(header, len)) { /* CWE-20: Validate input */
        return false;
    }
    
    if (config->headerCount >= MAX_HEADERS) { /* CWE-119: Bounds check */
        return false;
    }
    
    strncpy(config->allowedHeaders[config->headerCount].data, header, MAX_HEADER_LENGTH);
    config->allowedHeaders[config->headerCount].data[MAX_HEADER_LENGTH] = '\\0';
    config->allowedHeaders[config->headerCount].length = len;
    config->headerCount++;
    
    return true;
}

/* Add exposed header with validation */
bool addExposedHeader(CORSConfig* config, const char* header) {
    if (config == NULL || header == NULL) { /* CWE-476: NULL pointer check */
        return false;
    }
    
    size_t len = strnlen(header, MAX_HEADER_LENGTH + 1);
    
    if (!isValidHeaderName(header, len)) { /* CWE-20: Validate input */
        return false;
    }
    
    if (config->exposedHeaderCount >= MAX_HEADERS) { /* CWE-119: Bounds check */
        return false;
    }
    
    strncpy(config->exposedHeaders[config->exposedHeaderCount].data, header, MAX_HEADER_LENGTH);
    config->exposedHeaders[config->exposedHeaderCount].data[MAX_HEADER_LENGTH] = '\\0';
    config->exposedHeaders[config->exposedHeaderCount].length = len;
    config->exposedHeaderCount++;
    
    return true;
}

/* Check if origin is allowed */
bool isOriginAllowed(const CORSConfig* config, const char* origin) {
    if (config == NULL || origin == NULL) { /* CWE-476: NULL pointer check */
        return false;
    }
    
    size_t len = strnlen(origin, MAX_ORIGIN_LENGTH + 1);
    
    if (!isValidOrigin(origin, len)) { /* CWE-20: Validate before processing */
        return false;
    }
    
    /* Check for wildcard - only if credentials not allowed (security requirement) */
    for (int i = 0; i < config->originCount; i++) {
        if (strcmp(config->allowedOrigins[i].data, "*") == 0 && !config->allowCredentials) {
            return true;
        }
    }
    
    /* Check exact match */
    for (int i = 0; i < config->originCount; i++) {
        if (strcmp(config->allowedOrigins[i].data, origin) == 0) {
            return true;
        }
    }
    
    return false;
}

/* Add response header safely */
bool addResponseHeader(HTTPResponse* response, const char* name, const char* value) {
    if (response == NULL || name == NULL || value == NULL) { /* CWE-476: NULL pointer check */
        return false;
    }
    
    if (response->headerCount >= MAX_RESPONSE_HEADERS) { /* CWE-119: Bounds check */
        return false;
    }
    
    size_t nameLen = strnlen(name, MAX_HEADER_LENGTH + 1);
    size_t valueLen = strnlen(value, MAX_ORIGIN_LENGTH + 1);
    
    if (nameLen > MAX_HEADER_LENGTH || valueLen > MAX_ORIGIN_LENGTH) {
        return false;
    }
    
    strncpy(response->headers[response->headerCount].name, name, MAX_HEADER_LENGTH);
    response->headers[response->headerCount].name[MAX_HEADER_LENGTH] = '\\0'; /* CWE-120: Null terminate */
    
    strncpy(response->headers[response->headerCount].value, value, MAX_ORIGIN_LENGTH);
    response->headers[response->headerCount].value[MAX_ORIGIN_LENGTH] = '\\0';
    
    response->headerCount++;
    return true;
}

/* Generate CORS headers */
void generateCORSHeaders(const CORSConfig* config, HTTPResponse* response,
                        const char* origin, bool isPreflight) {
    if (config == NULL || response == NULL || origin == NULL) { /* CWE-476: NULL pointer check */
        return;
    }
    
    size_t originLen = strnlen(origin, MAX_ORIGIN_LENGTH + 1);
    
    if (!isValidOrigin(origin, originLen)) { /* CWE-20: Validate input */
        return;
    }
    
    if (!isOriginAllowed(config, origin)) {
        return; /* No CORS headers if origin not allowed */
    }
    
    /* Set Access-Control-Allow-Origin (never use * with credentials) */
    bool hasWildcard = false;
    for (int i = 0; i < config->originCount; i++) {
        if (strcmp(config->allowedOrigins[i].data, "*") == 0) {
            hasWildcard = true;
            break;
        }
    }
    
    if (config->allowCredentials) {
        addResponseHeader(response, "Access-Control-Allow-Origin", origin);
    } else if (hasWildcard) {
        addResponseHeader(response, "Access-Control-Allow-Origin", "*");
    } else {
        addResponseHeader(response, "Access-Control-Allow-Origin", origin);
    }
    
    /* Set Access-Control-Allow-Credentials */
    if (config->allowCredentials) {
        addResponseHeader(response, "Access-Control-Allow-Credentials", "true");
    }
    
    /* Preflight-specific headers */
    if (isPreflight) {
        /* Access-Control-Allow-Methods */
        if (config->methodCount > 0) {
            char methods[512] = {0}; /* CWE-665: Initialize buffer */
            size_t offset = 0;
            
            for (int i = 0; i < config->methodCount && offset < sizeof(methods) - 20; i++) {
                if (i > 0) {
                    int written = snprintf(methods + offset, sizeof(methods) - offset, ", "); /* CWE-120: Safe formatting */
                    if (written < 0 || written >= (int)(sizeof(methods) - offset)) {
                        break; /* Buffer full */
                    }
                    offset += written;
                }
                
                int written = snprintf(methods + offset, sizeof(methods) - offset, "%s",
                                     config->allowedMethods[i].data);
                if (written < 0 || written >= (int)(sizeof(methods) - offset)) {
                    break;
                }
                offset += written;
            }
            
            addResponseHeader(response, "Access-Control-Allow-Methods", methods);
        }
        
        /* Access-Control-Allow-Headers */
        if (config->headerCount > 0) {
            char headers[1024] = {0};
            size_t offset = 0;
            
            for (int i = 0; i < config->headerCount && offset < sizeof(headers) - 260; i++) {
                if (i > 0) {
                    int written = snprintf(headers + offset, sizeof(headers) - offset, ", ");
                    if (written < 0 || written >= (int)(sizeof(headers) - offset)) {
                        break;
                    }
                    offset += written;
                }
                
                int written = snprintf(headers + offset, sizeof(headers) - offset, "%s",
                                     config->allowedHeaders[i].data);
                if (written < 0 || written >= (int)(sizeof(headers) - offset)) {
                    break;
                }
                offset += written;
            }
            
            addResponseHeader(response, "Access-Control-Allow-Headers", headers);
        }
        
        /* Access-Control-Max-Age */
        char maxAge[16];
        snprintf(maxAge, sizeof(maxAge), "%d", config->maxAge); /* CWE-120: Safe formatting */
        maxAge[sizeof(maxAge) - 1] = '\\0';
        addResponseHeader(response, "Access-Control-Max-Age", maxAge);
    } else {
        /* Access-Control-Expose-Headers for actual requests */
        if (config->exposedHeaderCount > 0) {
            char headers[1024] = {0};
            size_t offset = 0;
            
            for (int i = 0; i < config->exposedHeaderCount && offset < sizeof(headers) - 260; i++) {
                if (i > 0) {
                    int written = snprintf(headers + offset, sizeof(headers) - offset, ", ");
                    if (written < 0 || written >= (int)(sizeof(headers) - offset)) {
                        break;
                    }
                    offset += written;
                }
                
                int written = snprintf(headers + offset, sizeof(headers) - offset, "%s",
                                     config->exposedHeaders[i].data);
                if (written < 0 || written >= (int)(sizeof(headers) - offset)) {
                    break;
                }
                offset += written;
            }
            
            addResponseHeader(response, "Access-Control-Expose-Headers", headers);
        }
    }
}

/* Handle API request with CORS */
void handleRequest(const CORSConfig* config, HTTPResponse* response,
                  const char* method, const char* origin, const char* path) {
    if (config == NULL || response == NULL || method == NULL || path == NULL) {
        return;
    }
    
    /* Initialize response - CWE-665 */
    memset(response, 0, sizeof(HTTPResponse));
    response->statusCode = 0;
    response->headerCount = 0;
    
    size_t methodLen = strnlen(method, MAX_METHOD_LENGTH + 1);
    
    if (!isValidMethod(method, methodLen)) { /* CWE-20: Validate input */
        response->statusCode = 400;
        strncpy(response->body, "Invalid HTTP method", sizeof(response->body) - 1);
        response->body[sizeof(response->body) - 1] = '\\0';
        return;
    }
    
    /* Check if preflight */
    bool isPreflight = (strcmp(method, "OPTIONS") == 0 && origin != NULL && origin[0] != '\\0');
    
    if (isPreflight) {
        response->statusCode = 204;
        generateCORSHeaders(config, response, origin, true);
        return;
    }
    
    /* Handle actual requests */
    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/data") == 0) {
        response->statusCode = 200;
        strncpy(response->body, "{\\"message\\":\\"Success\\",\\"data\\":[1,2,3]}", sizeof(response->body) - 1);
        response->body[sizeof(response->body) - 1] = '\\0';
        addResponseHeader(response, "Content-Type", "application/json");
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/data") == 0) {
        response->statusCode = 201;
        strncpy(response->body, "{\\"message\\":\\"Created\\",\\"id\\":123}", sizeof(response->body) - 1);
        response->body[sizeof(response->body) - 1] = '\\0';
        addResponseHeader(response, "Content-Type", "application/json");
    } else {
        response->statusCode = 404;
        strncpy(response->body, "{\\"error\\":\\"Not found\\"}", sizeof(response->body) - 1);
        response->body[sizeof(response->body) - 1] = '\\0';
        addResponseHeader(response, "Content-Type", "application/json");
    }
    
    /* Add CORS headers if origin present */
    if (origin != NULL && origin[0] != '\\0') {
        generateCORSHeaders(config, response, origin, false);
    }
}

int main(void) {
    printf("CORS-Enabled API Test Cases\\n\\n");
    
    /* Test Case 1: Basic GET request from allowed origin */
    {
        printf("Test 1: GET request from allowed origin\\n");
        CORSConfig config;
        initCORSConfig(&config);
        addAllowedOrigin(&config, "https://example.com");
        addExposedHeader(&config, "X-Custom-Header");
        
        HTTPResponse response;
        handleRequest(&config, &response, "GET", "https://example.com", "/api/data");
        
        printf("Status: %d\\n", response.statusCode);
        printf("CORS Headers:\\n");
        for (int i = 0; i < response.headerCount; i++) {
            printf("  %s: %s\\n", response.headers[i].name, response.headers[i].value);
        }
        printf("\\n");
    }
    
    /* Test Case 2: Preflight OPTIONS request */
    {
        printf("Test 2: Preflight OPTIONS request\\n");
        CORSConfig config;
        initCORSConfig(&config);
        addAllowedOrigin(&config, "https://app.example.com");
        addAllowedMethod(&config, "PUT");
        addAllowedHeader(&config, "Content-Type");
        addAllowedHeader(&config, "Authorization");
        config.maxAge = 3600;
        
        HTTPResponse response;
        handleRequest(&config, &response, "OPTIONS", "https://app.example.com", "/api/data");
        
        printf("Status: %d\\n", response.statusCode);
        printf("CORS Headers:\\n");
        for (int i = 0; i < response.headerCount; i++) {
            printf("  %s: %s\\n", response.headers[i].name, response.headers[i].value);
        }
        printf("\\n");
    }
    
    /* Test Case 3: Request from disallowed origin */
    {
        printf("Test 3: Request from disallowed origin\\n");
        CORSConfig config;
        initCORSConfig(&config);
        addAllowedOrigin(&config, "https://trusted.com");
        
        HTTPResponse response;
        handleRequest(&config, &response, "GET", "https://untrusted.com", "/api/data");
        
        printf("Status: %d\\n", response.statusCode);
        printf("CORS Headers (should be minimal):\\n");
        bool hasCORS = false;
        for (int i = 0; i < response.headerCount; i++) {
            if (strstr(response.headers[i].name, "Access-Control") != NULL) {
                printf("  %s: %s\\n", response.headers[i].name, response.headers[i].value);
                hasCORS = true;
            }
        }
        if (!hasCORS) {
            printf("  (none)\\n");
        }
        printf("\\n");
    }
    
    /* Test Case 4: Wildcard origin */
    {
        printf("Test 4: Wildcard origin configuration\\n");
        CORSConfig config;
        initCORSConfig(&config);
        addAllowedOrigin(&config, "*");
        
        HTTPResponse response;
        handleRequest(&config, &response, "GET", "https://any-origin.com", "/api/data");
        
        printf("Status: %d\\n", response.statusCode);
        printf("CORS Headers:\\n");
        for (int i = 0; i < response.headerCount; i++) {
            printf("  %s: %s\\n", response.headers[i].name, response.headers[i].value);
        }
        printf("\\n");
    }
    
    /* Test Case 5: POST with credentials */
    {
        printf("Test 5: POST with credentials\\n");
        CORSConfig config;
        initCORSConfig(&config);
        addAllowedOrigin(&config, "https://secure.example.com");
        addAllowedHeader(&config, "Content-Type");
        config.allowCredentials = true;
        
        HTTPResponse response;
        handleRequest(&config, &response, "POST", "https://secure.example.com", "/api/data");
        
        printf("Status: %d\\n", response.statusCode);
        printf("CORS Headers:\\n");
        for (int i = 0; i < response.headerCount; i++) {
            printf("  %s: %s\\n", response.headers[i].name, response.headers[i].value);
        }
        printf("\\n");
    }
    
    return 0;
}
