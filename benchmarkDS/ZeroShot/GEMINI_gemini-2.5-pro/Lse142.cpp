#include <iostream>
#include <string>
#include <unordered_set>
#include <regex>

/**
 * Unsubscribes an email address after validating its format.
 *
 * @param email The email address to unsubscribe.
 * @param database A reference to the subscriber database (unordered_set).
 * @return A string containing the status message.
 */
std::string unsubscribe(const std::string& email, std::unordered_set<std::string>& database) {
    // 1. Input Validation: Check for empty string to prevent errors.
    if (email.empty()) {
        return "Error: Email address cannot be empty.";
    }

    // 2. Input Validation: Use regex to check for valid email format.
    const std::regex email_pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!std::regex_match(email, email_pattern)) {
        return "Error: Invalid email address format.";
    }
    
    // --- Simulated Database Interaction ---
    // SECURITY NOTE: In a real C++ application, use a database library that supports
    // prepared statements (parameterized queries) to prevent SQL injection.
    // Example using a hypothetical DB library:
    // db::PreparedStatement stmt("DELETE FROM subscribers WHERE email = ?");
    // stmt.bind(1, email);
    // stmt.execute();

    // 3. Business Logic: Find and remove the email from the set.
    auto it = database.find(email);
    if (it != database.end()) {
        database.erase(it);
        return "Success: " + email + " has been unsubscribed.";
    } else {
        return "Info: " + email + " was not found in our database.";
    }
}

// Helper function to print the database contents for testing.
void print_database(const std::unordered_set<std::string>& db) {
    std::cout << "{ ";
    for (const auto& entry : db) {
        std::cout << "\"" << entry << "\" ";
    }
    std::cout << "}" << std::endl;
}


int main() {
    // Simulating a database table of subscribers using an unordered_set for efficiency.
    std::unordered_set<std::string> subscriberDatabase = {
        "test@example.com",
        "user1@domain.com",
        "another.user@service.net"
    };

    std::cout << "Initial database: ";
    print_database(subscriberDatabase);
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Unsubscribe an existing email
    std::cout << "1. " << unsubscribe("test@example.com", subscriberDatabase) << std::endl;
    std::cout << "   Database after: ";
    print_database(subscriberDatabase);

    // Test Case 2: Attempt to unsubscribe an email that is not in the database
    std::cout << "2. " << unsubscribe("nonexistent@example.com", subscriberDatabase) << std::endl;
    std::cout << "   Database after: ";
    print_database(subscriberDatabase);

    // Test Case 3: Attempt to unsubscribe with an invalid email format
    std::cout << "3. " << unsubscribe("invalid-email", subscriberDatabase) << std::endl;
    std::cout << "   Database after: ";
    print_database(subscriberDatabase);
    
    // Test Case 4: Attempt to unsubscribe another existing email
    std::cout << "4. " << unsubscribe("user1@domain.com", subscriberDatabase) << std::endl;
    std::cout << "   Database after: ";
    print_database(subscriberDatabase);

    // Test Case 5: Attempt to unsubscribe with an empty string
    std::cout << "5. " << unsubscribe("", subscriberDatabase) << std::endl;
    std::cout << "   Database after: ";
    print_database(subscriberDatabase);

    return 0;
}