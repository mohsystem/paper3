
#include <iostream>
#include <string>
#include <map>
#include <memory>

class User {
private:
    std::string username;
    std::string password;
    bool isAdmin;

public:
    User(const std::string& username, const std::string& password, bool isAdmin)
        : username(username), password(password), isAdmin(isAdmin) {}

    std::string getUsername() const {
        return username;
    }

    bool checkPassword(const std::string& pwd) const {
        return password == pwd;
    }

    bool getIsAdmin() const {
        return isAdmin;
    }
};

class Session {
private:
    std::shared_ptr<User> currentUser;

public:
    Session() : currentUser(nullptr) {}

    void login(std::shared_ptr<User> user) {
        currentUser = user;
    }

    void logout() {
        currentUser = nullptr;
    }

    bool isLoggedIn() const {
        return currentUser != nullptr;
    }

    std::shared_ptr<User> getCurrentUser() const {
        return currentUser;
    }
};

std::string redirectUser(const Session& session) {
    if (!session.isLoggedIn()) {
        return "Redirect to: /login";
    }

    auto user = session.getCurrentUser();
    if (user->getIsAdmin()) {
        return "Redirect to: /admin";
    } else {
        return "Redirect to: /user";
    }
}

bool authenticate(const std::string& username, const std::string& password,
                  const std::map<std::string, std::shared_ptr<User>>& userDatabase) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end() && it->second->checkPassword(password)) {
        return true;
    }
    return false;
}

int main() {
    // Create user database
    std::map<std::string, std::shared_ptr<User>> userDatabase;
    userDatabase["admin"] = std::make_shared<User>("admin", "admin123", true);
    userDatabase["john"] = std::make_shared<User>("john", "john123", false);
    userDatabase["jane"] = std::make_shared<User>("jane", "jane123", false);

    Session session;

    // Test Case 1: Not logged in
    std::cout << "Test Case 1 - Not logged in:" << std::endl;
    std::cout << redirectUser(session) << std::endl;
    std::cout << std::endl;

    // Test Case 2: Logged in as regular user (john)
    std::cout << "Test Case 2 - Logged in as regular user (john):" << std::endl;
    if (authenticate("john", "john123", userDatabase)) {
        session.login(userDatabase["john"]);
    }
    std::cout << redirectUser(session) << std::endl;
    session.logout();
    std::cout << std::endl;

    // Test Case 3: Logged in as admin
    std::cout << "Test Case 3 - Logged in as admin:" << std::endl;
    if (authenticate("admin", "admin123", userDatabase)) {
        session.login(userDatabase["admin"]);
    }
    std::cout << redirectUser(session) << std::endl;
    session.logout();
    std::cout << std::endl;

    // Test Case 4: Failed login attempt
    std::cout << "Test Case 4 - Failed login attempt:" << std::endl;
    if (authenticate("john", "wrongpassword", userDatabase)) {
        session.login(userDatabase["john"]);
    }
    std::cout << redirectUser(session) << std::endl;
    std::cout << std::endl;

    // Test Case 5: Logged in as another regular user (jane)
    std::cout << "Test Case 5 - Logged in as regular user (jane):" << std::endl;
    if (authenticate("jane", "jane123", userDatabase)) {
        session.login(userDatabase["jane"]);
    }
    std::cout << redirectUser(session) << std::endl;
    std::cout << std::endl;

    return 0;
}
