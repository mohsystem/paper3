#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

// Simulating a database of subscribed users with a static unordered_set.
static std::unordered_set<std::string> database;

/**
 * Unsubscribes a user by removing their email from the database.
 *
 * @param email The email address of the user to unsubscribe.
 * @return A string indicating the result of the operation.
 */
std::string unsubscribe(const std::string& email) {
    // Security: Validate input to prevent processing empty data.
    if (email.empty() || email.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
        return "Invalid email address provided.";
    }

    // Check if the email is in the database.
    if (database.count(email)) {
        // If it exists, remove it (unsubscribe).
        database.erase(email);
        return "User with email " + email + " has been successfully unsubscribed.";
    } else {
        // If it does not exist, return a message.
        return "User with email " + email + " is not subscribed.";
    }
}

void printDatabaseState() {
    std::cout << "Database state: { ";
    for (const auto& email : database) {
        std::cout << email << " ";
    }
    std::cout << "}\n";
}

int main() {
    // Pre-populate the simulated database for testing.
    database = {"user1@example.com", "user2@example.com", "user3@example.com"};

    printDatabaseState();
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Unsubscribe an existing user.
    std::string testEmail1 = "user1@example.com";
    std::cout << "Test 1: Unsubscribing '" << testEmail1 << "'" << std::endl;
    std::string result1 = unsubscribe(testEmail1);
    std::cout << "Result: " << result1 << std::endl;
    printDatabaseState();
    std::cout << std::endl;

    // Test Case 2: Try to unsubscribe the same user again.
    std::string testEmail2 = "user1@example.com";
    std::cout << "Test 2: Unsubscribing '" << testEmail2 << "' again" << std::endl;
    std::string result2 = unsubscribe(testEmail2);
    std::cout << "Result: " << result2 << std::endl;
    printDatabaseState();
    std::cout << std::endl;

    // Test Case 3: Unsubscribe a user that is not in the database.
    std::string testEmail3 = "nonexistent@example.com";
    std::cout << "Test 3: Unsubscribing '" << testEmail3 << "'" << std::endl;
    std::string result3 = unsubscribe(testEmail3);
    std::cout << "Result: " << result3 << std::endl;
    printDatabaseState();
    std::cout << std::endl;

    // Test Case 4: Unsubscribe with an empty string.
    std::string testEmail4 = "";
    std::cout << "Test 4: Unsubscribing with an empty email" << std::endl;
    std::string result4 = unsubscribe(testEmail4);
    std::cout << "Result: " << result4 << std::endl;
    printDatabaseState();
    std::cout << std::endl;
    
    // Test Case 5: Unsubscribe with a whitespace string.
    std::string testEmail5 = "   ";
    std::cout << "Test 5: Unsubscribing with a whitespace email" << std::endl;
    std::string result5 = unsubscribe(testEmail5);
    std::cout << "Result: " << result5 << std::endl;
    printDatabaseState();
    std::cout << std::endl;

    return 0;
}