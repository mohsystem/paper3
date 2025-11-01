
#include <iostream>
#include <string>
#include <map>
#include <set>

class Lse137 {
public:
    // Function to check if user is a moderator
    static bool authMod(const std::string& username, const std::string& password, const std::string& session) {
        // Sample moderator credentials (in real application, this would be from a database)
        std::map<std::string, std::string> moderators;
        moderators["admin"] = "admin123";
        moderators["mod1"] = "modpass1";
        moderators["mod2"] = "modpass2";
        
        // Sample valid sessions (in real application, this would be validated from session store)
        std::set<std::string> validSessions = {"session123", "session456", "session789"};
        
        // Check if username exists, password matches, and session is valid
        auto it = moderators.find(username);
        if (it != moderators.end() && 
            it->second == password && 
            validSessions.find(session) != validSessions.end()) {
            return true;
        }
        return false;
    }
    
    // Function that would be called when visiting /check_mod URL
    static bool checkMod(const std::string& username, const std::string& password, const std::string& session) {
        return authMod(username, password, session);
    }
};

int main() {
    // Test case 1: Valid moderator credentials and session
    std::cout << "Test 1: " << Lse137::checkMod("admin", "admin123", "session123") << std::endl;
    
    // Test case 2: Valid moderator but invalid session
    std::cout << "Test 2: " << Lse137::checkMod("mod1", "modpass1", "invalidsession") << std::endl;
    
    // Test case 3: Invalid username
    std::cout << "Test 3: " << Lse137::checkMod("user1", "password", "session456") << std::endl;
    
    // Test case 4: Valid username but wrong password
    std::cout << "Test 4: " << Lse137::checkMod("mod2", "wrongpass", "session789") << std::endl;
    
    // Test case 5: Another valid moderator with valid session
    std::cout << "Test 5: " << Lse137::checkMod("mod2", "modpass2", "session456") << std::endl;
    
    return 0;
}
