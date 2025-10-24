
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_HEADERS 10
#define MAX_ORIGINS 10
#define MAX_METHODS 10
#define MAX_HEADER_TYPES 10
#define MAX_STRING_LENGTH 256

typedef struct {
    char key[MAX_STRING_LENGTH];
    char value[MAX_STRING_LENGTH];
} Header;

typedef struct {
    Header headers[MAX_HEADERS];
    int count;
} HeaderMap;

int is_allowed_origin(const char* origin) {
    const char* allowed[] = {"http://localhost:3000", "http://example.com", "https://example.com"};
    for (int i = 0; i < 3; i++) {
        if (strcmp(origin, allowed[i]) == 0) return 1;
    }
    return 0;
}

int is_allowed_method(const char* method) {
    const char* allowed[] = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
    for (int i = 0; i < 5; i++) {
        if (strcmp(method, allowed[i]) == 0) return 1;
    }
    return 0;
}

void handle_cors(const char* origin, const char* method, const char* request_headers, HeaderMap* headers) {
    headers->count = 0;
    
    if (origin != NULL && strlen(origin) > 0 && is_allowed_origin(origin)) {
        strcpy(headers->headers[headers->count].key, "Access-Control-Allow-Origin");
        strcpy(headers->headers[headers->count].value, origin);
        headers->count++;
    }
    
    strcpy(headers->headers[headers->count].key, "Access-Control-Allow-Methods");
    strcpy(headers->headers[headers->count].value, "GET, POST, PUT, DELETE, OPTIONS");
    headers->count++;
    
    strcpy(headers->headers[headers->count].key, "Access-Control-Allow-Headers");
    strcpy(headers->headers[headers->count].value, "Content-Type, Authorization, X-Requested-With");
    headers->count++;
    
    strcpy(headers->headers[headers->count].key, "Access-Control-Max-Age");
    strcpy(headers->headers[headers->count].value, "3600");
    headers->count++;
    
    strcpy(headers->headers[headers->count].key, "Access-Control-Allow-Credentials");
    strcpy(headers->headers[headers->count].value, "true");
    headers->count++;
}

char* format_response(int status_code, const char* status_text, const HeaderMap* headers, const char* body) {
    char* response = (char*)malloc(4096);
    sprintf(response, "HTTP/1.1 %d %s\\n", status_code, status_text);
    
    for (int i = 0; i < headers->count; i++) {
        char temp[512];
        sprintf(temp, "%s: %s\\n", headers->headers[i].key, headers->headers[i].value);
        strcat(response, temp);
    }
    
    char temp[256];
    sprintf(temp, "Content-Type: application/json\\nContent-Length: %lu\\n\\n", strlen(body));
    strcat(response, temp);
    strcat(response, body);
    
    return response;
}

char* handle_api_request(const char* origin, const char* method, const char* request_headers, const char* path) {
    HeaderMap cors_headers;
    handle_cors(origin, method, request_headers, &cors_headers);
    
    if (strcmp(method, "OPTIONS") == 0) {
        return format_response(204, "No Content", &cors_headers, "");
    }
    
    if (origin != NULL && strlen(origin) > 0 && !is_allowed_origin(origin)) {
        return format_response(403, "Forbidden", &cors_headers, "{\\"error\\": \\"Origin not allowed\\"}");
    }
    
    if (!is_allowed_method(method)) {
        return format_response(405, "Method Not Allowed", &cors_headers, "{\\"error\\": \\"Method not allowed\\"}");
    }
    
    const char* response_body;
    if (strcmp(path, "/api/users") == 0) {
        response_body = "{\\"users\\": [{\\"id\\": 1, \\"name\\": \\"John\\"}, {\\"id\\": 2, \\"name\\": \\"Jane\\"}]}";
    } else if (strcmp(path, "/api/data") == 0) {
        response_body = "{\\"data\\": \\"Sample data from API\\"}";
    } else {
        response_body = "{\\"message\\": \\"API endpoint not found\\"}";
    }
    
    return format_response(200, "OK", &cors_headers, response_body);
}

int main() {
    printf("Test Case 1: Valid GET request from allowed origin\\n");
    char* response1 = handle_api_request("http://localhost:3000", "GET", "Content-Type", "/api/users");
    printf("%s\\n", response1);
    free(response1);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 2: OPTIONS preflight request\\n");
    char* response2 = handle_api_request("http://example.com", "OPTIONS", "Content-Type", "/api/data");
    printf("%s\\n", response2);
    free(response2);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 3: Request from disallowed origin\\n");
    char* response3 = handle_api_request("http://malicious.com", "GET", "Content-Type", "/api/users");
    printf("%s\\n", response3);
    free(response3);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 4: Invalid HTTP method\\n");
    char* response4 = handle_api_request("http://localhost:3000", "TRACE", "Content-Type", "/api/data");
    printf("%s\\n", response4);
    free(response4);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 5: POST request to valid endpoint\\n");
    char* response5 = handle_api_request("https://example.com", "POST", "Content-Type, Authorization", "/api/data");
    printf("%s\\n", response5);
    free(response5);
    
    return 0;
}
