#define CROW_MAIN
#include "crow_all.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>

// In-memory "database"
std::vector<std::pair<std::string, std::string>> messages;

// Utility to prevent XSS by escaping HTML special characters
std::string escapeHTML(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#39;");       break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}

// Simulates inserting a message into the database
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    if (!username.empty() && !message.empty()) {
        messages.push_back({username, message});
    }
}

// Renders the messages into an HTML string
std::string get_messages_as_html() {
    if (messages.empty()) {
        return "<p>No messages yet.</p>";
    }
    std::stringstream ss;
    for (const auto& msg : messages) {
        ss << "<p><strong>" << escapeHTML(msg.first) << ":</strong> " << escapeHTML(msg.second) << "</p>";
    }
    return ss.str();
}

// The main class name Lse149 is not directly applicable in the same way as Java,
// but we'll structure the main function to represent the application logic.
int main() {
    crow::SimpleApp app;

    // Route for the main page (display form and messages)
    CROW_ROUTE(app, "/")([](){
        std::stringstream ss;
        ss << "<!doctype html><html lang=\"en\">";
        ss << "<head><meta charset=\"utf-8\"><title>Message Board</title></head>";
        ss << "<body>";
        ss << "<h1>Post a Message</h1>";
        ss << "<form action=\"/post\" method=\"post\">";
        ss << "<label for=\"username\">Username:</label><br>";
        ss << "<input type=\"text\" id=\"username\" name=\"username\" required><br>";
        ss << "<label for=\"message\">Message:</label><br>";
        ss << "<textarea id=\"message\" name=\"message\" required></textarea><br><br>";
        ss << "<input type=\"submit\" value=\"Post\">";
        ss << "</form>";
        ss << "<h2>Messages</h2>";
        ss << "<div id=\"messages\">" << get_messages_as_html() << "</div>";
        ss << "</body></html>";
        return crow::response(ss.str());
    });

    // Route for handling form submission
    CROW_ROUTE(app, "/post").methods(crow::HTTPMethod::Post)
    ([](const crow::request& req){
        auto params = req.get_body_params();
        std::string username = params.get("username");
        std::string message = params.get("message");

        if (username.empty() || message.empty()) {
            return crow::response(400, "Username and message cannot be empty."); // Bad Request
        }

        insert_user_message_in_db(username, message);
        
        // Redirect to the main page
        auto res = crow::response(303); // See Other
        res.add_header("Location", "/");
        return res;
    });

    std::cout << "Server starting on port 18080" << std::endl;
    std::cout << "Manual Test Cases to run in your browser:" << std::endl;
    std::cout << "1. Go to http://127.0.0.1:18080. You should see a form and 'No messages yet'." << std::endl;
    std::cout << "2. Submit a message with username 'Alice' and message 'Hello World'. You should be redirected and see Alice's message." << std::endl;
    std::cout << "3. Submit another message with username 'Bob' and message 'Hi Alice!'. You should see both messages displayed." << std::endl;
    std::cout << "4. Submit a message with username 'Eve' and message '<b>bold</b>'. The output should show the literal tags '<b>bold</b>' and not bolded text, preventing XSS." << std::endl;
    std::cout << "5. Submit a message with username 'Mallory' and message '<script>alert(\"xss\")</script>'. No alert box should appear, and the script tag should be displayed as text." << std::endl;

    app.port(18080).multithreaded().run();
    
    return 0;
}

// To run this code:
// 1. You need a C++ compiler (like g++).
// 2. You need to download the Crow library (it's header-only).
//    - Go to https://github.com/CrowCpp/Crow and download `crow_all.h`.
//    - Place `crow_all.h` in the same directory as your source file.
// 3. Compile the code. You might need to link against Boost system and ssl libraries depending on your setup.
//    A simple compile command would be:
//    g++ -o Lse149 Lse149.cpp -pthread -lboost_system