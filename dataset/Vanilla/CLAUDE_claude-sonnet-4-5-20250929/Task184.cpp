
#include <iostream>
#include <string>
#include <unordered_set>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <regex>

class Task184 {
private:
    static std::unordered_set<std::string> usedNames;
    static bool seeded;
    std::string name;
    
    static std::string generateRandomName() {
        if (!seeded) {
            srand(time(NULL));
            seeded = true;
        }
        
        char letter1 = 'A' + (rand() % 26);
        char letter2 = 'A' + (rand() % 26);
        int number = rand() % 1000;
        
        std::stringstream ss;
        ss << letter1 << letter2 << std::setfill('0') << std::setw(3) << number;
        return ss.str();
    }
    
    static std::string generateUniqueName() {
        std::string newName;
        do {
            newName = generateRandomName();
        } while (usedNames.find(newName) != usedNames.end());
        usedNames.insert(newName);
        return newName;
    }
    
public:
    Task184() {
        name = generateUniqueName();
    }
    
    std::string getName() const {
        return name;
    }
    
    void reset() {
        if (!name.empty()) {
            usedNames.erase(name);
        }
        name = generateUniqueName();
    }
};

std::unordered_set<std::string> Task184::usedNames;
bool Task184::seeded = false;

int main() {
    std::cout << "Test Case 1: Create a robot and get its name" << std::endl;
    Task184 robot1;
    std::string name1 = robot1.getName();
    std::cout << "Robot 1 name: " << name1 << std::endl;
    std::regex pattern("[A-Z]{2}\\\\d{3}");
    std::cout << "Name matches pattern: " << (std::regex_match(name1, pattern) ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 2: Create another robot with different name" << std::endl;
    Task184 robot2;
    std::string name2 = robot2.getName();
    std::cout << "Robot 2 name: " << name2 << std::endl;
    std::cout << "Names are different: " << (name1 != name2 ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 3: Reset robot and verify new name" << std::endl;
    std::string oldName = robot1.getName();
    robot1.reset();
    std::string newName = robot1.getName();
    std::cout << "Old name: " << oldName << std::endl;
    std::cout << "New name: " << newName << std::endl;
    std::cout << "Names are different after reset: " << (oldName != newName ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 4: Create multiple robots and verify uniqueness" << std::endl;
    std::unordered_set<std::string> names;
    for (int i = 0; i < 10; i++) {
        Task184 robot;
        names.insert(robot.getName());
    }
    std::cout << "Created 10 robots, unique names count: " << names.size() << std::endl;
    std::cout << "All names are unique: " << (names.size() == 10 ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 5: Reset multiple times" << std::endl;
    Task184 robot3;
    std::unordered_set<std::string> resetNames;
    resetNames.insert(robot3.getName());
    for (int i = 0; i < 5; i++) {
        robot3.reset();
        resetNames.insert(robot3.getName());
    }
    std::cout << "Robot reset 5 times, unique names: " << resetNames.size() << std::endl;
    std::cout << "All reset names are unique: " << (resetNames.size() == 6 ? "true" : "false") << std::endl;
    
    return 0;
}
