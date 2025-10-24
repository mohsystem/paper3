#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>
#include <mutex>
#include <iomanip>
#include <sstream>

class Robot {
public:
    Robot() : name_("") {}

    // Rule of 5 to manage resources properly if needed, though simple here.
    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;
    Robot(Robot&&) = delete;
    Robot& operator=(Robot&&) = delete;
    ~Robot() = default;

    std::string getName() {
        if (name_.empty()) {
            name_ = generateUniqueName();
        }
        return name_;
    }

    void reset() {
        if (!name_.empty()) {
            std::lock_guard<std::mutex> lock(name_mutex_);
            used_names_.erase(name_);
            name_.clear();
        }
    }

private:
    std::string name_;

    static std::unordered_set<std::string> used_names_;
    static std::mutex name_mutex_;

    static std::string generateUniqueName() {
        // Seed with a real random value, if available
        static std::random_device rd;
        // The Mersenne Twister is a good general-purpose generator
        static std::mt19937 gen(rd());
        
        std::uniform_int_distribution<> letter_dist('A', 'Z');
        std::uniform_int_distribution<> number_dist(0, 999);

        while (true) {
            std::stringstream ss;
            ss << (char)letter_dist(gen)
               << (char)letter_dist(gen)
               << std::setw(3) << std::setfill('0') << number_dist(gen);
            std::string new_name = ss.str();

            std::lock_guard<std::mutex> lock(name_mutex_);
            if (used_names_.find(new_name) == used_names_.end()) {
                used_names_.insert(new_name);
                return new_name;
            }
        }
    }
};

std::unordered_set<std::string> Robot::used_names_;
std::mutex Robot::name_mutex_;

int main() {
    // Test Case 1: Create a robot and get its name
    Robot robot1;
    std::string name1 = robot1.getName();
    std::cout << "Test Case 1: Robot 1's first name: " << name1 << std::endl;

    // Test Case 2: Get the same robot's name again, should be the same
    std::string name1_again = robot1.getName();
    std::cout << "Test Case 2: Robot 1's name again: " << name1_again << std::endl;
    std::cout << "Name is consistent: " << (name1 == name1_again ? "true" : "false") << std::endl;

    // Test Case 3: Create a second robot, its name should be different
    Robot robot2;
    std::string name2 = robot2.getName();
    std::cout << "Test Case 3: Robot 2's name: " << name2 << std::endl;
    std::cout << "Names are unique: " << (name1 != name2 ? "true" : "false") << std::endl;

    // Test Case 4: Reset the first robot and get its new name
    std::cout << "Test Case 4: Resetting Robot 1..." << std::endl;
    robot1.reset();
    std::string newName1 = robot1.getName();
    std::cout << "Robot 1's new name: " << newName1 << std::endl;
    std::cout << "New name is different from old name: " << (name1 != newName1 ? "true" : "false") << std::endl;
    std::cout << "New name is unique from Robot 2: " << (newName1 != name2 ? "true" : "false") << std::endl;

    // Test Case 5: Create more robots
    std::cout << "Test Case 5: Creating more robots..." << std::endl;
    Robot robot3;
    Robot robot4;
    std::cout << "Robot 3's name: " << robot3.getName() << std::endl;
    std::cout << "Robot 4's name: " << robot4.getName() << std::endl;

    return 0;
}