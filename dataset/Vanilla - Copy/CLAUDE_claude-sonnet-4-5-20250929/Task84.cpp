
#include <iostream>
#include <string>
#include <set>
#include <random>
#include <ctime>

class Task84 {
private:
    static const std::string CHARACTERS;
    static const int SESSION_ID_LENGTH = 32;
    static std::set<std::string> usedSessionIds;
    static std::mt19937 rng;
    
public:
    static std::string generateSessionId() {
        std::string sessionId;
        do {
            sessionId = "";
            for (int i = 0; i < SESSION_ID_LENGTH; i++) {
                int index = rng() % CHARACTERS.length();
                sessionId += CHARACTERS[index];
            }
        } while (usedSessionIds.find(sessionId) != usedSessionIds.end());
        
        usedSessionIds.insert(sessionId);
        return sessionId;
    }
};

const std::string Task84::CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
std::set<std::string> Task84::usedSessionIds;
std::mt19937 Task84::rng(time(0));

int main() {
    std::cout << "Test Case 1: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 2: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 3: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 4: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 5: " << Task84::generateSessionId() << std::endl;
    
    return 0;
}
