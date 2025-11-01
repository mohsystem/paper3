#include <iostream>
#include <string>
#include <map>

struct User {
    std::string password;
    bool isLoggedIn;
};

// Use a global map to simulate the user database
std::map<std::string, User> userDatabase;

void addUser(const std::string& email, const std::string& password) {
    userDatabase[email] = {password, false};
}

bool login(const std::string& email, const std::string& password) {
    auto it = userDatabase.find(email);
    if (it != userDatabase.end() && it->second.password == password) {
        it->second.isLoggedIn = true;
        return true;
    }
    return false;
}

void logout(const std::string& email) {
    auto it = userDatabase.find(email);
    if (it != userDatabase.end()) {
        it->second.isLoggedIn = false;
    }
}

std::string changeEmail(const std::string& oldEmail, const std::string& newEmail, const std::string& confirmPassword) {
    auto it = userDatabase.find(oldEmail);

    if (it == userDatabase.end()) {
        return "Error: Old email does not exist.";
    }

    if (!it->second.isLoggedIn) {
        return "Error: User must be logged in to change email.";
    }

    if (it->second.password != confirmPassword) {
        return "Error: Incorrect password.";
    }
    
    if (userDatabase.count(newEmail)) {
        return "Error: New email is already taken.";
    }

    // Move the user data to a new key
    User userData = it->second;
    userDatabase.erase(it);
    userDatabase[newEmail] = userData;
    return "Success: Email changed from " + oldEmail + " to " + newEmail + ".";
}

int main() {
    // Setup: Add some users to our database
    addUser("user1@example.com", "password123");
    addUser("user2@example.com", "securepass456");

    std::cout << "--- Test Case 1: Successful Email Change ---" << std::endl;
    login("user1@example.com", "password123");
    std::cout << "Login status for user1: " << std::boolalpha << userDatabase.at("user1@example.com").isLoggedIn << std::endl;
    std::string result1 = changeEmail("user1@example.com", "new.user1@example.com", "password123");
    std::cout << result1 << std::endl;
    std::cout << "Database contains new.user1@example.com: " << std::boolalpha << (userDatabase.count("new.user1@example.com") > 0) << std::endl;
    std::cout << "Database contains user1@example.com: " << std::boolalpha << (userDatabase.count("user1@example.com") > 0) << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 2: Failed Change (Incorrect Password) ---" << std::endl;
    login("user2@example.com", "securepass456");
    std::string result2 = changeEmail("user2@example.com", "another.user@example.com", "wrongpassword");
    std::cout << result2 << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Failed Change (User Not Logged In) ---" << std::endl;
    logout("user2@example.com"); // Make sure user is logged out
    std::string result3 = changeEmail("user2@example.com", "new.user2@example.com", "securepass456");
    std::cout << result3 << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 4: Failed Change (Old Email Not Found) ---" << std::endl;
    std::string result4 = changeEmail("nonexistent@example.com", "anything@example.com", "anypassword");
    std::cout << result4 << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 5: Logout and then Fail Change ---" << std::endl;
    std::cout << "User 'user2@example.com' attempts another change after logging out." << std::endl;
    logout("user2@example.com");
    std::cout << "Login status for user2: " << std::boolalpha << userDatabase.at("user2@example.com").isLoggedIn << std::endl;
    std::string result5 = changeEmail("user2@example.com", "final.user2@example.com", "securepass456");
    std::cout << result5 << std::endl;
    std::cout << std::endl;

    return 0;
}