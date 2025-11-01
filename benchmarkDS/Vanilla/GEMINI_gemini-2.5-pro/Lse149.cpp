// NOTE: This code requires the 'cpp-httplib' single-header library.
// Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
// and place it in the same directory or in your compiler's include path.
// To compile: g++ -o server Lse149.cpp -pthread -std=c++11

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <mutex>
#include "httplib.h"

// A class to encapsulate the server logic
class Lse149 {
private:
    // In-memory "database" using a vector of pairs.
    // A mutex is used for thread-safe access since the server is multi-threaded.
    std::vector<std::pair<std::string, std::string>> messages;
    std::mutex messages_mutex;

public:
    Lse149() {
        // 5 Test cases: Pre-populate the database
        insert_user_message_in_db("Alice", "Hello from C++!");
        insert_user_message_in_db("Bob", "This server uses the cpp-httplib library.");
        insert_user_message_in_db("Charlie", "C++ can do web development too.");
        insert_user_message_in_db("David", "The server is multi-threaded by default.");
        insert_user_message_in_db("Eve", "Time to post a new message.");
    }

    // Function to simulate DB insertion
    void insert_user_message_in_db(const std::string& username, const std::string& message) {
        if (!username.empty() && !message.empty()) {
            std::lock_guard<std::mutex> lock(messages_mutex);
            messages.push_back({username, message});
        }
    }

    // Basic HTML escaping to prevent XSS
    static std::string escapeHtml(const std::string& text) {
        std::string result;
        result.reserve(text.size());
        for (char c : text) {
            switch (c) {
                case '&':  result += "&amp;";   break;
                case '\"': result += "&quot;";  break;
                case '\'': result += "&#039;";  break;
                case '<':  result += "&lt;";    break;
                case '>':  result += "&gt;";    break;
                default:   result += c;         break;
            }
        }
        return result;
    }

    void run_server() {
        httplib::Server svr;

        // GET handler for the root path '/'
        svr.Get("/", [this](const httplib::Request&, httplib::Response& res) {
            std::string html_body;
            html_body += "<html><body>";
            html_body += "<h1>Post a Message</h1>";
            html_body += "<form action=\"/post\" method=\"post\">";
            html_body += "Username: <input type=\"text\" name=\"username\"><br>";
            html_body += "Message:  <input type=\"text\" name=\"message\"><br>";
            html_body += "<input type=\"submit\" value=\"Post\">";
            html_body += "</form>";
            html_body += "<hr><h1>Messages</h1>";
            html_body += "<ul>";

            {
                std::lock_guard<std::mutex> lock(messages_mutex);
                // Iterate in reverse to show the latest messages first
                for (auto it = this->messages.rbegin(); it != this->messages.rend(); ++it) {
                    std::string username = escapeHtml(it->first);
                    std::string message = escapeHtml(it->second);
                    html_body += "<li><b>" + username + ":</b> " + message + "</li>";
                }
            }

            html_body += "</ul></body></html>";
            res.set_content(html_body, "text/html");
        });

        // POST handler for the '/post' path
        svr.Post("/post", [this](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("username") && req.has_param("message")) {
                std::string username = req.get_param_value("username");
                std::string message = req.get_param_value("message");
                this->insert_user_message_in_db(username, message);
            }
            // Redirect the user to the root page
            res.set_redirect("/");
        });

        std::cout << "C++ server started on port 8080. Open http://localhost:8080/" << std::endl;
        svr.listen("0.0.0.0", 8080);
    }
};

int main() {
    Lse149 app;
    app.run_server();
    return 0;
}