// To compile: gcc -o redirect_server_c redirect_server_c.c -lmicrohttpd -lcurl
// You need to have libmicrohttpd and libcurl installed.
// For Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev libcurl4-openssl-dev

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // For strcasecmp
#include <microhttpd.h>
#include <curl/curl.h>

#define PORT 8080
#define ALLOWED_DOMAIN "example.com"

// Forward declaration
int redirect_invalid_url(struct MHD_Connection *connection, const char *url_to_check);

int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    (void)cls; (void)version; (void)upload_data; (void)upload_data_size; (void)con_cls;

    if (0 != strcmp(method, "GET")) {
        return MHD_NO; // Only GET is allowed
    }

    if (0 == strcmp(url, "/redirect")) {
        const char *url_param = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "url");
        return redirect_invalid_url(connection, url_param);
    }

    return MHD_NO; // Not found
}

int redirect_invalid_url(struct MHD_Connection *connection, const char *url_to_check) {
    struct MHD_Response *response;
    int ret;

    if (url_to_check == NULL) {
        const char *error_msg = "URL parameter is missing.";
        response = MHD_create_response_from_buffer(strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) return MHD_NO;
        ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
        MHD_destroy_response(response);
        return ret;
    }

    CURLU *h = NULL;
    CURLUcode rc;
    char *host = NULL;
    char *scheme = NULL;
    int is_valid = 0;

    h = curl_url();
    if (!h) {
        // Handle curl handle creation failure
        const char *error_msg = "Internal server error.";
        response = MHD_create_response_from_buffer(strlen(error_msg), (void*)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) return MHD_NO;
        ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Rule #1: Use a robust library to parse and validate input URL
    rc = curl_url_set(h, CURLUPART_URL, url_to_check, 0);
    if (rc == CURLE_OK) {
        rc = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
        if (rc == CURLE_OK && scheme != NULL && (strcasecmp(scheme, "http") == 0 || strcasecmp(scheme, "https") == 0)) {
            rc = curl_url_get(h, CURLUPART_HOST, &host, 0);
            if (rc == CURLE_OK && host != NULL && strcasecmp(host, ALLOWED_DOMAIN) == 0) {
                is_valid = 1;
            }
        }
    }
    
    if (is_valid) {
        response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
            is_valid = 0; // Fall through to error handling
        } else {
             MHD_add_response_header(response, "Location", url_to_check);
             ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response);
             MHD_destroy_response(response);
        }
    } 
    
    if (!is_valid) {
        const char *error_msg = "Invalid URL or domain not allowed.";
        response = MHD_create_response_from_buffer(strlen(error_msg), (void *)error_msg, MHD_RESPMEM_PERSISTENT);
        if (response == NULL) {
             ret = MHD_NO;
        } else {
             ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
             MHD_destroy_response(response);
        }
    }

    // Cleanup
    curl_free(host);
    curl_free(scheme);
    curl_url_cleanup(h);

    return ret;
}


int main(void) {
    struct MHD_Daemon *daemon;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start the server\n");
        curl_global_cleanup();
        return 1;
    }

    printf("Server started on port %d\n", PORT);
    printf("--- Test Cases ---\n");
    printf("1. Valid Redirect: curl -v \"http://localhost:8080/redirect?url=http%%3A%%2F%%2Fexample.com%%2Fsome%%2Fpath\"\n");
    printf("2. Valid Redirect (HTTPS): curl -v \"http://localhost:8080/redirect?url=https%%3A%%2F%%2Fexample.com\"\n");
    printf("3. Invalid Domain: curl -v \"http://localhost:8080/redirect?url=http%%3A%%2F%%2Fgoogle.com\"\n");
    printf("4. Invalid Subdomain Attack: curl -v \"http://localhost:8080/redirect?url=http%%3A%%2F%%2Fexample.com.evil.com\"\n");
    printf("5. Malformed URL: curl -v \"http://localhost:8080/redirect?url=not_a_valid_url\"\n");
    
    printf("Press Enter to exit.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    curl_global_cleanup();

    return 0;
}