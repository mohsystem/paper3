
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class User {
public:
    std::string username;
    std::string passwordHash;
    bool isModerator;
    
    User() : isModerator(false) {}
    User(std::string u, std::string ph, bool im) : username(u), passwordHash(ph), isModerator(im) {}
};

class Session {
public:
    std::string sessionId;
    std::string username;
    time_t expiryTime;
    
    Session() : expiryTime(0) {}
    Session(std::string sid, std::string u, time_t et) : sessionId(sid), username(u), expiryTime(et) {}
    
    bool isValid() {
        return time(nullptr) < expiryTime;
    }
};

std::map<std::string, User> users;
std::map<std::string, Session> sessions;

std::string hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool auth_mod(const std::string& username, const std::string& password, const std::string& sessionId) {
    if (username.empty() || password.empty() || sessionId.empty()) {
        return false;
    }
    
    if (sessions.find(sessionId) == sessions.end()) {
        return false;
    }
    
    Session& session = sessions[sessionId];
    if (!session.isValid() || session.username != username) {
        return false;
    }
    
    if (users.find(username) == users.end()) {
        return false;
    }
    
    User& user = users[username];
    std::string hashedPassword = hashPassword(password);
    
    if (user.passwordHash != hashedPassword) {
        return false;
    }
    
    return user.isModerator;
}

bool check_mod(const std::string& username, const std::string& password, const std::string& sessionId) {
    return auth_mod(username, password, sessionId);
}

int main() {
    // Setup test data
    users["admin"] = User("admin", hashPassword("admin123"), true);
    users["user1"] = User("user1", hashPassword("pass123"), false);
    users["mod1"] = User("mod1", hashPassword("modpass"), true);
    
    std::string validSession = "session123";
    sessions[validSession] = Session(validSession, "admin", time(nullptr) + 3600);
    
    std::string validSession2 = "session456";
    sessions[validSession2] = Session(validSession2, "user1", time(nullptr) + 3600);
    
    std::string validSession3 = "session789";
    sessions[validSession3] = Session(validSession3, "mod1", time(nullptr) + 3600);
    
    // Test cases
    std::cout << "Test 1 (Valid moderator): " << (check_mod("admin", "admin123", validSession) ? "true" : "false") << std::endl;
    std::cout << "Test 2 (Valid non-moderator): " << (check_mod("user1", "pass123", validSession2) ? "true" : "false") << std::endl;
    std::cout << "Test 3 (Invalid password): " << (check_mod("admin", "wrongpass", validSession) ? "true" : "false") << std::endl;
    std::cout << "Test 4 (Invalid session): " << (check_mod("admin", "admin123", "invalidsession") ? "true" : "false") << std::endl;
    std::cout << "Test 5 (Another valid moderator): " << (check_mod("mod1", "modpass", validSession3) ? "true" : "false") << std::endl;
    
    return 0;
}
