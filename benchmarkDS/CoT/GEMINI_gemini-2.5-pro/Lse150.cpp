#include <iostream>
#include <vector>
#include <string>
#include <utility> // For std::pair

// In a real web application, this would be a database connection.
// Here, we use a global vector to simulate a database.
struct Message {
    std::string username;
    std::string message;
};
std::vector<Message> messageDatabase;

/**
 * @brief Sanitizes a string to prevent XSS by replacing HTML special characters.
 * @param input The raw user input string.
 * @return A new string that is safe to display in HTML.
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
 * @brief Simulates inserting a user message into the database.
 * This approach is inherently safe from SQL Injection. For a real SQL DB,
 * use prepared statements.
 */
void insert_user_message_in_db(const std::string& username, const std::string& message) {
    std::cout << "[DB] Inserting message for user: " << username << std::endl;
    messageDatabase.push_back({username, message});
}

// Helper function to trim whitespace from the start and end of a string
std::string trim(const std::string& str) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}

/**
 * @brief Simulates handling a POST request from a user.
 */
void post(const std::string& username, const std::string& message) {
    std::cout << "\n--- Handling POST Request ---" << std::endl;

    std::string trimmed_username = trim(username);
    std::string trimmed_message = trim(message);

    // Security: Validate input to prevent empty submissions.
    if (trimmed_username.empty() || trimmed_message.empty()) {
        std::cout << "Error: Username and message cannot be empty." << std::endl;
        std::cout << "--- End POST Request ---" << std::endl;
        return;
    }

    insert_user_message_in_db(trimmed_username, trimmed_message);
    
    std::cout << "Action: Message posted successfully." << std::endl;
    std::cout << "Action: Redirecting user to main page..." << std::endl;
    std::cout << "--- End POST Request ---" << std::endl;
}

/**
 * @brief Simulates rendering the main page with messages and a form.
 */
void showMainPage() {
    std::cout << "\n======================================" << std::endl;
    std::cout << "         Welcome to the Main Page" << std::endl;
    std::cout << "======================================" << std::endl;

    std::cout << "\n--- Message Board ---" << std::endl;
    if (messageDatabase.empty()) {
        std::cout << "(No messages yet)" << std::endl;
    } else {
        for (const auto& msg : messageDatabase) {
            // Security: Sanitize all output to prevent stored XSS.
            std::string safeUsername = sanitize(msg.username);
            std::string safeMessage = sanitize(msg.message);
            std::cout << safeUsername << ": " << safeMessage << std::endl;
        }
    }
    std::cout << "--- End of Messages ---" << std::endl;

    // Simulate the submission form.
    std::cout << "\n--- Submit a New Message (Form) ---" << std::endl;
    std::cout << "[ Username: ___________ ]" << std::endl;
    std::cout << "[ Message:  ___________ ]" << std::endl;
    std::cout << "[ [SUBMIT] ]" << std::endl;
    std::cout << "======================================\n" << std::endl;
}

int main() {
    // Initially, show the empty main page.
    showMainPage();

    // --- Test Cases ---

    // Test Case 1: A standard, valid post.
    std::cout << ">>> TEST CASE 1: Standard Post" << std::endl;
    post("Alice", "Hello everyone!");
    showMainPage();

    // Test Case 2: Another standard post.
    std::cout << ">>> TEST CASE 2: Another User Post" << std::endl;
    post("Bob", "Hi Alice, how are you?");
    showMainPage();

    // Test Case 3: A post with special HTML characters (XSS attempt).
    std::cout << ">>> TEST CASE 3: XSS Attempt" << std::endl;
    post("Mallory", "<script>alert('pwned')</script>");
    showMainPage();

    // Test Case 4: A post with only whitespace (should be rejected).
    std::cout << ">>> TEST CASE 4: Empty Message" << std::endl;
    post("Charlie", "      ");
    showMainPage();

    // Test Case 5: A post with a mix of content.
    std::cout << ">>> TEST CASE 5: Mixed Content" << std::endl;
    post("David", "I think 5 > 3 & 2 < 4. What do you think?");
    showMainPage();

    return 0;
}