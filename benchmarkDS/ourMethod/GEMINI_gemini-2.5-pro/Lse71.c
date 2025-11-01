#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#include "mongoose.h"
#include <curl/curl.h>

#if defined(_WIN32)
#include <windows.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// --- Configuration ---
static const char *s_http_port = "8080";
static const char *s_upload_dir = "uploads";
#define MAX_FILENAME_LEN 256
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB

// --- State for file uploads ---
struct file_upload_state {
  char temp_path[MAX_FILENAME_LEN];
  FILE *fp;
  size_t bytes_written;
};

// --- Helper Functions ---
// Safely gets the file extension from a path
static const char *get_file_extension(const char *filename) {
    if (filename == NULL) return "";
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot;
}

// Checks if a file extension is in the allowed list
static int is_extension_allowed(const char *filename) {
    const char *ext = get_file_extension(filename);
    if (strlen(ext) == 0) return 0;
    
    const char *allowed[] = {".txt", ".pdf", ".png", ".jpg", ".jpeg", ".gif", NULL};
    for (int i = 0; allowed[i] != NULL; i++) {
        if (strcasecmp(ext, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Extracts the basename of a path, preventing traversal
static const char *get_basename(const char *path) {
    if (path == NULL) return NULL;
    const char *basename = strrchr(path, '/');
    #if defined(_WIN32)
    const char *win_basename = strrchr(path, '\\');
    if (win_basename > basename) basename = win_basename;
    #endif
    return basename ? basename + 1 : path;
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    // Serve a simple upload form for GET requests to "/"
    if (mg_http_match_uri(hm, "/")) {
        mg_http_reply(nc, 200, "Content-Type: text/html\r\n",
          "<!DOCTYPE html><html><head><title>File Upload</title></head>"
          "<body><h1>Upload File</h1>"
          "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
          "<input type=\"file\" name=\"file\" /><input type=\"submit\" value=\"Upload\" />"
          "</form></body></html>");
    }
  } else if (ev == MG_EV_HTTP_PART_BEGIN) {
    struct mg_http_part *part = (struct mg_http_part *)ev_data;
    
    // Check if the form field name is "file"
    if (strcmp(part->name, "file") != 0) return;

    // Sanitize filename and validate extension
    const char *sanitized_filename = get_basename(part->filename);
    if (sanitized_filename == NULL || strlen(sanitized_filename) == 0) {
        mg_http_reply(nc, 400, "", "{\"error\": \"Invalid filename\"}");
        nc->is_draining = 1;
        return;
    }
    if (!is_extension_allowed(sanitized_filename)) {
        mg_http_reply(nc, 400, "", "{\"error\": \"File type not allowed\"}");
        nc->is_draining = 1;
        return;
    }
    
    struct file_upload_state *state = calloc(1, sizeof(struct file_upload_state));
    if (!state) {
        mg_http_reply(nc, 500, "", "{\"error\": \"Internal server error: out of memory\"}");
        nc->is_draining = 1;
        return;
    }
    part->user_data = state;

    // Create a unique temporary filename
    snprintf(state->temp_path, sizeof(state->temp_path), "%s/upload_tmp_%ld.tmp", 
             s_upload_dir, (long)time(NULL));
             
    state->fp = fopen(state->temp_path, "wb");
    if (state->fp == NULL) {
        perror("fopen");
        mg_http_reply(nc, 500, "", "{\"error\": \"Internal server error: could not create temp file\"}");
        nc->is_draining = 1;
        free(state);
        part->user_data = NULL;
    }

  } else if (ev == MG_EV_HTTP_PART_DATA) {
    struct mg_http_part *part = (struct mg_http_part *)ev_data;
    struct file_upload_state *state = (struct file_upload_state *) part->user_data;

    if (state == NULL || state->fp == NULL) return;

    if (fwrite(part->data.ptr, 1, part->data.len, state->fp) != part->data.len) {
        perror("fwrite");
        mg_http_reply(nc, 500, "", "{\"error\": \"Failed to write to file\"}");
        nc->is_draining = 1;
        return;
    }
    state->bytes_written += part->data.len;
    if (state->bytes_written > MAX_FILE_SIZE) {
        mg_http_reply(nc, 413, "", "{\"error\": \"File too large\"}");
        nc->is_draining = 1;
    }

  } else if (ev == MG_EV_HTTP_PART_END) {
    struct mg_http_part *part = (struct mg_http_part *)ev_data;
    struct file_upload_state *state = (struct file_upload_state *) part->user_data;
    
    if (state == NULL) return;

    if (state->fp != NULL) {
        fclose(state->fp);
        state->fp = NULL;
    }

    if (nc->is_draining) { // An error occurred, cleanup
        remove(state->temp_path);
    } else {
        // Success: rename temp file to final destination
        const char *sanitized_filename = get_basename(part->filename);
        const char *ext = get_file_extension(sanitized_filename);

        char final_path[MAX_FILENAME_LEN];
        snprintf(final_path, sizeof(final_path), "%s/upload_%ld_%d%s", 
                 s_upload_dir, (long)time(NULL), rand() % 1000, ext);
        
        if (rename(state->temp_path, final_path) != 0) {
            perror("rename");
            remove(state->temp_path);
            mg_http_reply(nc, 500, "", "{\"error\": \"Failed to save file\"}");
        } else {
            // Set restrictive permissions
            #if !defined(_WIN32)
            chmod(final_path, S_IRUSR | S_IWUSR);
            #endif
            mg_http_reply(nc, 200, "", "{\"status\": \"File uploaded successfully\"}");
        }
    }
    free(state);
    part->user_data = NULL;
  }
}

void create_dummy_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (fp) {
        fputs(content, fp);
        fclose(fp);
    }
}

void run_tests() {
    CURL *curl;
    CURLcode res;

    printf("\n--- Running Test Cases ---\n");
    curl_global_init(CURL_GLOBAL_ALL);

    const char *test_files[] = {"test1.txt", "test2.jpg", "test3.html", "test4.txt"};
    create_dummy_file(test_files[0], "hello world");
    create_dummy_file(test_files[1], "imagedata");
    create_dummy_file(test_files[2], "<html></html>");
    create_dummy_file(test_files[3], "malicious");
    
    struct {
        const char *name;
        const char *filename_to_send; // filename sent in the form
        const char *local_path;       // local file to read content from
        int expect_success;
    } tests[] = {
        {"Valid .txt file", "test1.txt", test_files[0], 1},
        {"Valid .jpg file", "test2.jpg", test_files[1], 1},
        {"Disallowed .html file", "test3.html", test_files[2], 0},
        {"Path traversal attempt", "../../etc/passwd.txt", test_files[3], 1}, // Server should sanitize
        {"No file", NULL, NULL, 0}
    };
    
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        curl = curl_easy_init();
        if(curl) {
            curl_mime *form = NULL;
            curl_mimepart *field = NULL;
            
            curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080/upload");

            if (tests[i].filename_to_send) {
                form = curl_mime_init(curl);
                field = curl_mime_addpart(form);
                curl_mime_name(field, "file");
                curl_mime_filedata(field, tests[i].local_path);
                curl_mime_filename(field, tests[i].filename_to_send);
                curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
            } else {
                // Post empty form for "No file" test
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            }

            res = curl_easy_perform(curl);
            
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            int is_success = (http_code >= 200 && http_code < 300);

            if (is_success == tests[i].expect_success) {
                printf("[PASS] %s\n", tests[i].name);
            } else {
                printf("[FAIL] %s - Expected %s, but got status %ld\n", tests[i].name, tests[i].expect_success ? "success" : "failure", http_code);
            }
            
            curl_easy_cleanup(curl);
            if(form) curl_mime_free(form);
        }
    }
    
    // Cleanup
    for(size_t i=0; i < sizeof(test_files)/sizeof(test_files[0]); ++i) {
        remove(test_files[i]);
    }

    curl_global_cleanup();
    printf("--- Test Cases Finished ---\n");
}


int main(int argc, char *argv[]) {
  srand((unsigned int)time(NULL));
  struct mg_mgr mgr;

  // Create upload directory if it doesn't exist
  MKDIR(s_upload_dir);

  mg_mgr_init(&mgr);
  printf("Starting web server on port %s\n", s_http_port);
  
  if (mg_http_listen(&mgr, s_http_port, ev_handler, NULL) == NULL) {
    fprintf(stderr, "Failed to start listener\n");
    return 1;
  }
  
  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    run_tests();
    mg_mgr_free(&mgr);
    return 0;
  }
  
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }

  mg_mgr_free(&mgr);
  return 0;
}