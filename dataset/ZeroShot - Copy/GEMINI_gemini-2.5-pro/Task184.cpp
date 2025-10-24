#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>
#include <mutex>
#include <memory>

class RobotNameManager {
public:
    static std::string generateUniqueName() {
        std::lock_guard<std::mutex> lock(mtx);

        if (namesInUse.size() >= MAX_NAMES) {
            // Or throw an exception
            return ""; 
        }

        std::string name;
        do {
            name = generateRandomName();
        } while (namesInUse.count(name)); // Loop until a unique name is found

        namesInUse.insert(name);
        return name;
    }

    static void releaseName(const std::string& name) {
        if (!name.empty()) {
            std::lock_guard<std::mutex> lock(mtx);
            namesInUse.erase(name);
        }
    }

private:
    static std::string generateRandomName() {
        // C++11 random number generation provides higher quality randomness
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> letter_dist('A', 'Z');
        static std::uniform_int_distribution<> digit_dist('0', '9');

        char name_chars[6];
        name_chars[0] = static_cast<char>(letter_dist(gen));
        name_chars[1] = static_cast<char>(letter_dist(gen));
        name_chars[2] = static_cast<char>(digit_dist(gen));
        name_chars[3] = static_cast<char>(digit_dist(gen));
        name_chars[4] = static_cast<char>(digit_dist(gen));
        name_chars[5] = '\0';
        return std::string(name_chars);
    }
    
    // Static members are shared across all instances
    static std::unordered_set<std::string> namesInUse;
    static std::mutex mtx;
    static const int MAX_NAMES = 26 * 26 * 1000;
};

// Initialize static members
std::unordered_set<std::string> RobotNameManager::namesInUse;
std::mutex RobotNameManager::mtx;

class Robot {
private:
    std::string robot_name;

public:
    Robot() : robot_name("") {} // No name initially

    // Destructor to release the name when the robot is destroyed
    ~Robot() {
        RobotNameManager::releaseName(robot_name);
    }
    
    // Disable copy constructor and assignment to prevent accidental name duplication
    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;
    
    // Allow move constructor and assignment for efficient resource transfer
    Robot(Robot&& other) noexcept : robot_name(std::move(other.robot_name)) {
        other.robot_name = ""; // Ensure the moved-from robot has no name
    }
    
    Robot& operator=(Robot&& other) noexcept {
        if (this != &other) {
            RobotNameManager::releaseName(robot_name); // Release current name
            robot_name = std::move(other.robot_name);
            other.robot_name = "";
        }
        return *this;
    }

    const std::string& getName() {
        if (robot_name.empty()) {
            robot_name = RobotNameManager::generateUniqueName();
        }
        return robot_name;
    }

    void reset() {
        RobotNameManager::releaseName(robot_name);
        robot_name = "";
    }
};

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Create a robot and get its name
    Robot robot1;
    std::cout << "Robot 1 initial name: " << robot1.getName() << std::endl;

    // Test Case 2: Create another robot and get its name
    Robot robot2;
    std::cout << "Robot 2 initial name: " << robot2.getName() << std::endl;

    // Test Case 3: Reset the first robot and get a new name
    std::cout << "Resetting Robot 1..." << std::endl;
    std::string oldName1 = robot1.getName();
    robot1.reset();
    std::cout << "Robot 1 old name was: " << oldName1 << std::endl;
    std::cout << "Robot 1 new name is: " << robot1.getName() << std::endl;

    // Test Case 4: Create more robots
    Robot robot3;
    Robot robot4;
    std::cout << "Robot 3 name: " << robot3.getName() << std::endl;
    std::cout << "Robot 4 name: " << robot4.getName() << std::endl;

    // Test Case 5: Get the name again, should be the same
    std::cout << "Robot 4 name (again): " << robot4.getName() << std::endl;
    
    std::cout << "--------------------" << std::endl << std::endl;
    
    return 0;
}