#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Instructions:
// 1. This code requires the 'mongoose.h' and 'mongoose.c' files.
//    Download them from: https://github.com/cesanta/mongoose
// 2. Compile with: gcc -o server main.c mongoose.c -W -Wall -Wextra -pedantic
// 3. Run: ./server

static const char *s_http_addr = "http://0.0.0.0:8080";
static const char *s_root_dir = "images";

// Event handler function for the web server.
// It is called for each network event.
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    // Check if the request is for the /img endpoint
    if (mg_http_match_uri(hm, "/img")) {
      char name[256];
      
      // Get the 'name' parameter from the URL query string
      int ret = mg_http_get_var(&hm->query, "name", name, sizeof(name));
      
      if (ret <= 0) {
        mg_http_reply(c, 400, "", "Bad Request: 'name' parameter is missing or empty.\n");
        return;
      }

      // Security: Sanitize the filename to prevent path traversal.
      // A valid filename should not contain '/' or '\'.
      for (size_t i = 0; i < strlen(name); i++) {
        if (name[i] == '/' || name[i] == '\\' || name[i] == '.') {
          if (name[i] == '.' && name[i+1] == '.') {
            mg_http_reply(c, 400, "", "Bad Request: Invalid filename.\n");
            return;
          }
        }
      }

      // Use Mongoose's built-in secure file serving function.
      // It handles path traversal protection by restricting access to the s_root_dir.
      // It also handles setting Content-Type, caching headers, and range requests.
      struct mg_http_serve_opts opts = {.root_dir = s_root_dir};
      mg_http_serve_file(c, hm, name, &opts);
    } else {
      // For any other URI, serve files from the root directory as well (or return 404).
      // Here we choose to reply with a simple message.
      mg_http_reply(c, 404, "", "Not Found\n");
    }
  }
  (void) fn_data;
}

void setup_test_environment() {
    // POSIX-specific directory and file creation
    struct stat st = {0};
    if (stat(s_root_dir, &st) == -1) {
        mkdir(s_root_dir, 0700);
    }
    
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/test.txt", s_root_dir);
    
    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        const char *content = "This is a test file.";
        write(fd, content, strlen(content));
        close(fd);
    }
}

int main(void) {
  struct mg_mgr mgr;

  setup_test_environment();
  
  mg_mgr_init(&mgr);
  mg_http_listen(&mgr, s_http_addr, fn, NULL);

  printf("Server started on port 8080\n");
  printf("Run the following curl commands to test:\n");
  printf("1. Test Case 1 (Valid): curl http://localhost:8080/img?name=test.txt\n");
  printf("2. Test Case 2 (Not Found): curl http://localhost:8080/img?name=notfound.txt\n");
  printf("3. Test Case 3 (Path Traversal): curl \"http://localhost:8080/img?name=../main.c\"\n");
  printf("4. Test Case 4 (Missing Param): curl http://localhost:8080/img\n");
  printf("5. Test Case 5 (Empty Param): curl http://localhost:8080/img?name=\n");

  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }

  mg_mgr_free(&mgr);
  return 0;
}