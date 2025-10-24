#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <unordered_set>
#include <chrono>

class Robot {
private:
    std::string robotName;
    static std::unordered_set<std::string> usedNames;

    // A better random number generator
    static std::mt19937& getRandomEngine() {
        static std::mt19937 engine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        return engine;
    }

    static std::string generateRandomName() {
        std::uniform_int_distribution<int> letter_dist('A', 'Z');
        std::uniform_int_distribution<int> digit_dist('0', '9');
        
        std::string name = "     "; // 5 chars
        name[0] = static_cast<char>(letter_dist(getRandomEngine()));
        name[1] = static_cast<char>(letter_dist(getRandomEngine()));
        name[2] = static_cast<char>(digit_dist(getRandomEngine()));
        name[3] = static_cast<char>(digit_dist(getRandomEngine()));
        name[4] = static_cast<char>(digit_dist(getRandomEngine()));
        
        return name;
    }

    void generateUniqueName() {
        std::string newName;
        do {
            newName = generateRandomName();
        } while (usedNames.count(newName));
        usedNames.insert(newName);
        this->robotName = newName;
    }

public:
    Robot() : robotName("") {}

    const std::string& getName() {
        if (robotName.empty()) {
            generateUniqueName();
        }
        return robotName;
    }

    void reset() {
        if (!robotName.empty()) {
            usedNames.erase(robotName);
            robotName = "";
        }
    }
};

// Initialize static member
std::unordered_set<std::string> Robot::usedNames;

class Task184 {
public:
    static void main() {
        // Test Case 1: Create a robot and get its name
        std::cout << "--- Test Case 1 ---" << std::endl;
        Robot r1;
        std::cout << "Robot 1 initial name: " << r1.getName() << std::endl;
        std::cout << "Robot 1 name again: " << r1.getName() << std::endl;

        // Test Case 2: Create another robot
        std::cout << "\n--- Test Case 2 ---" << std::endl;
        Robot r2;
        std::cout << "Robot 2 initial name: " << r2.getName() << std::endl;

        // Test Case 3: Reset the first robot and get a new name
        std::cout << "\n--- Test Case 3 ---" << std::endl;
        std::cout << "Resetting Robot 1..." << std::endl;
        r1.reset();
        std::cout << "Robot 1 new name: " << r1.getName() << std::endl;

        // Test Case 4: Create a few more robots
        std::cout << "\n--- Test Case 4 ---" << std::endl;
        Robot r3, r4;
        std::cout << "Robot 3 name: " << r3.getName() << std::endl;
        std::cout << "Robot 4 name: " << r4.getName() << std::endl;

        // Test Case 5: Reset r2 and r3
        std::cout << "\n--- Test Case 5 ---" << std::endl;
        std::cout << "Resetting Robot 2 and 3..." << std::endl;
        r2.reset();
        r3.reset();
        std::cout << "Robot 2 new name: " << r2.getName() << std::endl;
        std::cout << "Robot 3 new name: " << r3.getName() << std::endl;
        std::cout << "Robot 1 current name: " << r1.getName() << std::endl;
        std::cout << "Robot 4 current name: " << r4.getName() << std::endl;
    }
};

int main() {
    Task184::main();
    return 0;
}