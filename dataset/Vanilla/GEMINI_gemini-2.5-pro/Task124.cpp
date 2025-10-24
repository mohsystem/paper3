// WARNING: This code is for educational purposes only and is NOT secure.
// It demonstrates a basic in-memory data storage concept.
// Do NOT use this for storing real sensitive data.
// Real-world applications require robust encryption, secure storage, and compliance with standards like PCI DSS.

#include <iostream>
#include <string>
#include <map>
#include <optional>
#include <cctype>

struct UserProfile {
    std::string name;
    std::string email;
    std::string creditCardNumber;
};

// This map acts as our simple in-memory, non-persistent data store.
using UserDatabase = std::map<std::string, UserProfile>;

/**
 * Returns a masked version of the credit card number.
 * @param ccNumber The raw credit card number string.
 * @return A masked string (e.g., "************4444").
 */
std::string getMaskedCreditCardNumber(const std::string& ccNumber) {
    if (ccNumber.length() <= 4) {
        return ccNumber;
    }
    std::string masked = "";
    for (size_t i = 0; i < ccNumber.length() - 4; ++i) {
        if (std::isdigit(ccNumber[i])) {
            masked += '*';
        } else {
            masked += ccNumber[i];
        }
    }
    masked += ccNumber.substr(ccNumber.length() - 4);
    return masked;
}

/**
 * Stores a user profile in the in-memory database.
 * @param database The map representing the database.
 * @param user The UserProfile object to store.
 */
void storeUserProfile(UserDatabase& database, const UserProfile& user) {
    database[user.name] = user;
}

/**
 * Retrieves a user profile from the in-memory database.
 * @param database The map representing the database.
 * @param username The name of the user to retrieve.
 * @return An std::optional containing the UserProfile if found, otherwise an empty optional.
 */
std::optional<UserProfile> retrieveUserProfile(const UserDatabase& database, const std::string& username) {
    auto it = database.find(username);
    if (it != database.end()) {
        return it->second;
    }
    return std::nullopt;
}

void displayUserProfile(const UserProfile& user) {
    std::cout << "UserProfile{name='" << user.name
              << "', email='" << user.email
              << "', creditCardNumber='" << getMaskedCreditCardNumber(user.creditCardNumber)
              << "'}" << std::endl;
}

int main() {
    UserDatabase userDatabase;

    std::cout << "--- Storing 5 User Profiles ---" << std::endl;

    // Test Case 1
    storeUserProfile(userDatabase, {"Alice", "alice@example.com", "1111-2222-3333-4444"});
    std::cout << "Stored: Alice" << std::endl;

    // Test Case 2
    storeUserProfile(userDatabase, {"Bob", "bob@example.com", "5555-6666-7777-8888"});
    std::cout << "Stored: Bob" << std::endl;
    
    // Test Case 3
    storeUserProfile(userDatabase, {"Charlie", "charlie@example.com", "9999-8888-7777-6666"});
    std::cout << "Stored: Charlie" << std::endl;

    // Test Case 4
    storeUserProfile(userDatabase, {"Diana", "diana@example.com", "1234-5678-9012-3456"});
    std::cout << "Stored: Diana" << std::endl;

    // Test Case 5
    storeUserProfile(userDatabase, {"Eve", "eve@example.com", "0000-1111-2222-3333"});
    std::cout << "Stored: Eve" << std::endl;

    std::cout << "\n--- Retrieving User Profiles ---" << std::endl;

    // Retrieval Test 1
    std::cout << "Retrieving 'Bob':" << std::endl;
    auto retrievedUser = retrieveUserProfile(userDatabase, "Bob");
    if (retrievedUser) {
        std::cout << "Found: ";
        displayUserProfile(*retrievedUser);
    } else {
        std::cout << "User 'Bob' not found." << std::endl;
    }

    // Retrieval Test 2
    std::cout << "\nRetrieving 'Diana':" << std::endl;
    retrievedUser = retrieveUserProfile(userDatabase, "Diana");
    if (retrievedUser.has_value()) {
        std::cout << "Found: ";
        displayUserProfile(retrievedUser.value());
    } else {
        std::cout << "User 'Diana' not found." << std::endl;
    }

    // Retrieval Test 3 (User not found)
    std::cout << "\nRetrieving 'Frank':" << std::endl;
    retrievedUser = retrieveUserProfile(userDatabase, "Frank");
    if (retrievedUser) {
        std::cout << "Found: ";
        displayUserProfile(*retrievedUser);
    } else {
        std::cout << "User 'Frank' not found." << std::endl;
    }

    return 0;
}