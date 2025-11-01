/*
 * INSTRUCTIONS TO COMPILE AND RUN:
 * 1. This code requires the 'libmicrohttpd' library.
 * 2. Install the library. On Debian/Ubuntu:
 *    sudo apt-get update
 *    sudo apt-get install libmicrohttpd-dev
 * 3. Compile the code, linking against the library:
 *    gcc -o Lse41 Lse41.c -lmicrohttpd
 * 4. Run the executable:
 *    ./Lse41
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>

#define PORT 8888

// Function to generate and serve the /info page
int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    
    struct MHD_Response *response;
    int ret;

    // We only handle GET requests to /info
    if (0 != strcmp(method, "GET") || 0 != strcmp(url, "/info")) {
        const char *page = "<html><body>Not Found</body></html>";
        response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Hardcoded user data
    const char *username = "johndoe";
    const char *email = "j.doe@example.com";
    const char *ssn_last_4 = "5678";

    // Create the HTML response string
    char content[512];
    snprintf(content, sizeof(content),
             "<html>"
             "<head><title>User Information</title></head>"
             "<body>"
             "<h1>User Information</h1>"
             "<ul>"
             "<li><strong>Username:</strong> %s</li>"
             "<li><strong>Email:</strong> %s</li>"
             "<li><strong>SSN (Last 4 digits):</strong> %s</li>"
             "</ul>"
             "</body>"
             "</html>",
             username, email, ssn_last_4);
             
    response = MHD_create_response_from_buffer(strlen(content), (void *)content, MHD_RESPMEM_MUST_COPY);
    if (!response) {
        return MHD_NO;
    }
    
    MHD_add_response_header(response, "Content-Type", "text/html");
    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start the server on port %d\n", PORT);
        return 1;
    }
    
    printf("C server started on port %d\n", PORT);
    printf("Visit http://localhost:%d/info to see the user information.\n", PORT);

    // Test cases (instructions for manual testing)
    printf("\n--- How to Test ---\n");
    printf("Test Case 1: Open a web browser and navigate to http://localhost:8888/info\n");
    printf("Test Case 2: Use curl in your terminal: curl http://localhost:8888/info\n");
    printf("Test Case 3: Use a tool like Postman to send a GET request to http://localhost:8888/info\n");
    printf("Test Case 4: Use PowerShell Invoke-WebRequest: Invoke-WebRequest -Uri http://localhost:8888/info\n");
    printf("Test Case 5: Use Python requests library: python -c \"import requests; print(requests.get('http://localhost:8888/info').text)\"\n");


    printf("\nPress Enter to stop the server...\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}