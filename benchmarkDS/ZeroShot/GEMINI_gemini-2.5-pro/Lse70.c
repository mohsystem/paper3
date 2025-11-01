// To compile and run this code:
// 1. You need a C compiler (like gcc or clang).
// 2. You need the Mongoose library. Download mongoose.c and mongoose.h from https://github.com/cesanta/mongoose
// 3. Compile this file along with mongoose.c.
//    Example: gcc -o Lse70 Lse70.c mongoose.c -I. -W -Wall -Wextra -lpthread
// 4. Run the executable: ./Lse70
// 5. The server will start on http://0.0.0.0:8000

#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILENAME_LEN 256
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

static const char *s_allowed_extensions[] = {"txt", "jpg", "jpeg", "png", "pdf", NULL};

struct upload_state {
    char file_name[MAX_FILENAME_LEN];
    long long file_size;
    bool error;
    char error_message[128];
};

static const char *get_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

static bool is_extension_allowed(const char *ext) {
    for (int i = 0; s_allowed_extensions[i] != NULL; i++) {
        if (strcasecmp(ext, s_allowed_extensions[i]) == 0) return true;
    }
    return false;
}

// Sanitize filename by removing directory separators. Returns only the basename.
static const char *sanitize_filename(const char *filename) {
    const char *last_slash = strrchr(filename, '/');
    const char *last_bslash = strrchr(filename, '\\');
    const char *basename = filename;
    if (last_slash && last_slash > basename) basename = last_slash + 1;
    if (last_bslash && last_bslash > basename) basename = last_bslash + 1;
    return basename;
}

static void on_upload(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_PART_BEGIN) {
        struct mg_http_part *part = (struct mg_http_part *) ev_data;
        struct upload_state *us = calloc(1, sizeof(*us));
        c->fn_data = us;

        const char *basename = sanitize_filename(part->file_name);
        const char *extension = get_extension(basename);
        
        if (strlen(basename) == 0) {
            us->error = true;
            snprintf(us->error_message, sizeof(us->error_message), "Invalid file name.");
        } else if (!is_extension_allowed(extension)) {
            us->error = true;
            snprintf(us->error_message, sizeof(us->error_message), "File type not allowed.");
        } else {
            time_t t = time(NULL);
            snprintf(us->file_name, sizeof(us->file_name), "%s/%lx-%s", UPLOAD_DIR, (unsigned long)t, basename);
        }
    } else if (ev == MG_EV_HTTP_PART_DATA) {
        struct mg_http_part *part = (struct mg_http_part *) ev_data;
        struct upload_state *us = (struct upload_state *) c->fn_data;
        if (us->error) return;

        us->file_size += part->data.len;
        if (us->file_size > MAX_FILE_SIZE) {
            us->error = true;
            snprintf(us->error_message, sizeof(us->error_message), "File is too large.");
            mg_file_delete(&c->mgr->fs, us->file_name);
            return;
        }
        if (mg_file_append(&c->mgr->fs, us->file_name, part->data.ptr, part->data.len) != (int)part->data.len) {
            us->error = true;
            snprintf(us->error_message, sizeof(us->error_message), "Error writing to file.");
            mg_file_delete(&c->mgr->fs, us->file_name);
        }
    } else if (ev == MG_EV_HTTP_PART_END) {
        struct upload_state *us = (struct upload_state *) c->fn_data;
        if (us) {
            if (!us->error) {
                mg_http_reply(c, 200, "", "File uploaded successfully as %s\n", us->file_name);
            } else {
                mg_http_reply(c, 400, "", "%s\n", us->error_message);
            }
            free(us);
            c->fn_data = NULL;
        }
    }
    (void) fn_data;
}

static void upload_file_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/upload") && mg_vcmp(&hm->method, "POST") == 0) {
            mg_http_upload(c, hm, on_upload, NULL);
        } else {
             mg_http_reply(c, 200, "Content-Type: text/html\r\n",
                      "<!DOCTYPE html><html><body><h1>Upload File</h1>"
                      "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">"
                      "<input type=\"file\" name=\"file\" /><input type=\"submit\" value=\"Upload\" />"
                      "</form></body></html>");
        }
    }
    (void) fn_data;
}

int main(void) {
    struct mg_mgr mgr;
    
    mkdir(UPLOAD_DIR, 0755);

    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", upload_file_handler, NULL);
    
    printf("Starting C Mongoose server on http://0.0.0.0:8000\n");
    printf("Use a browser to access the root URL or use the curl commands below for testing.\n");
    printf("\n--- 5 Test Cases (using curl) ---\n");

    printf("\n1. Test Case: Successful upload of an allowed file type (.txt)\n");
    printf("echo \"This is a test file.\" > test.txt\n");
    printf("curl -X POST -F 'file=@test.txt' http://0.0.0.0:8000/upload\n");

    printf("\n2. Test Case: Attempted upload of a disallowed file type (.exe)\n");
    printf("echo \"dummy exe\" > test.exe\n");
    printf("curl -X POST -F 'file=@test.exe' http://0.0.0.0:8000/upload\n");

    printf("\n3. Test Case: Attempted upload of a file that is too large (e.g., > 10MB)\n");
    printf("fallocate -l 11M largefile.dat\n");
    printf("curl -X POST -F 'file=@largefile.dat' http://0.0.0.0:8000/upload\n");

    printf("\n4. Test Case: Attempted upload with a malicious filename (path traversal)\n");
    printf("echo \"malicious content\" > evil.txt\n");
    printf("curl -X POST -F 'file=@evil.txt;filename=\"../../evil.txt\"' http://0.0.0.0:8000/upload\n");

    printf("\n5. Test Case: Successful upload of another allowed file type (.jpg)\n");
    printf("# (Assuming you have a file named 'image.jpg' in the current directory)\n");
    printf("curl -X POST -F 'file=@image.jpg' http://0.0.0.0:8000/upload\n\n");
    
    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    
    mg_mgr_free(&mgr);
    return 0;
}