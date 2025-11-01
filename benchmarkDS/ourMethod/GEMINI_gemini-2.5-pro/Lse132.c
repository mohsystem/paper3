#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <regex.h>
#include <microhttpd.h>

#define PORT 8888
#define TEMPLATE_FILE "hello.html"
#define TEMPLATE_PLACEHOLDER "{{username}}"
#define MAX_USERNAME_LEN 50
#define MAX_TEMPLATE_SIZE 1024 // 1 KB limit for the template file

// A simple HTML escaper to prevent XSS.
// Allocates a new string which must be freed by the caller.
char* escape_html(const char* input) {
    if (!input) return NULL;

    size_t input_len = strlen(input);
    // Worst case: every character is replaced by a 6-char entity (e.g., '&quot;')
    char* escaped = malloc(input_len * 6 + 1);
    if (!escaped) return NULL;

    char* p = escaped;
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&': p += sprintf(p, "&amp;"); break;
            case '<': p += sprintf(p, "&lt;"); break;
            case '>': p += sprintf(p, "&gt;"); break;
            case '"': p += sprintf(p, "&quot;"); break;
            case '\'': p += sprintf(p, "&#39;"); break;
            default: *p++ = input[i]; break;
        }
    }
    *p = '\0';
    return escaped;
}

// Renders the final response by replacing the placeholder.
// Allocates a new string that must be freed.
char* render_template(const char* template_content, const char* sanitized_username) {
    const char* placeholder = TEMPLATE_PLACEHOLDER;
    char* found = strstr(template_content, placeholder);
    if (!found) {
        // If placeholder not found, just clone the content
        char* result = strdup(template_content);
        return result;
    }
    
    size_t final_len = strlen(template_content) - strlen(placeholder) + strlen(sanitized_username) + 1;
    char* response_body = malloc(final_len);
    if (!response_body) return NULL;
    
    // Copy content up to the placeholder
    ptrdiff_t head_len = found - template_content;
    memcpy(response_body, template_content, head_len);
    
    // Append the sanitized username
    memcpy(response_body + head_len, sanitized_username, strlen(sanitized_username));
    
    // Append the rest of the template
    strcpy(response_body + head_len + strlen(sanitized_username), found + strlen(placeholder));

    return response_body;
}

enum MHD_Result handle_hello_request(
    struct MHD_Connection* connection,
    const char* username) 
{
    // Rule #1: Validate input
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9_]{1,50}$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return MHD_NO; // Internal server error
    }
    reti = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);

    if (reti != 0) {
        const char *page = "Invalid username format.";
        struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "text/plain");
        int ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Rule #2: Sanitize output to prevent XSS
    char* sanitized_username = escape_html(username);
    if (!sanitized_username) return MHD_NO;

    // Rule #5 & #6: Safely read template file
    FILE* fp = fopen(TEMPLATE_FILE, "r");
    if (!fp) {
        free(sanitized_username);
        return MHD_NO;
    }

    struct stat st;
    if (fstat(fileno(fp), &st) != 0 || !S_ISREG(st.st_mode) || st.st_size > MAX_TEMPLATE_SIZE) {
        fclose(fp);
        free(sanitized_username);
        return MHD_NO;
    }
    
    char* template_content = malloc(st.st_size + 1);
    if (!template_content) {
        fclose(fp);
        free(sanitized_username);
        return MHD_NO;
    }
    
    if (fread(template_content, 1, st.st_size, fp) != (size_t)st.st_size) {
        free(template_content);
        fclose(fp);
        free(sanitized_username);
        return MHD_NO;
    }
    template_content[st.st_size] = '\0';
    fclose(fp);
    
    char* response_body = render_template(template_content, sanitized_username);
    free(template_content);
    free(sanitized_username);
    
    if (!response_body) return MHD_NO;

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_body), (void *)response_body, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "text/html");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    
    return ret;
}

enum MHD_Result answer_to_connection(
    void* cls,
    struct MHD_Connection* connection,
    const char* url,
    const char* method,
    const char* version,
    const char* upload_data,
    size_t* upload_data_size,
    void** con_cls)
{
    (void)cls; (void)version; (void)upload_data; (void)upload_data_size; (void)con_cls;

    if (0 != strcmp(method, "GET")) {
        return MHD_NO;
    }
    
    const char* prefix = "/hello/";
    if (0 == strncmp(url, prefix, strlen(prefix))) {
        const char* username = url + strlen(prefix);
        return handle_hello_request(connection, username);
    }
    
    const char *page = "Not Found";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

void create_template_file() {
    FILE* fp = fopen(TEMPLATE_FILE, "w");
    if (fp) {
        fprintf(fp, "<!DOCTYPE html>\n");
        fprintf(fp, "<html><head><title>Hello</title></head>\n");
        fprintf(fp, "<body><h1>Hello, %s!</h1></body></html>\n", TEMPLATE_PLACEHOLDER);
        fclose(fp);
        printf("Created '%s' template file.\n", TEMPLATE_FILE);
    }
}

int main(void) {
    create_template_file();

    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                                &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start daemon.\n");
        return 1;
    }

    printf("Server starting on http://localhost:%d\n", PORT);
    printf("Try the following 5 test cases in your browser or with curl:\n");
    printf("1. Valid: http://localhost:%d/hello/Alice\n", PORT);
    printf("2. Valid with numbers: http://localhost:%d/hello/user123\n", PORT);
    printf("3. Valid with underscore: http://localhost:%d/hello/test_user\n", PORT);
    printf("4. Invalid characters: http://localhost:%d/hello/bad!user (should return 400 Bad Request)\n", PORT);
    printf("5. XSS attempt: http://localhost:%d/hello/%%3Cscript%%3Ealert(1)%%3C/script%%3E (should display escaped text)\n", PORT);
    printf("Press Enter to quit.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}