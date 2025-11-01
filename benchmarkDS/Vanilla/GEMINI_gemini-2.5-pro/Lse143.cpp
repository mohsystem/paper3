#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

// Simulating a database of subscribed emails
std::unordered_set<std::string> emailDatabase;

/**
 * Unsubscribes a user by removing their email from the database.
 *
 * @param email The email address to unsubscribe.
 * @return A string indicating the result of the operation.
 */
std::string unsubscribe(const std::string& email) {
    if (emailDatabase.count(email)) {
        emailDatabase.erase(email);
        return "User with email '" + email + "' unsubscribed successfully.";
    } else {
        return "User with email '" + email + "' is not subscribed.";
    }
}

void printDatabase() {
    std::cout << "Current database: { ";
    for (const auto& email : emailDatabase) {
        std::cout << email << " ";
    }
    std::cout << "}\n" << std::endl;
}

int main() {
    // Pre-populate the database
    emailDatabase.insert("test1@example.com");
    emailDatabase.insert("user.two@domain.org");
    emailDatabase.insert("another@email.net");

    std::cout << "Initial database: { another@email.net user.two@domain.org test1@example.com }" << std::endl;
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Unsubscribe an existing user
    std::string email1 = "test1@example.com";
    std::cout << "Attempting to unsubscribe: " << email1 << std::endl;
    std::cout << "Result: " << unsubscribe(email1) << std::endl;
    printDatabase();

    // Test Case 2: Unsubscribe a non-existent user
    std::string email2 = "notfound@example.com";
    std::cout << "Attempting to unsubscribe: " << email2 << std::endl;
    std::cout << "Result: " << unsubscribe(email2) << std::endl;
    printDatabase();

    // Test Case 3: Unsubscribe another existing user
    std::string email3 = "another@email.net";
    std::cout << "Attempting to unsubscribe: " << email3 << std::endl;
    std::cout << "Result: " << unsubscribe(email3) << std::endl;
    printDatabase();

    // Test Case 4: Try to unsubscribe the first user again
    std::cout << "Attempting to unsubscribe: " << email1 << std::endl;
    std::cout << "Result: " << unsubscribe(email1) << std::endl;
    printDatabase();

    // Test Case 5: Unsubscribe the last user
    std::string email5 = "user.two@domain.org";
    std::cout << "Attempting to unsubscribe: " << email5 << std::endl;
    std::cout << "Result: " << unsubscribe(email5) << std::endl;
    printDatabase();

    return 0;
}