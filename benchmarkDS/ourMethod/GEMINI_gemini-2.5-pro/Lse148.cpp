#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // For std::all_of
#include <cctype>    // For isspace

// Simulating a database table with an in-memory vector
struct Message {
    std::string username;
    std::string text;
};

std::vector<Message> messageDatabase;
const size_t MAX_USERNAME_LENGTH = 50;
const size_t MAX_MESSAGE_LENGTH = 280;

/**
 * @brief Sanitizes input to prevent basic injection attacks for display.
 * In a real web app, use a proper HTML escaping library.
 * @param input The string to sanitize.
 * @return The sanitized string.
 */
std::string sanitize(const std::string& input) {
    std::string output;
    output.reserve(input.length());
    for (char c : input) {
        switch (c) {
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '&': output += "&amp;"; break;
            case '"': output += "&quot;"; break;
            case '\'': output += "&#39;"; break;
            default: output += c; break;
        }
    }
    return output;
}

/**
 * @brief Checks if a string is null, empty, or contains only whitespace.
 * @param str The string to check.
 * @return True if the string is empty or whitespace, false otherwise.
 */
bool isStringBlank(const std::string& str) {
    return str.empty() || std::all_of(str.begin(), str.end(), [](unsigned char c){ return std::isspace(c); });
}


/**
 * @brief Inserts a user message into our simulated database.
 * @param username The username of the poster.
 * @param messageText The message content.
 * @return true if insertion was successful, false otherwise.
 */
bool insert_user_message_in_db(const std::string& username, const std::string& messageText) {
    // Rule #1: Ensure all input is validated and sanitized
    if (isStringBlank(username) || username.length() > MAX_USERNAME_LENGTH) {
        std::cerr << "Error: Invalid username." << std::endl;
        return false;
    }
    if (isStringBlank(messageText) || messageText.length() > MAX_MESSAGE_LENGTH) {
        std::cerr << "Error: Invalid message." << std::endl;
        return false;
    }

    // Sanitize input before storing
    std::string sanitizedUsername = sanitize(username);
    std::string sanitizedMessage = sanitize(messageText);

    messageDatabase.push_back({sanitizedUsername, sanitizedMessage});
    std::cout << "Message from '" << sanitizedUsername << "' stored successfully." << std::endl;
    return true;
}

/**
 * @brief Called when a user submits a message.
 * @param username The username from the request.
 * @param message The message from the request.
 */
void post(const std::string& username, const std::string& message) {
    std::cout << "\n--- Handling post request ---" << std::endl;
    // It calls the insert_user_message_in_db() function
    bool success = insert_user_message_in_db(username, message);
    
    if (success) {
        // It redirects the user to the main page (simulated)
        std::cout << "Redirecting user to the main page..." << std::endl;
    } else {
        std::cout << "Post failed. Not redirecting." << std::endl;
    }
}

/**
 * @brief Displays all messages currently in the database.
 */
void displayMessages() {
    std::cout << "\n--- Displaying All Messages ---" << std::endl;
    if (messageDatabase.empty()) {
        std::cout << "No messages to display." << std::endl;
    } else {
        for (const auto& msg : messageDatabase) {
            // Output is already sanitized from when it was inserted
            std::cout << msg.username << ": " << msg.text << std::endl;
        }
    }
    std::cout << "-----------------------------" << std::endl << std::endl;
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1: Valid post
    post("Alice", "Hello, world!");
    displayMessages();

    // Test Case 2: Another valid post
    post("Bob", "This is a test message.");
    displayMessages();

    // Test Case 3: Invalid post (empty message)
    post("Charlie", " ");
    displayMessages();

    // Test Case 4: Invalid post (username too long)
    std::string longUsername(MAX_USERNAME_LENGTH + 1, 'a');
    post(longUsername, "My username is too long.");
    displayMessages();
    
    // Test Case 5: Post with characters that should be sanitized
    post("Eve", "<script>alert('XSS')</script>");
    displayMessages();

    return 0;
}