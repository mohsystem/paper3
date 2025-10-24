#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>

// A simple structure to hold user data
struct User {
    std::string name;
    std::string email;
    std::string hashedPassword; // In a real app, this would be a byte array
    std::string salt;

    User(std::string n, std::string e, std::string ph, std::string s)
        : name(std::move(n)), email(std::move(e)), hashedPassword(std::move(ph)), salt(std::move(s)) {}
};

class Task46 {
private:
    // In-memory map to act as a database
    std::map<std::string, User> userDatabase;
    const int MIN_PASSWORD_LENGTH = 8;
    // Basic regex for email validation
    const std::regex EMAIL_REGEX{"(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"};

    /**
     * WARNING: THIS IS A PLACEHOLDER HASHING FUNCTION FOR DEMONSTRATION ONLY.
     * DO NOT USE IN A PRODUCTION ENVIRONMENT.
     * In a real application, use a well-vetted, strong cryptographic library
     * like OpenSSL, libsodium, or bcrypt for password hashing.
     */
    std::string hashPassword(const std::string& password, const std::string& salt) {
        // This is a dummy hash. It's not secure.
        return "hashed_" + password + "_" + salt;
    }

    // Dummy salt generation
    std::string generateSalt() {
        return "dummy_salt_123";
    }

public:
    /**
     * Registers a new user after validating the inputs.
     * @param name The user's name.
     * @param email The user's email.
     * @param password The user's password.
     * @return A string indicating the result of the registration attempt.
     */
    std::string registerUser(const std::string& name, const std::string& email, const std::string& password) {
        // 1. Input Validation
        if (name.empty() || std::all_of(name.begin(), name.end(), isspace)) {
            return "Registration failed: Name cannot be empty.";
        }
        if (!std::regex_match(email, EMAIL_REGEX)) {
            return "Registration failed: Invalid email format.";
        }
        if (password.length() < MIN_PASSWORD_LENGTH) {
            return "Registration failed: Password must be at least " + std::to_string(MIN_PASSWORD_LENGTH) + " characters long.";
        }
        if (userDatabase.count(email)) {
            return "Registration failed: Email already exists.";
        }

        // 2. Secure Password Handling (using placeholder functions)
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);

        // 3. Store User Information
        User newUser(name, email, hashedPassword, salt);
        userDatabase.insert({email, newUser});

        return "User '" + name + "' registered successfully.";
    }

    void printDatabase() {
        std::cout << "\nCurrent users in database:" << std::endl;
        for (const auto& pair : userDatabase) {
            std::cout << "- User{name='" << pair.second.name << "', email='" << pair.second.email << "'}" << std::endl;
        }
    }
};

int main() {
    Task46 registrationSystem;
    std::cout << "--- User Registration System Test Cases ---" << std::endl;

    // Test Case 1: Successful registration
    std::cout << "1. " << registrationSystem.registerUser("Alice Smith", "alice@example.com", "Password123!") << std::endl;

    // Test Case 2: Attempt to register with an existing email
    std::cout << "2. " << registrationSystem.registerUser("Alice Jones", "alice@example.com", "AnotherPassword456") << std::endl;

    // Test Case 3: Attempt to register with an invalid email format
    std::cout << "3. " << registrationSystem.registerUser("Bob Johnson", "bob-at-invalid", "SecurePass789") << std::endl;

    // Test Case 4: Attempt to register with a weak (short) password
    std::cout << "4. " << registrationSystem.registerUser("Charlie Brown", "charlie@example.com", "pass") << std::endl;

    // Test Case 5: Attempt to register with an empty name
    std::cout << "5. " << registrationSystem.registerUser(" ", "david@example.com", "ValidPassword101") << std::endl;

    // Print database contents (for verification)
    registrationSystem.printDatabase();

    return 0;
}