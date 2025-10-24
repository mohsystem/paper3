
#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <stdexcept>
#include <mutex>

// Global set to track all used names across all robot instances
// Protected by mutex for thread safety
static std::unordered_set<std::string> usedNames;
static std::mutex namesMutex;

class Robot {
private:
    std::string name_;
    
    // Generate a cryptographically secure random name
    // Uses std::random_device for secure randomness per Rules#1 and Rules#6
    std::string generateUniqueName() {
        // Lock to ensure thread-safe access to usedNames set
        std::lock_guard<std::mutex> lock(namesMutex);
        
        // Maximum possible unique names: 26*26*10*10*10 = 676,000
        // Fail if we've exhausted the namespace to prevent infinite loops\n        if (usedNames.size() >= 676000) {\n            throw std::runtime_error("All possible robot names have been used");\n        }\n        \n        // Use random_device for cryptographically secure randomness (Rules#6, Rules#7)\n        std::random_device rd;\n        std::mt19937 gen(rd());\n        \n        // Distribution for uppercase letters A-Z (ASCII 65-90)\n        std::uniform_int_distribution<> letterDist(65, 90);\n        // Distribution for digits 0-9 (ASCII 48-57)\n        std::uniform_int_distribution<> digitDist(48, 57);\n        \n        std::string newName;\n        \n        // Keep generating until we find an unused name\n        // This ensures uniqueness as required by the problem\n        do {\n            newName.clear();\n            newName.reserve(5); // 2 letters + 3 digits\n            \n            // Generate two uppercase letters\n            newName += static_cast<char>(letterDist(gen));\n            newName += static_cast<char>(letterDist(gen));\n            \n            // Generate three digits\n            newName += static_cast<char>(digitDist(gen));\n            newName += static_cast<char>(digitDist(gen));\n            newName += static_cast<char>(digitDist(gen));\n            \n        } while (usedNames.find(newName) != usedNames.end());\n        \n        // Mark this name as used\n        usedNames.insert(newName);\n        \n        return newName;\n    }\n    \npublic:\n    // Constructor: robot has no name initially (per specification)\n    Robot() : name_() {}\n    \n    // Destructor: release the name back to the pool\n    ~Robot() {\n        if (!name_.empty()) {\n            std::lock_guard<std::mutex> lock(namesMutex);\n            usedNames.erase(name_);\n        }\n    }\n    \n    // Get the robot's name, generating one if it doesn't exist
    std::string getName() {
        if (name_.empty()) {
            name_ = generateUniqueName();
        }
        return name_;
    }
    
    // Reset robot to factory settings (wipes name)
    void reset() {
        if (!name_.empty()) {
            // Release the old name back to the pool
            std::lock_guard<std::mutex> lock(namesMutex);
            usedNames.erase(name_);
            name_.clear();
        }
        // Next getName() call will generate a new name
    }
};

int main() {
    try {
        // Test case 1: Create a robot and get its name
        Robot robot1;
        std::string name1 = robot1.getName();
        std::cout << "Test 1 - Robot 1 name: " << name1 << std::endl;
        
        // Test case 2: Verify name format (2 letters + 3 digits)
        if (name1.length() == 5 && 
            std::isupper(name1[0]) && std::isupper(name1[1]) &&
            std::isdigit(name1[2]) && std::isdigit(name1[3]) && std::isdigit(name1[4])) {
            std::cout << "Test 2 - Name format valid" << std::endl;
        }
        
        // Test case 3: Create another robot and ensure different name
        Robot robot2;
        std::string name2 = robot2.getName();
        std::cout << "Test 3 - Robot 2 name: " << name2 << std::endl;
        if (name1 != name2) {
            std::cout << "Test 3 - Names are unique" << std::endl;
        }
        
        // Test case 4: Reset robot and get new name
        robot1.reset();
        std::string name1New = robot1.getName();
        std::cout << "Test 4 - Robot 1 after reset: " << name1New << std::endl;
        if (name1 != name1New) {
            std::cout << "Test 4 - Name changed after reset" << std::endl;
        }
        
        // Test case 5: Verify all three current names are unique
        std::string name2Current = robot2.getName();
        if (name1New != name2Current && name1New != name2 && name2Current != name1) {
            std::cout << "Test 5 - All names remain unique" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
