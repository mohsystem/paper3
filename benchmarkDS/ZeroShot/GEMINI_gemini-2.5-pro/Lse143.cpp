#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>
#include <vector>

// A secure regex for email validation to prevent malformed input.
const std::regex EMAIL_REGEX("^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$");

/**
 * Checks if the given email has a valid format.
 * @param email The email to validate.
 * @return true if the email is valid, false otherwise.
 */
bool isValidEmail(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    return std::regex_match(email, EMAIL_REGEX);
}

/**
 * Unsubscribes a user by removing their email from a simulated database.
 * In a real application, this would interact with a persistent database and
 * should use parameterized queries to prevent SQL injection.
 * @param database An unordered_set representing the list of subscribed users.
 *                 This data structure provides efficient lookups and removals.
 * @param email The email of the user to unsubscribe.
 * @return A status message.
 */
std::string unsubscribe(std::unordered_set<std::string>& database, const std::string& email) {
    if (!isValidEmail(email)) {
        return "Error: Invalid email format provided.";
    }

    // The find() and erase() operations are efficient (average O(1)) on an unordered_set.
    if (database.count(email)) {
        database.erase(email);
        // In a real scenario, you would also trigger other actions,
        // e.g., removing them from a mailing list service via an API call.
        return "User with email '" + email + "' has been successfully unsubscribed.";
    } else {
        return "User with email '" + email + "' is not subscribed.";
    }
}

void printDatabase(const std::unordered_set<std::string>& db) {
    std::cout << "{ ";
    for (const auto& email : db) {
        std::cout << "\"" << email << "\" ";
    }
    std::cout << "}" << std::endl;
}

int main() {
    // 1. Initialize a simulated database of subscribed emails
    std::unordered_set<std::string> subscribedUsers = {
        "test.user@example.com",
        "jane.doe@example.org",
        "john.smith@email.net"
    };

    std::cout << "Initial database: ";
    printDatabase(subscribedUsers);
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Unsubscribe an existing user
    std::cout << "\nTest 1: Unsubscribing an existing user..." << std::endl;
    std::string result1 = unsubscribe(subscribedUsers, "jane.doe@example.org");
    std::cout << "Result: " << result1 << std::endl;
    std::cout << "Database state: ";
    printDatabase(subscribedUsers);

    // Test Case 2: Try to unsubscribe the same user again
    std::cout << "\nTest 2: Unsubscribing the same user again..." << std::endl;
    std::string result2 = unsubscribe(subscribedUsers, "jane.doe@example.org");
    std::cout << "Result: " << result2 << std::endl;
    std::cout << "Database state: ";
    printDatabase(subscribedUsers);

    // Test Case 3: Try to unsubscribe a user not in the database
    std::cout << "\nTest 3: Unsubscribing a non-existent user..." << std::endl;
    std::string result3 = unsubscribe(subscribedUsers, "not.found@example.com");
    std::cout << "Result: " << result3 << std::endl;
    std::cout << "Database state: ";
    printDatabase(subscribedUsers);

    // Test Case 4: Try to unsubscribe with an invalid email format
    std::cout << "\nTest 4: Unsubscribing with an invalid email..." << std::endl;
    std::string result4 = unsubscribe(subscribedUsers, "invalid-email");
    std::cout << "Result: " << result4 << std::endl;
    std::cout << "Database state: ";
    printDatabase(subscribedUsers);
    
    // Test Case 5: Try to unsubscribe with an empty email
    std::cout << "\nTest 5: Unsubscribing with an empty email..." << std::endl;
    std::string result5 = unsubscribe(subscribedUsers, "");
    std::cout << "Result: " << result5 << std::endl;
    std::cout << "Database state: ";
    printDatabase(subscribedUsers);

    return 0;
}