#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

/**
 * Checks if an email is in the database and removes it if found.
 *
 * @param database A reference to an unordered_set of strings representing the email database.
 * @param email The email address to unsubscribe.
 * @return A string indicating the result of the operation.
 */
std::string unsubscribe(std::unordered_set<std::string>& database, const std::string& email) {
    if (database.count(email)) {
        database.erase(email);
        return email + " has been unsubscribed successfully.";
    } else {
        return email + " is not in the database.";
    }
}

// Helper function to print the contents of the database
void print_database(const std::unordered_set<std::string>& db) {
    std::cout << "{ ";
    for (const auto& email : db) {
        std::cout << "\"" << email << "\" ";
    }
    std::cout << "}" << std::endl;
}

int main() {
    // Simulate a database with an unordered_set
    std::unordered_set<std::string> emailDatabase = {
        "user1@example.com",
        "user2@example.com",
        "user3@example.com"
    };

    std::cout << "Initial database: ";
    print_database(emailDatabase);

    // --- 5 Test Cases ---
    std::vector<std::string> testEmails = {
        "user2@example.com",     // Case 1: Unsubscribe an existing email.
        "user4@example.com",     // Case 2: Try to unsubscribe an email that does not exist.
        "user1@example.com",     // Case 3: Unsubscribe another existing email.
        "user2@example.com",     // Case 4: Try to unsubscribe the first email again.
        "user3@example.com"      // Case 5: Unsubscribe the last remaining email.
    };
    
    for (int i = 0; i < testEmails.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Attempting to unsubscribe: " << testEmails[i] << std::endl;
        std::string result = unsubscribe(emailDatabase, testEmails[i]);
        std::cout << "Result: " << result << std::endl;
        std::cout << "Current database: ";
        print_database(emailDatabase);
    }

    return 0;
}