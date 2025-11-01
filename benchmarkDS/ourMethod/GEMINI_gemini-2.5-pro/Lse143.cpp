#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <mutex>

// In a real application, this would be a persistent database.
// Using std::unordered_set for efficient lookups and std::mutex for thread-safety.
static std::unordered_set<std::string> database;
static std::mutex db_mutex;

// A simple regex for email validation.
const std::regex EMAIL_REGEX("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$");
const size_t MAX_EMAIL_LENGTH = 254;

/**
 * @brief Unsubscribes a user by removing their email from the database.
 *
 * @param email The email address of the user to unsubscribe.
 * @return A string message indicating the result of the operation.
 */
std::string unsubscribe(const std::string& email) {
    // Rule #1: Ensure all input is validated.
    if (email.empty()) {
        return "Error: Email parameter cannot be empty.";
    }
    if (email.length() > MAX_EMAIL_LENGTH) {
        return "Error: Email address is too long.";
    }
    if (!std::regex_match(email, EMAIL_REGEX)) {
        return "Error: Invalid email format.";
    }

    // Use a lock_guard for exception-safe, thread-safe access to the database.
    std::lock_guard<std::mutex> guard(db_mutex);

    auto it = database.find(email);
    if (it != database.end()) {
        database.erase(it);
        // In a real application, you would also perform other actions,
        // e.g., unsubscribing from a third-party mailing list service.
        return "User unsubscribed successfully.";
    } else {
        return "User is not subscribed.";
    }
}

void print_database() {
    std::cout << "   Database state: { ";
    for (const auto& entry : database) {
        std::cout << "\"" << entry << "\" ";
    }
    std::cout << "}" << std::endl;
}

int main() {
    // Populate the database for testing
    database.insert("test@example.com");
    database.insert("user1@domain.com");
    database.insert("another.user@service.net");

    std::cout << "Initial database state:" << std::endl;
    print_database();
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: A valid email that exists in the database.
    std::string email1 = "test@example.com";
    std::cout << "\n1. Unsubscribing '" << email1 << "':" << std::endl;
    std::string result1 = unsubscribe(email1);
    std::cout << "   Result: " << result1 << std::endl;
    print_database();

    // Test Case 2: The same email, which should now be gone.
    std::cout << "\n2. Unsubscribing '" << email1 << "' again:" << std::endl;
    std::string result2 = unsubscribe(email1);
    std::cout << "   Result: " << result2 << std::endl;

    // Test Case 3: A valid email that is not in the database.
    std::string email3 = "notfound@example.com";
    std::cout << "\n3. Unsubscribing '" << email3 << "':" << std::endl;
    std::string result3 = unsubscribe(email3);
    std::cout << "   Result: " << result3 << std::endl;

    // Test Case 4: An invalid email format.
    std::string email4 = "invalid-email";
    std::cout << "\n4. Unsubscribing '" << email4 << "':" << std::endl;
    std::string result4 = unsubscribe(email4);
    std::cout << "   Result: " << result4 << std::endl;

    // Test Case 5: An empty email input.
    std::string email5 = "";
    std::cout << "\n5. Unsubscribing an empty email:" << std::endl;
    std::string result5 = unsubscribe(email5);
    std::cout << "   Result: " << result5 << std::endl;
    
    std::cout << "\n--- Test Cases Complete ---" << std::endl;
    std::cout << "Final database state:" << std::endl;
    print_database();

    return 0;
}