/*
================================================================================
IMPORTANT: This code requires the 'Mongoose' networking library.
It is a single-file library. You can get it from:
https://github.com/cesanta/mongoose

1. Download 'mongoose.c' and 'mongoose.h'.
2. Place them in the same directory as this C source file.
3. Compile all files together and link against pthread (on Linux/macOS).
   Example on Linux/macOS:
   gcc -o Lse65 Lse65.c mongoose.c -I. -lpthread
   Example on Windows (MinGW):
   gcc -o Lse65.exe Lse65.c mongoose.c -I. -lws2_32
================================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mongoose header for networking
#include "mongoose.h"

// Platform-specific includes for threading and file system operations
#ifdef _WIN32
#include <windows.h>
#define THREAD_T HANDLE
#define THREAD_FN_T DWORD WINAPI
#define START_THREAD(p_thread, fn, arg) \
  do { *(p_thread) = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)fn, arg, 0, NULL); } while(0)
#define JOIN_THREAD(thread) WaitForSingleObject(thread, INFINITE)
#define MKDIR(path) _mkdir(path)
#else
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#define THREAD_T pthread_t
#define THREAD_FN_T void*
#define START_THREAD(p_thread, fn, arg) pthread_create(p_thread, NULL, fn, arg)
#define JOIN_THREAD(thread) pthread_join(thread, NULL)
#define MKDIR(path) mkdir(path, 0755)
#endif

static const char *s_server_url = "http://localhost:8083";
static const char *s_upload_dir = "uploads";
static volatile bool s_exit_flag = false;

// --- Server-side Logic ---
// Mongoose event handler function for the server.
static void server_event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        // We only handle uploads to /upload URI
        if (mg_http_match_uri(hm, "/upload")) {
            // Mongoose's built-in function to save uploaded file.
            mg_http_upload(c, hm, s_upload_dir);
            printf("Server: Handled file upload request.\n");
        } else {
            mg_http_reply(c, 404, "", "Not Found\n");
        }
    }
    (void) fn_data;
}

// --- Client-side Logic & Server Polling ---
struct client_upload_data {
    const char *file_path;
    bool done;
    int status_code;
};

// Mongoose event handler for the client connection.
static void client_event_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct client_upload_data *data = (struct client_upload_data *)fn_data;
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        data->status_code = mg_http_status(hm);
        printf("Client Result: Status: %d, Response: %.*s\n", 
               data->status_code, (int)hm->body.len, hm->body.ptr);
        data->done = true; // Signal that the upload is complete
    } else if (ev == MG_EV_ERROR) {
        data->status_code = -1; // Indicate an error
        printf("Client Error: A connection error occurred.\n");
        data->done = true; // Signal completion on error
    }
    (void) c;
}

// Function to perform the upload.
void upload_pdf_file(struct mg_mgr *mgr, const char *file_path) {
    struct client_upload_data data = {file_path, false, 0};
    mg_http_upload(mgr, s_server_url "/upload", NULL, client_event_handler, &data, "file", file_path);

    // Wait for the upload to complete
    while (!data.done && !s_exit_flag) {
        mg_mgr_poll(mgr, 50);
    }
}

// The server's main loop, running in a separate thread.
THREAD_FN_T server_thread_func(void *param) {
    struct mg_mgr *mgr = (struct mg_mgr *)param;
    printf("Server thread started, listening on %s\n", s_server_url);
    while (!s_exit_flag) {
        mg_mgr_poll(mgr, 1000); // Poll for events
    }
    printf("Server thread stopping.\n");
    return 0;
}

// --- Main function with test cases ---
int main(void) {
    struct mg_mgr mgr;
    THREAD_T server_thread;

    // 1. Setup server and start it in a new thread
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, s_server_url, server_event_handler, &mgr);
    START_THREAD(&server_thread, server_thread_func, &mgr);
    
#ifdef _WIN32
    Sleep(100); // Give server a moment to start
#else
    usleep(100000); // Give server a moment to start
#endif

    // 2. Setup test directory and files
    MKDIR(s_upload_dir);

    printf("\n--- Running 5 Test Cases ---\n");
    for (int i = 1; i <= 5; i++) {
        char file_name[32];
        snprintf(file_name, sizeof(file_name), "test_file_%d.pdf", i);
        
        FILE *fp = fopen(file_name, "w");
        if (fp) {
            fprintf(fp, "This is a dummy PDF file content for test %d", i);
            fclose(fp);
        }

        printf("\nTest Case %d: Uploading %s\n", i, file_name);
        upload_pdf_file(&mgr, file_name);
        
        // Verify file exists
        char uploaded_path[256];
        snprintf(uploaded_path, sizeof(uploaded_path), "%s/%s", s_upload_dir, file_name);
        FILE *verify_fp = fopen(uploaded_path, "r");
        if (verify_fp) {
             printf("Verification: SUCCESS - File found in '%s' folder.\n", s_upload_dir);
             fclose(verify_fp);
        } else {
             printf("Verification: FAILED - File not found in '%s' folder.\n", s_upload_dir);
        }
    }
    
    // 4. Signal server to stop and wait for it to exit
    printf("\nShutting down server...\n");
    s_exit_flag = true;
    JOIN_THREAD(server_thread);
    mg_mgr_free(&mgr);
    printf("Server stopped.\n");
    
    // 5. Cleanup
    printf("Cleaning up test files...\n");
    for (int i = 1; i <= 5; i++) {
        char file_name[32];
        snprintf(file_name, sizeof(file_name), "test_file_%d.pdf", i);
        remove(file_name);
    }

    return 0;
}