/*
 * Writing a complete, single-file web application with robust CSRF protection in C
 * is highly impractical and not recommended. C lacks standard libraries for core web
 * development tasks such as:
 * - HTTP server implementation
 * - HTTP request/response parsing
 * - Session management (cookies, server-side storage)
 * - Secure random number generation for tokens
 * - HTML templating
 *
 * Such a task requires external libraries (e.g., libmicrohttpd, OpenSSL) and a
 * full project structure, not a single source file.
 *
 * The code below is purely a conceptual illustration of the required logic
 * and is NOT runnable or complete. It demonstrates the flow of a CSRF check
 * in a hypothetical C web application environment.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// --- HYPOTHETICAL WEB FRAMEWORK STRUCTURES AND FUNCTIONS ---
typedef struct {
    char method[8]; // "GET" or "POST"
    // In reality, this would be a more complex structure (e.g., hash map)
    char form_data[1024]; 
} HttpRequest;

typedef struct {
    // In reality, this would be a server-side storage mechanism
    char csrf_token[65];
} HttpSession;

// Hypothetical function to get a value from form data
const char* get_form_value(const HttpRequest* req, const char* key) {
    // Dummy implementation
    if (strcmp(key, "csrf_token") == 0 && strstr(req->form_data, "csrf_token=")) {
        return strstr(req->form_data, "csrf_token=") + 11;
    }
    return NULL;
}

// Hypothetical function to generate a secure random token
void generate_csrf_token(char* buffer, size_t size) {
    // WARNING: This is NOT cryptographically secure. For demonstration only.
    // A real implementation MUST use a cryptographic library like OpenSSL's RAND_bytes.
    srand(time(NULL));
    for (size_t i = 0; i < size - 1; ++i) {
        sprintf(buffer + (i * 2), "%02x", rand() % 256);
    }
    buffer[size-1] = '\0';
}
// --- END OF HYPOTHETICAL CODE ---

void handle_get_request(HttpSession* session) {
    printf("--- Handling GET Request ---\n");
    // 1. Generate a new CSRF token
    generate_csrf_token(session->csrf_token, sizeof(session->csrf_token));
    
    // 2. In a real app, store token in session storage
    printf("Generated CSRF Token: %s\n", session->csrf_token);
    
    // 3. Render the HTML form with the token
    printf("Rendering HTML form with hidden token field...\n");
    printf("<form method='POST'>\n");
    printf("  <input type='hidden' name='csrf_token' value='%s'>\n", session->csrf_token);
    printf("  <input type='submit' value='Submit'>\n");
    printf("</form>\n\n");
}

void handle_post_request(HttpSession* session, const HttpRequest* request) {
    printf("--- Handling POST Request ---\n");
    
    // 4. Get tokens from form and session
    const char* form_token = get_form_value(request, "csrf_token");
    const char* session_token = session->csrf_token;
    
    printf("Session Token: %s\n", session_token);
    printf("Form Token   : %s\n", form_token ? form_token : "MISSING");

    // 5. Validate the token
    if (form_token == NULL || strcmp(session_token, form_token) != 0) {
        printf("RESULT: CSRF VALIDATION FAILED! Request rejected (403 Forbidden).\n\n");
        return;
    }

    // 6. Invalidate the token after use
    session->csrf_token[0] = '\0';
    printf("Token validated and invalidated.\n");
    
    printf("RESULT: CSRF VALIDATION PASSED! Processing request...\n\n");
}


int main() {
    printf("This is a conceptual C example simulating a web request lifecycle.\n");
    printf("It cannot run as a real web server.\n\n");

    // This main function simulates the "5 test cases"
    HttpSession user_session;
    HttpRequest request;
    
    // --- Test Case 1 & 2: Valid Submission ---
    printf("--- Test Case 1 & 2: Normal Load and Valid Submission ---\n");
    handle_get_request(&user_session); // User loads the page, gets a token
    strcpy(request.method, "POST");
    sprintf(request.form_data, "email=test@example.com&csrf_token=%s", user_session.csrf_token);
    handle_post_request(&user_session, &request); // User submits with correct token

    // --- Test Case 3: Invalid Token Submission ---
    printf("--- Test Case 3: Invalid Token Submission ---\n");
    handle_get_request(&user_session); // User gets a new token
    strcpy(request.method, "POST");
    strcpy(request.form_data, "email=test@example.com&csrf_token=thisisafaketoken");
    handle_post_request(&user_session, &request);

    // --- Test Case 4: Missing Token Submission ---
    printf("--- Test Case 4: Missing Token Submission ---\n");
    handle_get_request(&user_session); // User gets a new token
    strcpy(request.method, "POST");
    strcpy(request.form_data, "email=test@example.com"); // No token
    handle_post_request(&user_session, &request);
    
    // --- Test Case 5: Token Replay Attack ---
    printf("--- Test Case 5: Token Replay Attack ---\n");
    handle_get_request(&user_session); // User gets token
    sprintf(request.form_data, "email=test@example.com&csrf_token=%s", user_session.csrf_token);
    handle_post_request(&user_session, &request); // First submission is valid
    printf("-> User tries to submit the SAME form again (replay)...\n");
    handle_post_request(&user_session, &request); // Second submission should fail as token is now invalid

    return 0;
}