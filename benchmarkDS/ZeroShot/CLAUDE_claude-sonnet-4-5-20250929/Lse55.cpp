
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <ctime>
#include <cstdlib>

class User {
public:
    std::string username;
    std::string passwordHash;
    std::string role;
    
    User() {}
    User(const std::string& u, const std::string& p, const std::string& r)
        : username(u), passwordHash(p), role(r) {}
};

class Lse55 {
private:
    static std::map<std::string, User> users;
    static std::map<std::string, std::string> sessions;
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    static std::string generateSessionId() {
        std::stringstream ss;
        ss << "session_" << time(0) << "_" << rand();
        return ss.str();
    }

public:
    static void initializeUsers() {
        users["admin"] = User("admin", hashPassword("admin123"), "admin");
        users["user1"] = User("user1", hashPassword("user123"), "user");
        users["user2"] = User("user2", hashPassword("pass123"), "user");
        users["testuser"] = User("testuser", hashPassword("test123"), "user");
        users["john"] = User("john", hashPassword("john123"), "user");
    }
    
    static std::string login() {
        return "LOGIN_PAGE: Please enter username and password";
    }
    
    static std::string doLogin(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "ERROR: Username and password required";
        }
        
        auto it = users.find(username);
        if (it == users.end()) {
            return "ERROR: Invalid credentials";
        }
        
        std::string hashedPassword = hashPassword(password);
        if (it->second.passwordHash != hashedPassword) {
            return "ERROR: Invalid credentials";
        }
        
        std::string sessionId = generateSessionId();
        sessions[sessionId] = username;
        
        return "SUCCESS: Logged in as " + username + " | Session: " + sessionId;
    }
    
    static std::string index(const std::string& sessionId = "") {
        if (sessionId.empty() || sessions.find(sessionId) == sessions.end()) {
            return "INDEX_PAGE: Welcome! Please login to continue.";
        }
        
        std::string username = sessions[sessionId];
        return "INDEX_PAGE: Welcome " + username + "! You are logged in.";
    }
    
    static std::string userPage(const std::string& sessionId = "") {
        if (sessionId.empty() || sessions.find(sessionId) == sessions.end()) {
            return "ERROR: Unauthorized. Please login first.";
        }
        
        std::string username = sessions[sessionId];
        auto it = users.find(username);
        
        if (it == users.end()) {
            return "ERROR: User not found";
        }
        
        return "USER_PAGE: Hello " + username + "! This is your user dashboard.";
    }
    
    static std::string adminPage(const std::string& sessionId = "") {
        if (sessionId.empty() || sessions.find(sessionId) == sessions.end()) {
            return "ERROR: Unauthorized. Please login first.";
        }
        
        std::string username = sessions[sessionId];
        auto it = users.find(username);
        
        if (it == users.end() || it->second.role != "admin") {
            return "ERROR: Forbidden. Admin access required.";
        }
        
        return "ADMIN_PAGE: Welcome Admin " + username + "! You have full access.";
    }
};

std::map<std::string, User> Lse55::users;
std::map<std::string, std::string> Lse55::sessions;

int main() {
    srand(time(0));
    Lse55::initializeUsers();
    
    std::cout << "=== Test Case 1: Access login page ===" << std::endl;
    std::cout << Lse55::login() << std::endl;
    
    std::cout << "\\n=== Test Case 2: Successful user login ===" << std::endl;
    std::string session1 = Lse55::doLogin("user1", "user123");
    std::cout << session1 << std::endl;
    std::string sessionId1 = session1.substr(session1.find("Session: ") + 9);
    
    std::cout << "\\n=== Test Case 3: Failed login with wrong password ===" << std::endl;
    std::cout << Lse55::doLogin("user1", "wrongpass") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Access index and user page ===" << std::endl;
    std::cout << Lse55::index(sessionId1) << std::endl;
    std::cout << Lse55::userPage(sessionId1) << std::endl;
    
    std::cout << "\\n=== Test Case 5: Admin login and access admin page ===" << std::endl;
    std::string session2 = Lse55::doLogin("admin", "admin123");
    std::cout << session2 << std::endl;
    std::string sessionId2 = session2.substr(session2.find("Session: ") + 9);
    std::cout << Lse55::adminPage(sessionId2) << std::endl;
    std::cout << "User trying to access admin page:" << std::endl;
    std::cout << Lse55::adminPage(sessionId1) << std::endl;
    
    return 0;
}
