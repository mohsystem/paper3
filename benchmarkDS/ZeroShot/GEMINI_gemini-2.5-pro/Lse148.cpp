#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// A simple structure to hold message information
struct Message {
    std::string username;
    std::string content;
};

// In-memory vector to act as a mock database
std::vector<Message> messageDatabase;

/**
 * Sanitizes user input to prevent Cross-Site Scripting (XSS) attacks.
 * This is done by escaping HTML special characters.
 *
 * @param input The raw string from the user.
 * @return A sanitized string safe for HTML rendering.
 */
std::string htmlEscape(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.length()); // Pre-allocate memory for efficiency
    for (char c : input) {
        switch (c) {
            case '&':  escaped += "&amp;";   break;
            case '<':  escaped += "&lt;";    break;
            case '>':  escaped += "&gt;";    break;
            case '"':  escaped += "&quot;";  break;
            case '\'': escaped += "&#39;";   break;
            default:   escaped += c;         break;
        }
    }
    return escaped;
}

/**
 * Inserts a user's message into our mock database.
 * Input is stored raw. Sanitization happens on display.
 *
 * @param username The name of the user posting the message.
 * @param message  The content of the message.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    // Basic input validation
    if (username.empty() || message.empty()) {
        std::cout << "Error: Username and message cannot be empty." << std::endl;
        return;
    }
    if (username.length() > 50 || message.length() > 280) {
        std::cout << "Error: Username or message is too long." << std::endl;
        return;
    }
    messageDatabase.push_back({username, message});
    std::cout << "Message from '" << username << "' stored successfully." << std::endl;
}

/**
 * Simulates a user submitting a post.
 *
 * @param username The name of the user from the request.
 * @param message  The message content from the request.
 */
void post(const std::string& username, const std::string& message) {
    std::cout << "\n--- Received new post from user: " << username << " ---" << std::endl;
    insert_user_message_in_db(username, message);
    std::cout << "...Redirecting user to the main page." << std::endl;
}

/**
 * Displays all messages from the mock database in a secure manner.
 */
void display_all_messages() {
    std::cout << "\n--- Main Page: All Messages ---" << std::endl;
    if (messageDatabase.empty()) {
        std::cout << "No messages to display." << std::endl;
    } else {
        for (const auto& msg : messageDatabase) {
            // Sanitize output just before displaying it to prevent stored XSS.
            std::string safeUsername = htmlEscape(msg.username);
            std::string safeContent = htmlEscape(msg.content);
            std::cout << "User: " << safeUsername << std::endl;
            std::cout << "Message: " << safeContent << std::endl;
            std::cout << "---------------------------------" << std::endl;
        }
    }
}


int main() {
    // --- Test Cases ---

    // Test Case 1: Normal post
    post("Alice", "Hello, world! This is my first message.");

    // Test Case 2: Another normal post
    post("Bob", "This is a great platform!");

    // Test Case 3: Post with special HTML characters
    post("Charlie", "My thoughts on C++: it's great for when you need <performance> & control.");

    // Test Case 4: A malicious post attempting an XSS attack
    post("Eve", "<script>alert('You have been hacked!');</script>");

    // Test Case 5: Post with quotes
    post("David", "He said, \"It's a 'nice' day.\"");

    // Display all the messages at the end
    display_all_messages();

    return 0;
}