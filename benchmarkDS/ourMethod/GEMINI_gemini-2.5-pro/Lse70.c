/*
 * This code requires libmicrohttpd, a small C library for embedding an HTTP server.
 *
 * Installation (Debian/Ubuntu):
 * sudo apt-get install libmicrohttpd-dev
 *
 * Compilation command:
 * gcc -o server server.c -lmicrohttpd
 *
 * The code implements a file upload handler that saves files to an 'uploads' directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <microhttpd.h>

#define PORT 8080
#define UPLOADS_DIR "uploads"
#define MAX_FILENAME_LEN 256
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

// Holds information about an ongoing upload connection.
struct ConnectionInfo {
    FILE *file_handle;
    char filename[MAX_FILENAME_LEN];
    long long total_bytes;
    int error;
};

// Sanitizes a filename by removing directory paths.
// Returns a pointer to the sanitized name within the original buffer.
static const char* sanitize_filename(const char* filename) {
    if (filename == NULL) {
        return NULL;
    }
    // Find the last occurrence of a path separator
    const char* last_slash = strrchr(filename, '/');
    const char* last_backslash = strrchr(filename, '\\');
    
    const char* basename = filename;
    if (last_slash && last_slash > basename) {
        basename = last_slash + 1;
    }
    if (last_backslash && last_backslash > basename) {
        basename = last_backslash + 1;
    }

    // Basic validation
    if (strlen(basename) == 0 || strcmp(basename, ".") == 0 || strcmp(basename, "..") == 0) {
        return NULL; // Invalid filename
    }
    
    return basename;
}


static int send_response(struct MHD_Connection *connection, int status_code, const char *page) {
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    if (!response) {
        return MHD_NO;
    }
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}


// Iterator for processing POST data.
static int iterate_post(void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
                        const char *filename, const char *content_type,
                        const char *transfer_encoding, const char *data, uint64_t off,
                        size_t size) {
    struct ConnectionInfo *con_info = coninfo_cls;

    if (con_info->error) {
        return MHD_NO; // Stop processing if an error occurred
    }

    // We only care about the 'file' form field
    if (0 != strcmp(key, "file")) {
        return MHD_YES;
    }

    if (filename != NULL && con_info->file_handle == NULL) {
        // This is the start of a new file upload
        const char *sanitized = sanitize_filename(filename);
        if (!sanitized) {
            con_info->error = 1;
            return MHD_NO;
        }
        
        snprintf(con_info->filename, MAX_FILENAME_LEN, "%s/%s", UPLOADS_DIR, sanitized);
        
        // Use open with O_CREAT | O_EXCL to safely create the file, preventing overwrites and TOCTOU
        int fd = open(con_info->filename, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            // File likely exists or another error occurred
            con_info->error = 1;
            return MHD_NO;
        }
        
        con_info->file_handle = fdopen(fd, "wb");
        if (con_info->file_handle == NULL) {
             close(fd);
             con_info->error = 1;
             return MHD_NO;
        }
        con_info->total_bytes = 0;
    }

    if (size > 0) {
        if (!con_info->file_handle) {
            return MHD_YES; // Not our file, or an error occurred
        }
        
        con_info->total_bytes += size;
        if (con_info->total_bytes > MAX_FILE_SIZE) {
            // File too large, clean up and set error
            fclose(con_info->file_handle);
            con_info->file_handle = NULL;
            remove(con_info->filename); // Delete partial file
            con_info->error = 2; // Specific error code for "too large"
            return MHD_NO;
        }
        
        if (fwrite(data, 1, size, con_info->file_handle) != size) {
            // Write error
            fclose(con_info->file_handle);
            con_info->file_handle = NULL;
            remove(con_info->filename);
            con_info->error = 1;
            return MHD_NO;
        }
    }

    return MHD_YES;
}

// Cleanup function when a request is completed.
static void request_completed(void *cls, struct MHD_Connection *connection,
                              void **con_cls, enum MHD_RequestTerminationCode toe) {
    struct ConnectionInfo *con_info = *con_cls;
    if (NULL == con_info) {
        return;
    }
    if (con_info->file_handle) {
        fclose(con_info->file_handle);
    }
    free(con_info);
    *con_cls = NULL;
}


// Main handler for incoming connections.
static int answer_to_connection(void *cls, struct MHD_Connection *connection,
                                const char *url, const char *method,
                                const char *version, const char *upload_data,
                                size_t *upload_data_size, void **con_cls) {
    if (NULL == *con_cls) {
        // First call for this connection, set up state
        if (0 == strcmp(method, "POST") && 0 == strcmp(url, "/upload")) {
            struct ConnectionInfo *con_info = malloc(sizeof(struct ConnectionInfo));
            if (NULL == con_info) return MHD_NO;
            
            memset(con_info, 0, sizeof(struct ConnectionInfo));
            
            MHD_PostProcessor *pp = MHD_create_post_processor(connection, 1024, iterate_post, (void *)con_info);
            if (NULL == pp) {
                free(con_info);
                return MHD_NO;
            }
            
            *con_cls = (void *)pp;
            return MHD_YES;
        } else {
            // Not a valid upload request
            return send_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, "Use POST to /upload");
        }
    }

    if (0 == strcmp(method, "POST")) {
        MHD_PostProcessor *pp = *con_cls;
        if (*upload_data_size != 0) {
            MHD_post_process(pp, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        } else {
            // POST processing finished
            struct ConnectionInfo *con_info = NULL;
            MHD_get_post_processor_data(pp, (void**)&con_info);
            MHD_destroy_post_processor(pp);
            *con_cls = NULL; // Mark as processed
            
            if (con_info && con_info->error) {
                if (con_info->error == 2) {
                     return send_response(connection, MHD_HTTP_PAYLOAD_TOO_LARGE, "File too large");
                }
                return send_response(connection, MHD_HTTP_BAD_REQUEST, "Upload failed");
            }

            return send_response(connection, MHD_HTTP_OK, "File uploaded successfully");
        }
    }

    return send_response(connection, MHD_HTTP_BAD_REQUEST, "Invalid request");
}

int main() {
    // Create uploads directory if it doesn't exist
    struct stat st = {0};
    if (stat(UPLOADS_DIR, &st) == -1) {
        mkdir(UPLOADS_DIR, 0700);
    }

    struct MHD_Daemon *daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                                &answer_to_connection, NULL,
                                                MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL,
                                                MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server running on port %d\n", PORT);
    printf("Uploads will be saved to '%s' directory.\n\n", UPLOADS_DIR);
    printf("--- How to Test with cURL ---\n");
    printf("1. Create a test file: echo 'c test content' > test_c.txt\n");
    printf("2. Valid file upload: curl -F \"file=@test_c.txt\" http://localhost:%d/upload\n", PORT);
    printf("3. File too large: Create a file > 10MB and try to upload it.\n");
    printf("4. Path traversal: curl -F \"file=@test_c.txt;filename=../../test_c.txt\" http://localhost:%d/upload\n", PORT);
    printf("5. Overwrite attempt: curl -F \"file=@test_c.txt\" http://localhost:%d/upload (should fail after first success)\n", PORT);

    (void)getchar(); // Wait for user input to stop the server

    MHD_stop_daemon(daemon);
    return 0;
}