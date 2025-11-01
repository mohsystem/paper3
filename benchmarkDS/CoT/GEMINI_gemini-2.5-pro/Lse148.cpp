#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

// Structure to hold a single message
struct Message {
    std::string username;
    std::string message;
};

// Simulating a database with a vector of Message structs
std::vector<Message> messageDatabase;

/**
 * Sanitizes user input to prevent basic XSS attacks.
 * Replaces special HTML characters with their corresponding entities.
 * @param input The string to sanitize.
 * @return The sanitized string.
 */
std::string sanitize(const std::string& input) {
    std::string output = "";
    output.reserve(input.length()); // Pre-allocate memory for efficiency
    for (char c : input) {
        switch (c) {
            case '&':  output += "&amp;";       break;
            case '<':  output += "&lt;";        break;
            case '>':  output += "&gt;";        break;
            case '"':  output += "&quot;";      break;
            case '\'': output += "&#x27;";      break;
            default:   output += c;             break;
        }
    }
    return output;
}

/**
 * Inserts a user's message into the simulated database.
 * @param username The sanitized username.
 * @param message The sanitized message.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    messageDatabase.push_back({username, message});
    std::cout << "LOG: Message from '" << username << "' stored in DB." << std::endl;
}

/**
 * Simulates handling a POST request to submit a message.
 * It gets username and message, sanitizes them, inserts them into the DB,
 * and simulates a redirect.
 * @param username The raw username from the request.
 * @param message The raw message from the request.
 */
void post(const std::string& username, const std::string& message) {
    // 1. Input validation
    // Check if string is empty or contains only whitespace
    bool isUsernameEmpty = std::all_of(username.begin(), username.end(), isspace);
    bool isMessageEmpty = std::all_of(message.begin(), message.end(), isspace);

    if (username.empty() || isUsernameEmpty || message.empty() || isMessageEmpty) {
        std::cout << "ERROR: Username or message cannot be empty." << std::endl;
        return;
    }

    // 2. Sanitize inputs to prevent XSS
    std::string sanitizedUsername = sanitize(username);
    std::string sanitizedMessage = sanitize(message);

    // 3. Insert into database
    insert_user_message_in_db(sanitizedUsername, sanitizedMessage);

    // 4. Redirect
    std::cout << "Redirecting user to the main page..." << std::endl << std::endl;
}

/**
 * Displays all the messages currently stored in the database.
 */
void displayMessages() {
    std::cout << "--- Displaying All Messages ---" << std::endl;
    if (messageDatabase.empty()) {
        std::cout << "No messages to display." << std::endl;
    } else {
        for (const auto& entry : messageDatabase) {
            std::cout << entry.username << ": " << entry.message << std::endl;
        }
    }
    std::cout << "-----------------------------" << std::endl << std::endl;
}

int main() {
    std::cout << "Initial State:" << std::endl;
    displayMessages();

    // --- Test Cases ---
    std::cout << ">>> Running Test Cases <<<" << std::endl;

    // Test Case 1: Standard post
    post("Alice", "Hello World!");

    // Test Case 2: Another standard post
    post("Bob", "This is a test message.");

    // Test Case 3: Post with characters that need sanitization
    post("Charlie", "<script>alert('xss')</script>");

    // Test Case 4: Post with an empty message (should be rejected)
    post("David", "  ");

    // Test Case 5: Post from a user who posted before
    post("Alice", "I am posting again!");

    std::cout << ">>> Test Cases Finished <<<" << std::endl << std::endl;

    std::cout << "Final State:" << std::endl;
    displayMessages();

    return 0;
}