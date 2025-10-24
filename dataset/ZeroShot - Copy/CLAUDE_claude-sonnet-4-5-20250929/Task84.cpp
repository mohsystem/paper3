
#include <iostream>
#include <string>
#include <random>
#include <set>
#include <ctime>

class Task84 {
private:
    static const std::string CHARACTERS;
    static const int SESSION_ID_LENGTH = 32;
    static std::set<std::string> generatedIds;
    static std::random_device rd;
    static std::mt19937 generator;
    
public:
    static std::string generateSessionId() {
        std::uniform_int_distribution<> distribution(0, CHARACTERS.length() - 1);
        std::string sessionId;
        
        do {
            sessionId.clear();
            for (int i = 0; i < SESSION_ID_LENGTH; i++) {
                sessionId += CHARACTERS[distribution(generator)];
            }
        } while (generatedIds.find(sessionId) != generatedIds.end());
        
        generatedIds.insert(sessionId);
        return sessionId;
    }
};

const std::string Task84::CHARACTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
std::set<std::string> Task84::generatedIds;
std::random_device Task84::rd;
std::mt19937 Task84::generator(Task84::rd());

int main() {
    std::cout << "Test Case 1: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 2: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 3: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 4: " << Task84::generateSessionId() << std::endl;
    std::cout << "Test Case 5: " << Task84::generateSessionId() << std::endl;
    
    // Verify uniqueness
    std::set<std::string> testSet;
    for (int i = 0; i < 5; i++) {
        testSet.insert(Task84::generateSessionId());
    }
    std::cout << "\\nAll generated IDs are unique: " << (testSet.size() == 5 ? "true" : "false") << std::endl;
    
    return 0;
}
