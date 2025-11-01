#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <ctime>

// WARNING: This code uses placeholder functions for cryptography for demonstration.
// In a real-world application, ALWAYS use a vetted cryptographic library
// like OpenSSL, libsodium, or a platform-specific API (e.g., CAPI/CNG on Windows).
// DO NOT use this placeholder cryptographic code in production.

// Placeholder for a cryptographically secure random number generator.
// In production, use OpenSSL's RAND_bytes or equivalent.
std::vector<unsigned char> placeholder_generate_salt(size_t size) {
    std::vector<unsigned char> salt(size);
    for (size_t i = 0; i < size; ++i) {
        salt[i] = static_cast<unsigned char>(rand() % 256);
    }
    return salt;
}

// Placeholder for a strong key derivation function like PBKDF2, scrypt or Argon2.
// This function is INSECURE and for demonstration of logic ONLY.
std::vector<unsigned char> placeholder_hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash;
    // A real implementation would use a KDF here. This is a trivial example.
    for (char c : password) {
        hash.push_back(static_cast<unsigned char>(c));
    }
    hash.insert(hash.end(), salt.begin(), salt.end());
    return hash;
}

// Performs a constant-time comparison of two byte vectors to prevent timing attacks.
bool constant_time_compare(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    unsigned char result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Clears a string's memory. Using a volatile pointer discourages compiler optimization.
void secure_clear_string(std::string& s) {
    if (s.empty()) return;
    // Using volatile to signal to the compiler that this operation must not be optimized out.
    volatile char* p = &s[0];
    for (size_t i = 0; i < s.length(); ++i) {
        p[i] = 0;
    }
    s.clear();
}

class User {
private:
    std::string username;
    std::string email;
    std::vector<unsigned char> passwordHash;
    std::vector<unsigned char> salt;

public:
    User(const std::string& uname, const std::string& mail, std::string& password)
        : username(uname), email(mail) {
        if (uname.empty() || mail.empty() || password.empty()) {
            throw std::invalid_argument("Username, email, and password cannot be empty.");
        }
        salt = placeholder_generate_salt(16);
        passwordHash = placeholder_hash_password(password, salt);
    }

    bool verifyPassword(std::string& password) {
        if (password.empty()) return false;
        std::vector<unsigned char> newHash = placeholder_hash_password(password, salt);
        return constant_time_compare(passwordHash, newHash);
    }

    const std::string& getUsername() const { return username; }
    const std::string& getEmail() const { return email; }
    void setEmail(const std::string& newEmail) { email = newEmail; }
};

class LoginSystem {
private:
    User* currentUser;
    bool isLoggedIn;

public:
    LoginSystem() : currentUser(nullptr), isLoggedIn(false) {}

    ~LoginSystem() {
        delete currentUser;
    }
    
    // Disable copy constructor and assignment operator
    LoginSystem(const LoginSystem&) = delete;
    LoginSystem& operator=(const LoginSystem&) = delete;

    bool registerUser(const std::string& username, const std::string& email, std::string& password) {
        delete currentUser;
        currentUser = nullptr;
        try {
            currentUser = new User(username, email, password);
            isLoggedIn = false;
            std::cout << "User " << username << " registered successfully." << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Registration error: " << e.what() << std::endl;
            return false;
        } finally {
            secure_clear_string(password);
        }
        return true;
    }

    bool login(const std::string& username, std::string& password) {
        isLoggedIn = false;
        if (!currentUser || currentUser->getUsername() != username) {
            std::cout << "Login failed: User not found." << std::endl;
        } else if (currentUser->verifyPassword(password)) {
            isLoggedIn = true;
            std::cout << "Login successful for user " << username << "." << std::endl;
        } else {
            std::cout << "Login failed: Incorrect password." << std::endl;
        }
        secure_clear_string(password);
        return isLoggedIn;
    }

    void logout() {
        isLoggedIn = false;
        std::cout << "User logged out." << std::endl;
    }

    bool changeEmail(const std::string& oldEmail, std::string& password, const std::string& newEmail) {
        bool success = false;
        if (!isLoggedIn) {
            std::cout << "Email change failed: User is not logged in." << std::endl;
        } else if (newEmail.empty()) {
            std::cout << "Email change failed: New email cannot be empty." << std::endl;
        } else if (currentUser->getEmail() != oldEmail) {
            std::cout << "Email change failed: Old email does not match." << std::endl;
        } else if (!currentUser->verifyPassword(password)) {
            std::cout << "Email change failed: Incorrect password." << std::endl;
        } else {
            currentUser->setEmail(newEmail);
            std::cout << "Email for " << currentUser->getUsername() << " changed successfully to " << newEmail << "." << std::endl;
            success = true;
        }
        secure_clear_string(password);
        return success;
    }

    const User* getCurrentUser() const {
        return isLoggedIn ? currentUser : nullptr;
    }
};

int main() {
    // Seed for placeholder random generator. NOT for production use.
    srand(time(0)); 

    LoginSystem system;

    std::string initial_pass = "Password123!";
    system.registerUser("testuser", "initial@example.com", initial_pass);

    std::cout << "\n--- Test Case 1: Successful Email Change ---" << std::endl;
    std::string pass1 = "Password123!";
    system.login("testuser", pass1);
    if (auto user = system.getCurrentUser()) {
        std::cout << "Current email: " << user->getEmail() << std::endl;
        std::string pass_change = "Password123!";
        system.changeEmail("initial@example.com", pass_change, "new@example.com");
        std::cout << "New email: " << user->getEmail() << std::endl;
    }
    system.logout();

    std::cout << "\n--- Test Case 2: Change Email with Wrong Password ---" << std::endl;
    std::string pass2 = "Password123!";
    system.login("testuser", pass2);
    if (auto user = system.getCurrentUser()) {
        std::cout << "Current email: " << user->getEmail() << std::endl;
        std::string wrong_pass = "WrongPassword!";
        system.changeEmail("new@example.com", wrong_pass, "another@example.com");
        std::cout << "Email after failed attempt: " << user->getEmail() << std::endl;
    }
    system.logout();

    std::cout << "\n--- Test Case 3: Change Email with Wrong Old Email ---" << std::endl;
    std::string pass3 = "Password123!";
    system.login("testuser", pass3);
    if (auto user = system.getCurrentUser()) {
        std::cout << "Current email: " << user->getEmail() << std::endl;
        std::string pass_change3 = "Password123!";
        system.changeEmail("wrong.old@example.com", pass_change3, "another@example.com");
        std::cout << "Email after failed attempt: " << user->getEmail() << std::endl;
    }
    system.logout();
    
    std::cout << "\n--- Test Case 4: Change Email When Not Logged In ---" << std::endl;
    std::string pass4 = "Password123!";
    system.changeEmail("new@example.com", pass4, "another@example.com");
    std::string pass4_login = "Password123!";
    system.login("testuser", pass4_login);
    if (auto user = system.getCurrentUser()) {
         std::cout << "Email after failed attempt: " << user->getEmail() << std::endl;
    }
    system.logout();

    std::cout << "\n--- Test Case 5: Login with Wrong Password ---" << std::endl;
    std::string pass5 = "NotThePassword";
    system.login("testuser", pass5);
    std::string pass5_change = "Password123!";
    system.changeEmail("new@example.com", pass5_change, "another@example.com");

    return 0;
}