#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstddef> // For size_t

// Constants for secure input validation
const size_t MAX_USERNAME_LENGTH = 50;
const size_t MAX_MESSAGE_LENGTH = 280;

// Simulated in-memory database.
// A static global variable makes it accessible by functions in this file.
static std::map<std::string, std::vector<std::string>> userMessagesDb;

/**
 * Inserts a user's message into the simulated database.
 * In a real application, this is where secure database interaction would happen.
 * @param username The validated user's name.
 * @param message The validated message content.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    std::cout << "DB: Inserting message for user '" << username << "'." << std::endl;
    // SECURITY: In a real application, use parameterized queries or prepared statements
    // to prevent SQL injection. NEVER build a query by concatenating user input.
    
    userMessagesDb[username].push_back(message);
}

/**
 * Checks if a C++ string is effectively blank (contains only whitespace).
 * @param str The string to check.
 * @return True if the string is blank, false otherwise.
 */
bool isStringBlank(const std::string& str) {
    return str.find_first_not_of(" \t\n\r") == std::string::npos;
}

/**
 * Simulates handling a POST request to submit a message.
 * It gets the username and message, performs validation, calls the DB insert,
 * and simulates a redirect.
 * @param username The username from the request.
 * @param message The message from the request.
 * @return A string indicating the result, simulating a redirect URL.
 */
std::string post(const std::string& username, const std::string& message) {
    // SECURITY: Validate all user input before processing.
    // 1. Check for effectively empty strings. std::string is not nullable.
    if (isStringBlank(username)) {
        std::cerr << "Error: Username cannot be empty." << std::endl;
        return "redirect:/main?error=invalid_username";
    }
    if (isStringBlank(message)) {
        std::cerr << "Error: Message cannot be empty." << std::endl;
        return "redirect:/main?error=invalid_message";
    }

    // 2. Enforce length limits. std::string protects against buffer overflows.
    if (username.length() > MAX_USERNAME_LENGTH) {
        std::cerr << "Error: Username is too long." << std::endl;
        return "redirect:/main?error=username_too_long";
    }
    if (message.length() > MAX_MESSAGE_LENGTH) {
        std::cerr << "Error: Message is too long." << std::endl;
        return "redirect:/main?error=message_too_long";
    }

    // SECURITY: To prevent Cross-Site Scripting (XSS), user-provided content
    // should be properly HTML-escaped before being rendered on a web page.
    // This function only handles storing data, not rendering it.

    insert_user_message_in_db(username, message);
    
    std::cout << "Success! Redirecting to the main page." << std::endl;
    return "redirect:/main";
}

class Lse150 {
public:
    /**
     * The main method simulates a user interface (like a form on a main page)
     * and runs several test cases.
     */
    static void runTests() {
        std::cout << "--- Running Test Cases ---" << std::endl;

        // Test Case 1: Valid submission
        std::cout << "\n--- Test Case 1: Valid submission ---" << std::endl;
        post("alice", "Hello, world!");

        // Test Case 2: Another valid submission from a different user
        std::cout << "\n--- Test Case 2: Another valid user ---" << std::endl;
        post("bob", "This is a test message.");

        // Test Case 3: Message is too long
        std::cout << "\n--- Test Case 3: Message too long ---" << std::endl;
        std::string longMessage(300, 'a');
        post("alice", longMessage);

        // Test Case 4: Empty username (whitespace only)
        std::cout << "\n--- Test Case 4: Empty username ---" << std::endl;
        post("  ", "This should fail.");

        // Test Case 5: Potentially malicious input
        std::cout << "\n--- Test Case 5: Input with special characters ---" << std::endl;
        post("charlie", "<script>alert('xss')</script>");

        std::cout << "\n--- Final Database State ---" << std::endl;
        for (const auto& pair : userMessagesDb) {
            std::cout << "User: " << pair.first << std::endl;
            for (const auto& msg : pair.second) {
                std::cout << "  - " << msg << std::endl;
            }
        }
    }
};

int main() {
    Lse150::runTests();
    return 0;
}