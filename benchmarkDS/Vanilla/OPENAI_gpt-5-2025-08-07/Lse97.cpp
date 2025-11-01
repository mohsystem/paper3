#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>

class Lse97 {
public:
    struct User {
        std::string firstName;
        std::string lastName;
        std::string email;
        std::string passwordHash;
    };

    static std::string registerUser(const std::string& username,
                                    const std::string& password,
                                    const std::string& firstName,
                                    const std::string& lastName,
                                    const std::string& email) {
        if (db.find(username) != db.end()) {
            return "Username already exists";
        }
        std::string hash = simpleHash(password);
        db[username] = User{firstName, lastName, email, hash};
        return "Registration succeeded";
    }

private:
    static std::unordered_map<std::string, User> db;

    static std::string simpleHash(const std::string& s) {
        unsigned long long h = 5381ULL;
        for (unsigned char c : s) {
            h = ((h << 5) + h) + c; // h * 33 + c
        }
        std::ostringstream oss;
        oss << std::hex << h;
        return oss.str();
    }
};

std::unordered_map<std::string, Lse97::User> Lse97::db;

int main() {
    std::cout << Lse97::registerUser("alice", "p@ss1", "Alice", "Wonder", "alice@example.com") << "\n";
    std::cout << Lse97::registerUser("bob", "12345", "Bob", "Builder", "bob@example.com") << "\n";
    std::cout << Lse97::registerUser("alice", "newpass", "Alice", "W", "alice2@example.com") << "\n";
    std::cout << Lse97::registerUser("charlie", "pw", "Char", "Lie", "charlie@example.com") << "\n";
    std::cout << Lse97::registerUser("bob", "other", "Bobby", "B", "bob@other.com") << "\n";
    return 0;
}