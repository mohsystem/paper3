// This code requires the Mongoose library.
// Get mongoose.c and mongoose.h from https://github.com/cesanta/mongoose
// Compile with: gcc -o server your_file.c mongoose.c -I.
// On Windows with MSVC: cl your_file.c mongoose.c /I. /W4
// For this single-file requirement, we'll #include mongoose.c.
#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <uuid/uuid.h> // Requires libuuid-dev on Debian/Ubuntu, or similar

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#define MKDIR(path) mkdir(path, 0755)
#endif


static const char *s_http_addr = "http://0.0.0.0:8080";
static const char *s_upload_dir = "uploads";
#define MAX_FILENAME_LEN 256
#define MAX_PATH_LEN 512

// Note: A real UUID implementation is preferred. This is a placeholder for systems without libuuid.
void generate_simple_uuid(char *dst) {
    long t = time(NULL);
    int r = rand();
    sprintf(dst, "%lx-%x", t, r);
}

// Safely sanitize a filename by extracting its basename.
// Returns a pointer to the basename within the original string.
// Rejects any string containing path separators.
static char* sanitize_filename(char *filename) {
    if (filename == NULL || strpbrk(filename, "/\\") != NULL) {
        return NULL; // Reject any path-like string
    }
    return filename; // Already a basename
}

static void ev_handler(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // Serve root page with upload form
        if (mg_http_match_uri(hm, "/")) {
            const char *html_form =
                "<!DOCTYPE html><html><head><title>File Upload</title></head><body>"
                "<h2>Upload PDF File</h2>"
                "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
                "Select PDF file to upload:"
                "<input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\" accept=\".pdf\" required>"
                "<input type=\"submit\" value=\"Upload PDF\" name=\"submit\">"
                "</form></body></html>";
            mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", html_form);
        } else if (mg_http_match_uri(hm, "/upload")) {
             mg_http_reply(c, 405, "", "Method Not Allowed - use multipart/form-data POST");
        } else {
            mg_http_reply(c, 404, "", "Not Found");
        }
    } else if (ev == MG_EV_HTTP_PART) {
        struct mg_http_part *part = (struct mg_http_part *) ev_data;
        if (strcmp(part->name, "fileToUpload") == 0) {
            // This is our file part.
            // On the first part chunk (part->data.len > 0), open a file.
            if (part->status == MG_HTTP_PART_BEGIN) {
                if (part->filename == NULL || part->filename[0] == '\0') {
                    mg_http_reply(c, 400, "", "Bad Request: No filename provided.");
                    c->is_draining = 1;
                    return;
                }

                // --- Security: Sanitize filename ---
                char* sanitized_basename = sanitize_filename(part->filename);
                if (sanitized_basename == NULL || strlen(sanitized_basename) == 0) {
                    mg_http_reply(c, 400, "", "Bad Request: Invalid filename (contains path characters).");
                    c->is_draining = 1;
                    return;
                }

                // Check extension
                const char *ext = strrchr(sanitized_basename, '.');
                if (!ext || strcasecmp(ext, ".pdf") != 0) {
                    mg_http_reply(c, 400, "", "Bad Request: Only PDF files are allowed.");
                    c->is_draining = 1;
                    return;
                }
                
                // Generate unique filename to prevent overwrites
                char uuid_str[37];
                #ifdef __APPLE__ // or other systems without uuid/uuid.h easily available
                   generate_simple_uuid(uuid_str);
                #else
                   uuid_t binuuid;
                   uuid_generate_random(binuuid);
                   uuid_unparse_lower(binuuid, uuid_str);
                #endif

                char unique_filename[MAX_FILENAME_LEN];
                snprintf(unique_filename, sizeof(unique_filename), "%s_%s", uuid_str, sanitized_basename);
                
                // --- Security: Construct safe path ---
                char final_path[MAX_PATH_LEN];
                snprintf(final_path, sizeof(final_path), "%s/%s", s_upload_dir, unique_filename);

                // Open file for writing. Using "wbx" would be ideal (atomic create), but not portable.
                // We use a unique name, so collision is highly unlikely.
                FILE *fp = fopen(final_path, "wb");
                if (fp == NULL) {
                    mg_http_reply(c, 500, "", "Internal Server Error: Could not open file for writing.");
                    c->is_draining = 1;
                } else {
                    c->user_data = fp; // Store file pointer in connection context
                    printf("Receiving file: %s\n", final_path);
                }
            } else if (part->status == MG_HTTP_PART_DATA) {
                 FILE *fp = (FILE *) c->user_data;
                 if (fp != NULL && fwrite(part->data.ptr, 1, part->data.len, fp) != part->data.len) {
                    mg_http_reply(c, 500, "", "Internal Server Error: Failed to write to file.");
                    fclose(fp);
                    c->user_data = NULL; // Clear pointer
                    c->is_draining = 1;
                 }
            } else if (part->status == MG_HTTP_PART_END) {
                FILE *fp = (FILE *) c->user_data;
                if (fp != NULL) {
                    printf("File saved successfully.\n");
                    fclose(fp);
                    c->user_data = NULL; // Clear pointer
                    mg_http_reply(c, 200, "", "File uploaded successfully!");
                }
                // If fp is NULL, an error was already sent.
            }
        }
    }
    (void) fn_data;
}

int main(void) {
    struct mg_mgr mgr;
    
    // Create upload directory if it doesn't exist
    MKDIR(s_upload_dir);

    mg_mgr_init(&mgr);
    printf("Starting web server on %s\n", s_http_addr);
    if (mg_http_listen(&mgr, s_http_addr, ev_handler, NULL) == NULL) {
        fprintf(stderr, "Cannot start server on %s\n", s_http_addr);
        return 1;
    }

    // --- Test Cases Instructions ---
    printf("\n--- To run test cases, execute the following commands in your terminal ---\n");
    printf("1. Create some dummy files:\n");
    printf("   echo \"%%PDF-1.4...\" > test1.pdf\n");
    printf("   echo \"This is text\" > not_a_pdf.txt\n");
    printf("   echo \"evil\" > evil.pdf\n");
    printf("   touch empty.pdf\n");
    printf("\n2. Run curl commands:\n");
    printf("   # Test 1 (Success): Valid PDF\n");
    printf("   curl -v -F \"fileToUpload=@test1.pdf\" %s/upload\n", s_http_addr);
    printf("   # Test 2 (Fail): Invalid file type\n");
    printf("   curl -v -F \"fileToUpload=@not_a_pdf.txt\" %s/upload\n", s_http_addr);
    printf("   # Test 3 (Fail): Malicious filename with path characters\n");
    printf("   curl -v -F \"fileToUpload=@evil.pdf;filename=../../evil.pdf\" %s/upload\n", s_http_addr);
    printf("   # Test 4 (Success): Normal upload\n");
    printf("   curl -v -F \"fileToUpload=@evil.pdf\" %s/upload\n", s_http_addr);
    printf("   # Test 5 (Fail): No filename provided\n");
    printf("   curl -v -F \"fileToUpload=@empty.pdf;filename=\" %s/upload\n", s_http_addr);
    printf("--------------------------------------------------------------------------\n");

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}

// Include mongoose.c directly to make it a "single file" project for simplicity.
#include "mongoose.c"