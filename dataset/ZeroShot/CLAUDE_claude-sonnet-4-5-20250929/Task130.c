
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_HEADERS 20
#define MAX_STRING 256
#define MAX_ORIGINS 10
#define MAX_METHODS 10
#define MAX_ALLOWED_HEADERS 10

typedef struct {
    char key[MAX_STRING];
    char value[MAX_STRING];
} Header;

typedef struct {
    int statusCode;
    Header headers[MAX_HEADERS];
    int headerCount;
    char body[1024];
} APIResponse;

typedef struct {
    char* origins[MAX_ORIGINS];
    int originCount;
    char* methods[MAX_METHODS];
    int methodCount;
    char* allowedHeaders[MAX_ALLOWED_HEADERS];
    int allowedHeaderCount;
    int maxAge;
} CORSConfig;

void initCORSConfig(CORSConfig* config) {
    static char* origins[] = {"https://example.com", "https://app.example.com", "http://localhost:3000"};
    static char* methods[] = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
    static char* allowedHeaders[] = {"Content-Type", "Authorization", "X-Requested-With"};
    
    config->originCount = 3;
    for (int i = 0; i < config->originCount; i++) {
        config->origins[i] = origins[i];
    }
    
    config->methodCount = 5;
    for (int i = 0; i < config->methodCount; i++) {
        config->methods[i] = methods[i];
    }
    
    config->allowedHeaderCount = 3;
    for (int i = 0; i < config->allowedHeaderCount; i++) {
        config->allowedHeaders[i] = allowedHeaders[i];
    }
    
    config->maxAge = 3600;
}

void initAPIResponse(APIResponse* response, int statusCode) {
    response->statusCode = statusCode;
    response->headerCount = 0;
    response->body[0] = '\\0';
}

void setHeader(APIResponse* response, const char* key, const char* value) {
    if (response->headerCount < MAX_HEADERS) {
        strncpy(response->headers[response->headerCount].key, key, MAX_STRING - 1);
        strncpy(response->headers[response->headerCount].value, value, MAX_STRING - 1);
        response->headerCount++;
    }
}

void setBody(APIResponse* response, const char* body) {
    strncpy(response->body, body, 1023);
    response->body[1023] = '\\0';
}

int isOriginAllowed(const CORSConfig* config, const char* origin) {
    for (int i = 0; i < config->originCount; i++) {
        if (strcmp(config->origins[i], origin) == 0) {
            return 1;
        }
    }
    return 0;
}

int isMethodAllowed(const CORSConfig* config, const char* method) {
    for (int i = 0; i < config->methodCount; i++) {
        if (strcmp(config->methods[i], method) == 0) {
            return 1;
        }
    }
    return 0;
}

void joinStrings(char* result, char** strings, int count, const char* delimiter) {
    result[0] = '\\0';
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(result, delimiter);
        strcat(result, strings[i]);
    }
}

APIResponse handleCORS(const CORSConfig* config, const char* origin, const char* method) {
    APIResponse response;
    initAPIResponse(&response, 200);
    
    if (origin && strlen(origin) > 0) {
        if (isOriginAllowed(config, origin)) {
            setHeader(&response, "Access-Control-Allow-Origin", origin);
            setHeader(&response, "Access-Control-Allow-Credentials", "true");
            setHeader(&response, "Vary", "Origin");
        } else {
            response.statusCode = 403;
            setBody(&response, "{\\"error\\": \\"Origin not allowed\\"}");
            return response;
        }
    }
    
    if (strcmp(method, "OPTIONS") == 0) {
        char methodsStr[MAX_STRING];
        char headersStr[MAX_STRING];
        char maxAgeStr[20];
        
        joinStrings(methodsStr, config->methods, config->methodCount, ", ");
        joinStrings(headersStr, config->allowedHeaders, config->allowedHeaderCount, ", ");
        sprintf(maxAgeStr, "%d", config->maxAge);
        
        setHeader(&response, "Access-Control-Allow-Methods", methodsStr);
        setHeader(&response, "Access-Control-Allow-Headers", headersStr);
        setHeader(&response, "Access-Control-Max-Age", maxAgeStr);
        response.statusCode = 204;
    } else {
        if (!isMethodAllowed(config, method)) {
            response.statusCode = 405;
            setBody(&response, "{\\"error\\": \\"Method not allowed\\"}");
            return response;
        }
        setHeader(&response, "Access-Control-Expose-Headers", "Content-Length, X-Request-ID");
    }
    
    return response;
}

APIResponse handleAPIRequest(const CORSConfig* config, const char* origin, const char* method, const char* path) {
    APIResponse corsResponse = handleCORS(config, origin, method);
    
    if (corsResponse.statusCode != 200 && corsResponse.statusCode != 204) {
        return corsResponse;
    }
    
    if (strcmp(method, "GET") == 0 && strcmp(path, "/api/data") == 0) {
        setBody(&corsResponse, "{\\"message\\": \\"Data retrieved successfully\\", \\"data\\": [1, 2, 3]}");
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/api/data") == 0) {
        setBody(&corsResponse, "{\\"message\\": \\"Data created successfully\\", \\"id\\": 123}");
    } else {
        corsResponse.statusCode = 404;
        setBody(&corsResponse, "{\\"error\\": \\"Endpoint not found\\"}");
    }
    
    setHeader(&corsResponse, "Content-Type", "application/json");
    return corsResponse;
}

void printAPIResponse(const APIResponse* response) {
    printf("Status Code: %d\\n", response->statusCode);
    printf("Headers:\\n");
    for (int i = 0; i < response->headerCount; i++) {
        printf("  %s: %s\\n", response->headers[i].key, response->headers[i].value);
    }
    printf("Body: %s\\n", response->body);
}

int main() {
    CORSConfig config;
    initCORSConfig(&config);
    
    printf("=== CORS API Implementation Test Cases ===\\n\\n");
    
    printf("Test Case 1: Preflight OPTIONS request from allowed origin\\n");
    APIResponse response1 = handleAPIRequest(&config, "https://example.com", "OPTIONS", "/api/data");
    printAPIResponse(&response1);
    printf("\\n============================================================\\n\\n");
    
    printf("Test Case 2: GET request from allowed origin\\n");
    APIResponse response2 = handleAPIRequest(&config, "https://example.com", "GET", "/api/data");
    printAPIResponse(&response2);
    printf("\\n============================================================\\n\\n");
    
    printf("Test Case 3: POST request from allowed origin\\n");
    APIResponse response3 = handleAPIRequest(&config, "http://localhost:3000", "POST", "/api/data");
    printAPIResponse(&response3);
    printf("\\n============================================================\\n\\n");
    
    printf("Test Case 4: Request from disallowed origin\\n");
    APIResponse response4 = handleAPIRequest(&config, "https://malicious.com", "GET", "/api/data");
    printAPIResponse(&response4);
    printf("\\n============================================================\\n\\n");
    
    printf("Test Case 5: Request with disallowed method\\n");
    APIResponse response5 = handleAPIRequest(&config, "https://example.com", "PATCH", "/api/data");
    printAPIResponse(&response5);
    printf("\\n============================================================\\n\\n");
    
    return 0;
}
