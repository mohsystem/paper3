#include "civetweb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

// Note: This code requires the civetweb library.
// You need to have civetweb.h and civetweb.c in your project.
// Compile on Linux/macOS: gcc your_file.c civetweb.c -o server -lpthread -ldl
// Compile on Windows: cl your_file.c civetweb.c /link /out:server.exe ws2_32.lib

typedef struct {
    char *username;
    char *text;
} Message;

static Message *messages = NULL;
static size_t message_count = 0;
static size_t message_capacity = 0;

#ifdef _WIN32
static CRITICAL_SECTION messages_mutex;
#else
static pthread_mutex_t messages_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 280

static void initialize_mutex() {
#ifdef _WIN32
    InitializeCriticalSection(&messages_mutex);
#endif
}

static void lock_mutex() {
#ifdef _WIN32
    EnterCriticalSection(&messages_mutex);
#else
    pthread_mutex_lock(&messages_mutex);
#endif
}

static void unlock_mutex() {
#ifdef _WIN32
    LeaveCriticalSection(&messages_mutex);
#else
    pthread_mutex_unlock(&messages_mutex);
#endif
}

char* escapeHtml(const char* input) {
    if (!input) return NULL;
    size_t input_len = strlen(input);
    size_t worst_case_len = input_len * 5 + 1;
    char* escaped = (char*)malloc(worst_case_len);
    if (!escaped) return NULL;

    char* p = escaped;
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&':  p += sprintf(p, "&amp;");   break;
            case '<':  p += sprintf(p, "&lt;");    break;
            case '>':  p += sprintf(p, "&gt;");    break;
            case '"':  p += sprintf(p, "&quot;");  break;
            case '\'': p += sprintf(p, "&#x27;");  break;
            default:   *p++ = input[i];            break;
        }
    }
    *p = '\0';
    return escaped;
}

int insert_user_message_in_db(const char* username, const char* message_text) {
    if (!username || !message_text) return -1;
    
    lock_mutex();
    if (message_count >= message_capacity) {
        size_t new_capacity = (message_capacity == 0) ? 10 : message_capacity * 2;
        Message *new_messages = (Message*)realloc(messages, new_capacity * sizeof(Message));
        if (!new_messages) {
            unlock_mutex(); return -1;
        }
        messages = new_messages;
        message_capacity = new_capacity;
    }
#ifdef _WIN32
    messages[message_count].username = _strdup(username);
    messages[message_count].text = _strdup(message_text);
#else
    messages[message_count].username = strdup(username);
    messages[message_count].text = strdup(message_text);
#endif
    if (!messages[message_count].username || !messages[message_count].text) {
        free(messages[message_count].username);
        free(messages[message_count].text);
        unlock_mutex(); return -1;
    }
    message_count++;
    unlock_mutex();
    return 0;
}

int request_handler(struct mg_connection *conn, void *cbdata) {
    const struct mg_request_info *req_info = mg_get_request_info(conn);

    if (strcmp(req_info->request_method, "GET") == 0 && strcmp(req_info->local_uri, "/") == 0) {
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        mg_printf(conn,
                  "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Message Board</title>"
                  "<style>body { font-family: sans-serif; margin: 2em; } .message { border: 1px solid #ccc; padding: 10px; margin-bottom: 10px; border-radius: 5px; } .message-user { font-weight: bold; } .message-text { white-space: pre-wrap; word-wrap: break-word; }</style>"
                  "</head><body>"
                  "<h1>Post a Message</h1>"
                  "<form action=\"/post\" method=\"post\">"
                  "<label for=\"username\">Username:</label><br>"
                  "<input type=\"text\" id=\"username\" name=\"username\" required maxlength=\"%d\"><br><br>"
                  "<label for=\"message\">Message:</label><br>"
                  "<textarea id=\"message\" name=\"message\" rows=\"4\" cols=\"50\" required maxlength=\"%d\"></textarea><br><br>"
                  "<input type=\"submit\" value=\"Submit\">"
                  "</form>"
                  "<h2>Messages</h2>",
                  MAX_USERNAME_LENGTH, MAX_MESSAGE_LENGTH);
        
        lock_mutex();
        for (size_t i = 0; i < message_count; ++i) {
            size_t index = message_count - 1 - i;
            char *safe_username = escapeHtml(messages[index].username);
            char *safe_message = escapeHtml(messages[index].text);
            if (safe_username && safe_message) {
                 mg_printf(conn, "<div class='message'><div class='message-user'>%s</div><div class='message-text'>%s</div></div>", safe_username, safe_message);
            }
            free(safe_username);
            free(safe_message);
        }
        unlock_mutex();
        mg_printf(conn, "</body></html>");
        return 200;

    } else if (strcmp(req_info->request_method, "POST") == 0 && strcmp(req_info->local_uri, "/post") == 0) {
        char username_buf[MAX_USERNAME_LENGTH + 1] = {0};
        char message_buf[MAX_MESSAGE_LENGTH + 1] = {0};
        int username_len = mg_get_var(conn, "username", username_buf, sizeof(username_buf));
        int message_len = mg_get_var(conn, "message", message_buf, sizeof(message_buf));

        if (username_len <= 0 || message_len <= 0) {
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 32\r\n\r\nUsername and message are required.");
            return 400;
        }
        if (insert_user_message_in_db(username_buf, message_buf) != 0) {
            mg_printf(conn, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 20\r\n\r\nError saving message.");
            return 500;
        }
        mg_printf(conn, "HTTP/1.1 303 See Other\r\nLocation: /\r\n\r\n");
        return 303;
    }

    mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found.");
    return 404;
}

void cleanup() {
    lock_mutex();
    for (size_t i = 0; i < message_count; ++i) {
        free(messages[i].username);
        free(messages[i].text);
    }
    free(messages);
    unlock_mutex();
#ifndef _WIN32
    pthread_mutex_destroy(&messages_mutex);
#endif
}

int main(void) {
    initialize_mutex();
    
    insert_user_message_in_db("Alice", "Hello from C and Civetweb!");
    insert_user_message_in_db("Bob", "This is a test message.");
    insert_user_message_in_db("Charlie", "Memory is managed manually.");
    insert_user_message_in_db("David", "Mutexes for thread safety are important.");
    insert_user_message_in_db("Eve", "<script>alert('I am escaped!')</script>");

    const char *options[] = {"listening_ports", "8080", "num_threads", "4", NULL};
    struct mg_context *ctx = mg_start(NULL, 0, options);

    if (ctx == NULL) {
        fprintf(stderr, "Failed to start server.\n");
        cleanup(); return 1;
    }
    mg_set_request_handler(ctx, "/", request_handler, NULL);
    printf("Server started on http://localhost:8080\nPress Enter to stop the server.\n");
    getchar();
    mg_stop(ctx);
    printf("Server stopped.\n");
    cleanup();
    return 0;
}