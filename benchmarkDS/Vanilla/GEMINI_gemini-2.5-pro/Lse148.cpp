#include <iostream>
#include <vector>
#include <string>

// A simple struct to hold message data
struct Message {
    std::string username;
    std::string message;
};

// Simulating a database table with a global vector
std::vector<Message> messagesDb;

/**
 * Inserts a user's message into our simulated database.
 * @param username The name of the user.
 * @param message The content of the message.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    messagesDb.push_back({username, message});
    std::cout << "DB: Message from '" << username << "' stored successfully." << std::endl;
}

/**
 * Simulates a user submitting a message.
 * It gets the username and message, inserts it into the DB, and redirects.
 * @param username The username from the request.
 * @param message The message from the request.
 */
void post(const std::string& username, const std::string& message) {
    std::cout << "\nPOST request received for user: " << username << std::endl;
    insert_user_message_in_db(username, message);
    // Simulate redirecting the user
    std::cout << "Action: Redirecting user to the main page..." << std::endl;
}

/**
 * Displays all the messages currently stored in the database.
 */
void displayMessages() {
    std::cout << "\n--- Main Message Board ---" << std::endl;
    if (messagesDb.empty()) {
        std::cout << "No messages to display." << std::endl;
    } else {
        for (const auto& msg : messagesDb) {
            std::cout << msg.username << ": " << msg.message << std::endl;
        }
    }
    std::cout << "--------------------------" << std::endl;
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1
    post("Alice", "Hello, world!");

    // Test Case 2
    post("Bob", "This is my first post.");

    // Test Case 3
    post("Charlie", "Does this thing work?");

    // Test Case 4
    post("Alice", "I'm posting again!");

    // Test Case 5
    post("Dave", "Hi everyone!");

    // Now, display all the messages that have been posted.
    displayMessages();

    return 0;
}