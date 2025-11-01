#include "civetweb.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>
#include <cstring>
#include <algorithm>

// Note: This code requires the civetweb library.
// You need to have civetweb.h and civetweb.c in your project.
// Compile on Linux/macOS: g++ -std=c++11 your_file.cpp civetweb.c -o server -lpthread -ldl
// Compile on Windows: cl your_file.cpp civetweb.c /link /out:server.exe ws2_32.lib

// A simple struct to hold message data
struct Message {
    std::string username;
    std::string text;
};

// In-memory "database"
static std::vector<Message> messages;
// Mutex to protect the messages vector from concurrent access
static std::mutex messages_mutex;

// Max lengths for input validation
const size_t MAX_USERNAME_LENGTH = 50;
const size_t MAX_MESSAGE_LENGTH = 280;

/**
 * A utility function to escape HTML characters to prevent XSS.
 */
std::string escapeHtml(const std::string& input) {
    std::ostringstream escaped;
    escaped.str("");
    for (char c : input) {
        switch (c) {
            case '&':  escaped << "&amp;";       break;
            case '<':  escaped << "&lt;";        break;
            case '>':  escaped << "&gt;";        break;
            case '"':  escaped << "&quot;";      break;
            case '\'': escaped << "&#x27;";      break;
            default:   escaped << c;             break;
        }
    }
    return escaped.str();
}

/**
 * Inserts a user message into the in-memory database in a thread-safe manner.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    std::lock_guard<std::mutex> lock(messages_mutex);
    messages.push_back({username, message});
}

// Main handler for all requests
int request_handler(struct mg_connection *conn, void *cbdata) {
    const struct mg_request_info *req_info = mg_get_request_info(conn);

    if (strcmp(req_info->request_method, "GET") == 0 && strcmp(req_info->local_uri, "/") == 0) {
        std::ostringstream html;
        html << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Message Board</title>"
             << "<style>body { font-family: sans-serif; margin: 2em; } .message { border: 1px solid #ccc; padding: 10px; margin-bottom: 10px; border-radius: 5px; } .message-user { font-weight: bold; } .message-text { white-space: pre-wrap; word-wrap: break-word; }</style>"
             << "</head><body>"
             << "<h1>Post a Message</h1>"
             << "<form action=\"/post\" method=\"post\">"
             << "<label for=\"username\">Username:</label><br>"
             << "<input type=\"text\" id=\"username\" name=\"username\" required maxlength=\"" << MAX_USERNAME_LENGTH << "\"><br><br>"
             << "<label for=\"message\">Message:</label><br>"
             << "<textarea id=\"message\" name=\"message\" rows=\"4\" cols=\"50\" required maxlength=\"" << MAX_MESSAGE_LENGTH << "\"></textarea><br><br>"
             << "<input type=\"submit\" value=\"Submit\">"
             << "</form>"
             << "<h2>Messages</h2>";

        {
            std::lock_guard<std::mutex> lock(messages_mutex);
            for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
                html << "<div class='message'>"
                     << "<div class='message-user'>" << escapeHtml(it->username) << "</div>"
                     << "<div class='message-text'>" << escapeHtml(it->text) << "</div>"
                     << "</div>";
            }
        }
        html << "</body></html>";
        std::string body = html.str();
        mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s", body.length(), body.c_str());
        return 200;

    } else if (strcmp(req_info->request_method, "POST") == 0 && strcmp(req_info->local_uri, "/post") == 0) {
        char username_buf[MAX_USERNAME_LENGTH + 1];
        char message_buf[MAX_MESSAGE_LENGTH + 1];
        
        int username_len = mg_get_var(conn, "username", username_buf, sizeof(username_buf));
        int message_len = mg_get_var(conn, "message", message_buf, sizeof(message_buf));

        if (username_len <= 0 || message_len <= 0) {
            mg_printf(conn, "HTTP/1.1 400 Bad Request\r\nContent-Length: 32\r\n\r\nUsername and message are required.");
            return 400;
        }
        
        std::string username(username_buf, username_len);
        std::string message(message_buf, message_len);

        insert_user_message_in_db(username, message);
        mg_printf(conn, "HTTP/1.1 303 See Other\r\nLocation: /\r\n\r\n");
        return 303;
    }
    
    mg_printf(conn, "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found.");
    return 404;
}

int main() {
    insert_user_message_in_db("Alice", "Hello from C++ and Civetweb!");
    insert_user_message_in_db("Bob", "This is a test message.");
    insert_user_message_in_db("Charlie", "Concurrency is handled with std::mutex.");
    insert_user_message_in_db("David", "Using mg_get_var is a safe way to get POST data.");
    insert_user_message_in_db("Eve", "<h1>HTML will be escaped</h1>");

    const char *options[] = {"listening_ports", "8080", "num_threads", "4", NULL};
    struct mg_context *ctx = mg_start(NULL, 0, options);

    if (ctx == NULL) {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }
    
    mg_set_request_handler(ctx, "/", request_handler, NULL);
    
    std::cout << "Server started on http://localhost:8080" << std::endl;
    std::cout << "Press Enter to stop the server." << std::endl;
    getchar();
    
    mg_stop(ctx);
    std::cout << "Server stopped." << std::endl;
    
    return 0;
}