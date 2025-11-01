#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm> // For std::transform
#include <cctype>    // For ::tolower

// Helper function to convert a string to lowercase
std::string toLower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower_str;
}

/**
 * Simulates unsubscribing an email address from a database.
 * The database is represented by an unordered_set of strings.
 *
 * @param database The set of subscribed email addresses. This set will be modified.
 * @param email The email address to unsubscribe.
 * @return A status message indicating success or failure.
 */
std::string unsubscribe(std::unordered_set<std::string>& database, const std::string& email) {
    // Basic input validation
    if (email.empty() || email.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
        return "Invalid email address provided.";
    }

    // Normalize email to lowercase for case-insensitive comparison
    std::string normalizedEmail = toLower(email);
    
    // Check if the email is in the database and remove it
    if (database.count(normalizedEmail)) {
        database.erase(normalizedEmail);
        return "Successfully unsubscribed " + email + ".";
    } else {
        return "Email address " + email + " not found in the database.";
    }
}

void print_database(const std::unordered_set<std::string>& db) {
    std::cout << "{ ";
    for (const auto& entry : db) {
        std::cout << entry << " ";
    }
    std::cout << "}" << std::endl;
}

int main() {
    // 1. Initialize the simulated database with lowercase emails
    std::unordered_set<std::string> emailDatabase = {
        "user1@example.com",
        "user2@example.com",
        "subscriber@test.com",
        "another.user@domain.org"
    };

    std::cout << "Initial database: ";
    print_database(emailDatabase);

    // 2. Define test cases
    std::vector<std::string> testEmails = {
        "user1@example.com",      // Case 1: Email exists and should be removed.
        "user99@example.com",     // Case 2: Email does not exist.
        "user1@example.com",      // Case 3: Email should now be gone after the first test.
        "SUBSCRIBER@TEST.COM",  // Case 4: Test case-insensitivity.
        " "                       // Case 5: Whitespace-only string test.
    };
    
    // 3. Run test cases
    for (const auto& email : testEmails) {
        std::cout << "\nAttempting to unsubscribe: " << email << std::endl;
        std::string result = unsubscribe(emailDatabase, email);
        std::cout << "Result: " << result << std::endl;
        std::cout << "Current database: ";
        print_database(emailDatabase);
    }

    return 0;
}