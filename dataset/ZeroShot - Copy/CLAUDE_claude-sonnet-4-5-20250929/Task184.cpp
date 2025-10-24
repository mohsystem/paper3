
#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <mutex>
#include <stdexcept>
#include <vector>

class Task184 {
private:
    static std::unordered_set<std::string> usedNames;
    static std::mutex mtx;
    static std::mt19937 rng;
    static const int MAX_NAMES = 26 * 26 * 1000;
    
    std::string name;
    
    static std::string generateRandomName() {
        std::uniform_int_distribution<int> letterDist(0, 25);
        std::uniform_int_distribution<int> digitDist(0, 999);
        
        char letter1 = 'A' + letterDist(rng);
        char letter2 = 'A' + letterDist(rng);
        int number = digitDist(rng);
        
        char buffer[6];
        snprintf(buffer, sizeof(buffer), "%c%c%03d", letter1, letter2, number);
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
        usedNames.erase(name);
        name = generateUniqueName();
    }
};

std::unordered_set<std::string> Task184::usedNames;
std::mutex Task184::mtx;
std::mt19937 Task184::rng(std::random_device{}());

int main() {
    std::cout << "Test Case 1: Creating new robots" << std::endl;
    Task184 robot1;
    Task184 robot2;
    std::cout << "Robot 1: " << robot1.getName() << std::endl;
    std::cout << "Robot 2: " << robot2.getName() << std::endl;
    std::cout << "Names are unique: " << (robot1.getName() != robot2.getName() ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 2: Resetting a robot" << std::endl;
    std::string oldName = robot1.getName();
    robot1.reset();
    std::string newName = robot1.getName();
    std::cout << "Old name: " << oldName << std::endl;
    std::cout << "New name: " << newName << std::endl;
    std::cout << "Names are different: " << (oldName != newName ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 3: Creating multiple robots" << std::endl;
    std::vector<Task184> robots;
    for (int i = 0; i < 5; i++) {
        robots.emplace_back();
    }
    std::unordered_set<std::string> names;
    for (const auto& robot : robots) {
        std::string robotName = robot.getName();
        names.insert(robotName);
        std::cout << "Robot: " << robotName << std::endl;
    }
    std::cout << "All names unique: " << (names.size() == robots.size() ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 4: Resetting multiple robots" << std::endl;
    for (auto& robot : robots) {
        std::string before = robot.getName();
        robot.reset();
        std::cout << before << " -> " << robot.getName() << std::endl;
    }
    
    std::cout << "\\nTest Case 5: Verify name format" << std::endl;
    Task184 robot3;
    std::string testName = robot3.getName();
    bool validFormat = testName.length() == 5 && 
                       isupper(testName[0]) && isupper(testName[1]) &&
                       isdigit(testName[2]) && isdigit(testName[3]) && isdigit(testName[4]);
    std::cout << "Robot name: " << testName << std::endl;
    std::cout << "Valid format (2 letters + 3 digits): " << (validFormat ? "true" : "false") << std::endl;
    
    return 0;
}
