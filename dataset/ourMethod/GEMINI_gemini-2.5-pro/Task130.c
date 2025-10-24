#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_HEADERS_SIZE 512
#define MAX_BODY_SIZE 256
#define NUM_ALLOWED_ORIGINS 2

// Whitelist of allowed origins.
const char* ALLOWED_ORIGINS[NUM_ALLOWED_ORIGINS] = {
    "https://example.com",
    "https://trusted.site.org"
};
// Define allowed methods and headers for preflight requests.
const char* ALLOWED_METHODS = "GET, POST, OPTIONS";
const char* ALLOWED_HEADERS = "Content-Type, Authorization";
const char* MAX_AGE = "86400"; // 24 hours

// Represents a simplified HTTP Response with fixed-size buffers to prevent overflows
typedef struct {
    int status_code;
    char headers[MAX_HEADERS_SIZE];
    char body[MAX_BODY_SIZE];
} Response;

// Helper function to check if the provided origin is in the whitelist.
bool is_origin_allowed(const char* origin_header) {
    if (origin_header == NULL) {
        return false;
    }
    for (int i = 0; i < NUM_ALLOWED_ORIGINS; ++i) {
        if (strcmp(ALLOWED_ORIGINS[i], origin_header) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Handles a request by applying CORS rules and populating a Response struct.
 * This function simulates a server-side API endpoint's CORS logic.
 *
 * @param res Pointer to the Response struct to be populated.
 * @param method The HTTP method (e.g., "GET", "OPTIONS"). Cannot be NULL.
 * @param origin_header The value of the "Origin" header. Can be NULL.
 */
void handle_cors_request(Response* res, const char* method, const char* origin_header) {
    // Initialize response struct to a known state
    res->status_code = 0;
    res->headers[0] = '\0';
    res->body[0] = '\0';

    // Rule #3: Validate input. The origin must be from the predefined whitelist.
    bool origin_allowed = is_origin_allowed(origin_header);

    if (!origin_allowed) {
        // If the origin is not allowed, or no origin header was provided,
        // do not add any CORS headers and return an error.
        res->status_code = 403;
        snprintf(res->body, MAX_BODY_SIZE, "{\"error\": \"CORS policy does not allow this origin.\"}");
        return;
    }

    // The origin is allowed, add the primary CORS response header.
    // Rule #5: Use snprintf for safe string formatting into fixed-size buffers.
    int offset = 0;
    offset += snprintf(res->headers + offset, MAX_HEADERS_SIZE - offset, "Access-Control-Allow-Origin: %s\n", origin_header);
    
    // Handle preflight (OPTIONS) request
    if (strcmp(method, "OPTIONS") == 0) {
        res->status_code = 204; // No Content
        offset += snprintf(res->headers + offset, MAX_HEADERS_SIZE - offset, "Access-Control-Allow-Methods: %s\n", ALLOWED_METHODS);
        offset += snprintf(res->headers + offset, MAX_HEADERS_SIZE - offset, "Access-Control-Allow-Headers: %s\n", ALLOWED_HEADERS);
        snprintf(res->headers + offset, MAX_HEADERS_SIZE - offset, "Access-Control-Max-Age: %s\n", MAX_AGE);
        // Body is empty for 204 response
        return;
    }

    // Handle actual API requests (e.g., GET, POST)
    if (strcmp(method, "GET") == 0 || strcmp(method, "POST") == 0) {
        res->status_code = 200;
        snprintf(res->headers + offset, MAX_HEADERS_SIZE - offset, "Content-Type: application/json\n");
        snprintf(res->body, MAX_BODY_SIZE, "{\"data\": \"some protected data\"}");
        return;
    }

    // If the method is not OPTIONS, GET, or POST, it's not allowed.
    // The Access-Control-Allow-Origin header is still sent because the origin was valid.
    res->status_code = 405;
    snprintf(res->body, MAX_BODY_SIZE, "{\"error\": \"Method not allowed.\"}");
}

void print_response(const Response* res) {
    printf("Status: %d\n", res->status_code);
    printf("Headers:\n");
    if (strlen(res->headers) == 0) {
        printf("  (none)\n");
    } else {
        printf("%s", res->headers);
    }
    printf("Body:\n%s\n", res->body);
}

int main() {
    Response response;

    printf("--- Test Case 1: Valid GET request from an allowed origin ---\n");
    handle_cors_request(&response, "GET", "https://example.com");
    print_response(&response);
    printf("\n----------------------------------------------------------\n\n");

    printf("--- Test Case 2: Valid Preflight (OPTIONS) request from an allowed origin ---\n");
    handle_cors_request(&response, "OPTIONS", "https://trusted.site.org");
    print_response(&response);
    printf("\n----------------------------------------------------------\n\n");

    printf("--- Test Case 3: Request from a disallowed origin ---\n");
    handle_cors_request(&response, "GET", "https://malicious.com");
    print_response(&response);
    printf("\n----------------------------------------------------------\n\n");

    printf("--- Test Case 4: Request with a missing Origin header ---\n");
    handle_cors_request(&response, "GET", NULL);
    print_response(&response);
    printf("\n----------------------------------------------------------\n\n");

    printf("--- Test Case 5: Request with a disallowed method (PUT) from an allowed origin ---\n");
    handle_cors_request(&response, "PUT", "https://example.com");
    print_response(&response);
    printf("\n----------------------------------------------------------\n\n");

    return 0;
}