// Instructions:
// 1. This code uses the Mongoose web server library. Download `mongoose.h` and `mongoose.c`
//    from https://github.com/cesanta/mongoose and place them in the same directory.
// 2. Compile with: gcc -o server main.c mongoose.c -I. -lpthread -ldl
// 3. Create a directory named `images` in the same folder as the executable.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "mongoose.h"

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h> // For realpath on POSIX
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static const char *s_http_addr = "http://0.0.0.0:8000";
static const char *s_root_dir = "images";
static char s_abs_root_dir[PATH_MAX];

// Event handler function for the Mongoose web server
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Check if the URL starts with /images/. This is our "decorator".
        if (mg_http_match_uri(hm, "/images/*")) {
            // Extract the filename part of the URI
            // e.g., for URI "/images/cat.jpg", we get "cat.jpg"
            char req_filename[PATH_MAX];
            const char* uri_ptr = hm->uri.ptr + strlen("/images/");
            mg_url_decode(uri_ptr, hm->uri.len - strlen("/images/"), req_filename, sizeof(req_filename), 0);

            // --- Security Checks ---
            // 1. Sanitize: Reject any paths containing ".." or "/" or "\"
            if (strstr(req_filename, "..") != NULL || strchr(req_filename, '/') != NULL || strchr(req_filename, '\\') != NULL) {
                mg_http_reply(c, 400, "", "Bad Request: Invalid filename\n");
                return;
            }

            // 2. Construct full path safely
            char file_path[PATH_MAX];
            int n = snprintf(file_path, sizeof(file_path), "%s/%s", s_root_dir, req_filename);
            if (n < 0 || (size_t)n >= sizeof(file_path)) {
                mg_http_reply(c, 500, "", "Internal Error: Path too long\n");
                return;
            }
            
            // 3. Resolve to a canonical path to prevent symlink and other tricks
            char resolved_path[PATH_MAX];
            #ifdef _WIN32
            if (_fullpath(resolved_path, file_path, sizeof(resolved_path)) == NULL) {
                 mg_http_reply(c, 404, "", "Not Found\n");
                 return;
            }
            #else
            if (realpath(file_path, resolved_path) == NULL) {
                mg_http_reply(c, 404, "", "Not Found\n");
                return;
            }
            #endif

            // 4. Final check: Ensure the resolved path is within our root directory
            if (strncmp(resolved_path, s_abs_root_dir, strlen(s_abs_root_dir)) != 0) {
                mg_http_reply(c, 403, "", "Forbidden\n");
                return;
            }
            
            // Use Mongoose's secure file server function on the validated path
            struct mg_http_serve_opts opts = {.root_dir = "."}; // Serve relative to current dir
            mg_http_serve_file(c, hm, resolved_path, &opts);
        } else {
            mg_http_reply(c, 404, "", "Not Found: Try a URL like /images/somefile.txt\n");
        }
    }
    (void) fn_data;
}


void setup_test_environment() {
    mkdir(s_root_dir, 0755);
    char filepath[256];
    
    snprintf(filepath, sizeof(filepath), "%s/%s", s_root_dir, "test1.txt");
    FILE *f1 = fopen(filepath, "w");
    if (f1) {
        fprintf(f1, "This is a test file.");
        fclose(f1);
    }
    
    snprintf(filepath, sizeof(filepath), "%s/%s", s_root_dir, "test2.jpg");
    FILE *f2 = fopen(filepath, "w");
    if (f2) {
        fprintf(f2, "This is a fake jpeg.");
        fclose(f2);
    }
}


int main(void) {
    setup_test_environment();

    // Get the absolute path of our root directory for security checks
    #ifdef _WIN32
    if (_fullpath(s_abs_root_dir, s_root_dir, sizeof(s_abs_root_dir)) == NULL) {
        fprintf(stderr, "Error resolving absolute path for %s\n", s_root_dir);
        return 1;
    }
    #else
    if (realpath(s_root_dir, s_abs_root_dir) == NULL) {
        perror("realpath");
        fprintf(stderr, "Error resolving absolute path for %s\n", s_root_dir);
        return 1;
    }
    #endif

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    printf("Starting server on %s\n", s_http_addr);
    
    printf("Test cases:\n");
    printf("1. Valid file: %s/images/test1.txt\n", s_http_addr);
    printf("2. Valid file with extension: %s/images/test2.jpg\n", s_http_addr);
    printf("3. Non-existent file: %s/images/nonexistent.png\n", s_http_addr);
    printf("4. Path traversal attempt (should be 400): %s/images/../main.c\n", s_http_addr);
    printf("5. Path traversal with encoding (should be 400): %s/images/%%2e%%2e/main.c\n", s_http_addr);
    
    mg_http_listen(&mgr, s_http_addr, fn, NULL);
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
    return 0;
}