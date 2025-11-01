
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Lse55 {
private:
    struct User {
        std::string username;
        std::string passwordHash;
        std::string role;
        
        User() {}
        User(std::string u, std::string p, std::string r) 
            : username(u), passwordHash(p), role(r) {}
    };
    
    struct Session {
        std::string username;
        std::string role;
        time_t expiryTime;
        
        Session() {}
        Session(std::string u, std::string r) 
            : username(u), role(r) {
            expiryTime = time(nullptr) + 3600;
        }
        
        bool isValid() {
            return time(nullptr) < expiryTime;
        }
    };
    
    static std::map<std::string, User> users;
    static std::map<std::string, Session> sessions;
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    static std::string generateSessionToken() {
        std::stringstream ss;
        ss << time(nullptr) << rand();
        return hashPassword(ss.str());
    }
    
public:
    static std::string login() {
        return "LOGIN_PAGE: Please provide username and password";
    }
    
    static std::map<std::string, std::string> doLogin(const std::string& username, const std::string& password) {
        std::map<std::string, std::string> response;
        
        if (username.empty() || password.empty()) {
            response["status"] = "error";
            response["message"] = "Invalid credentials";
            return response;
        }
        
        auto it = users.find(username);
        if (it == users.end() || it->second.passwordHash != hashPassword(password)) {
            response["status"] = "error";
            response["message"] = "Invalid credentials";
            return response;
        }
        
        std::string sessionToken = generateSessionToken();
        sessions[sessionToken] = Session(it->second.username, it->second.role);
        
        response["status"] = "success";
        response["sessionToken"] = sessionToken;
        response["message"] = "Login successful";
        return response;
    }
    
    static std::string index() {
        return "INDEX_PAGE: Welcome to the home page";
    }
    
    static std::string userPage(const std::string& sessionToken) {
        if (sessionToken.empty()) {
            return "ERROR: Authentication required";
        }
        
        auto it = sessions.find(sessionToken);
        if (it == sessions.end() || !it->second.isValid()) {
            return "ERROR: Invalid or expired session";
        }
        
        return "USER_PAGE: Welcome " + it->second.username + "!";
    }
    
    static std::string adminPage(const std::string& sessionToken) {
        if (sessionToken.empty()) {
            return "ERROR: Authentication required";
        }
        
        auto it = sessions.find(sessionToken);
        if (it == sessions.end() || !it->second.isValid()) {
            return "ERROR: Invalid or expired session";
        }
        
        if (it->second.role != "admin") {
            return "ERROR: Unauthorized - Admin access required";
        }
        
        return "ADMIN_PAGE: Welcome Admin " + it->second.username + "!";
    }
    
    static void initTestUsers() {
        users["user1"] = User("user1", hashPassword("password123"), "user");
        users["admin1"] = User("admin1", hashPassword("admin123"), "admin");
    }
};

std::map<std::string, Lse55::User> Lse55::users;
std::map<std::string, Lse55::Session> Lse55::sessions;

int main() {
    srand(time(nullptr));
    Lse55::initTestUsers();
    
    std::cout << "=== Test Case 1: Login page ===" << std::endl;
    std::cout << Lse55::login() << std::endl << std::endl;
    
    std::cout << "=== Test Case 2: Failed login ===" << std::endl;
    auto result1 = Lse55::doLogin("user1", "wrongpassword");
    std::cout << "Status: " << result1["status"] << ", Message: " << result1["message"] << std::endl << std::endl;
    
    std::cout << "=== Test Case 3: Successful user login ===" << std::endl;
    auto result2 = Lse55::doLogin("user1", "password123");
    std::cout << "Status: " << result2["status"] << ", Message: " << result2["message"] << std::endl;
    std::string userToken = result2["sessionToken"];
    std::cout << std::endl;
    
    std::cout << "=== Test Case 4: User accessing user page ===" << std::endl;
    std::cout << Lse55::userPage(userToken) << std::endl << std::endl;
    
    std::cout << "=== Test Case 5: User accessing admin page (should fail) ===" << std::endl;
    std::cout << Lse55::adminPage(userToken) << std::endl << std::endl;
    
    std::cout << "=== Test Case 6: Admin login and access ===" << std::endl;
    auto result3 = Lse55::doLogin("admin1", "admin123");
    std::string adminToken = result3["sessionToken"];
    std::cout << Lse55::adminPage(adminToken) << std::endl << std::endl;
    
    std::cout << "=== Test Case 7: Index page ===" << std::endl;
    std::cout << Lse55::index() << std::endl;
    
    return 0;
}
