
#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <mutex>
#include <stdexcept>
#include <regex>

class Task184 {
private:
    static std::unordered_set<std::string> usedNames;
    static std::mutex mtx;
    static const int MAX_NAMES = 26 * 26 * 1000;
    static std::random_device rd;
    static std::mt19937 gen;
    
    std::string name;
    
    static std::string generateRandomName() {
        std::uniform_int_distribution<> letterDist(0, 25);
        std::uniform_int_distribution<> digitDist(0, 999);
        
        char letter1 = 'A' + letterDist(gen);
        char letter2 = 'A' + letterDist(gen);
        int digits = digitDist(gen);
        
        char buffer[7];
        snprintf(buffer, sizeof(buffer), "%c%c%03d", letter1, letter2, digits);
        return std::string(buffer);
    }
    
    static std::string generateUniqueName() {
        std::lock_guard<std::mutex> lock(mtx);
        
        if (usedNames.size() >= MAX_NAMES) {
            throw std::runtime_error("All possible robot names have been exhausted");
        }
        
        std::string newName;
        do {
            newName = generateRandomName();
        } while (usedNames.find(newName) != usedNames.end());
        
        usedNames.insert(newName);
        return newName;
    }
    
public:
    Task184() : name(generateUniqueName()) {}
    
    std::string getName() const {
        return name;
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!name.empty()) {
            usedNames.erase(name);
        }
        name = generateUniqueName();
    }
};

std::unordered_set<std::string> Task184::usedNames;
std::mutex Task184::mtx;
std::random_device Task184::rd;
std::mt19937 Task184::gen(Task184::rd());

int main() {
    std::cout << "Test Case 1: Create robot and get name\\n";
    Task184 robot1;
    std::string name1 = robot1.getName();
    std::cout << "Robot 1 name: " << name1 << "\\n";
    std::regex pattern("[A-Z]{2}\\\\d{3}");
    std::cout << "Name matches pattern: " << (std::regex_match(name1, pattern) ? "true" : "false") << "\\n";
    
    std::cout << "\\nTest Case 2: Create another robot with different name\\n";
    Task184 robot2;
    std::string name2 = robot2.getName();
    std::cout << "Robot 2 name: " << name2 << "\\n";
    std::cout << "Names are different: " << (name1 != name2 ? "true" : "false") << "\\n";
    
    std::cout << "\\nTest Case 3: Reset robot and verify new name\\n";
    std::string oldName = robot1.getName();
    robot1.reset();
    std::string newName = robot1.getName();
    std::cout << "Old name: " << oldName << "\\n";
    std::cout << "New name: " << newName << "\\n";
    std::cout << "Names are different: " << (oldName != newName ? "true" : "false") << "\\n";
    
    std::cout << "\\nTest Case 4: Create multiple robots\\n";
    Task184 robot3;
    Task184 robot4;
    Task184 robot5;
    std::cout << "Robot 3: " << robot3.getName() << "\\n";
    std::cout << "Robot 4: " << robot4.getName() << "\\n";
    std::cout << "Robot 5: " << robot5.getName() << "\\n";
    
    std::cout << "\\nTest Case 5: Verify uniqueness after resets\\n";
    robot2.reset();
    robot3.reset();
    std::unordered_set<std::string> allNames;
    allNames.insert(robot1.getName());
    allNames.insert(robot2.getName());
    allNames.insert(robot3.getName());
    allNames.insert(robot4.getName());
    allNames.insert(robot5.getName());
    std::cout << "All 5 names are unique: " << (allNames.size() == 5 ? "true" : "false") << "\\n";
    
    return 0;
}
