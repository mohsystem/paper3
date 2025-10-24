#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <iomanip>
#include <sstream>

class RobotFactory {
public:
    static std::string generateUnique() {
        if (allocated.size() >= CAPACITY) {
            throw std::runtime_error("All possible robot names have been allocated.");
        }
        std::uniform_int_distribution<int> letter(0, 25);
        std::uniform_int_distribution<int> number(0, 999);

        std::string name;
        do {
            char l1 = static_cast<char>('A' + letter(gen));
            char l2 = static_cast<char>('A' + letter(gen));
            int num = number(gen);
            std::ostringstream oss;
            oss << l1 << l2 << std::setw(3) << std::setfill('0') << num;
            name = oss.str();
        } while (allocated.find(name) != allocated.end());
        allocated.insert(name);
        return name;
    }

    static void release(const std::string& name) {
        allocated.erase(name);
    }

private:
    static inline std::unordered_set<std::string> allocated{};
    static inline std::mt19937 gen{std::random_device{}()};
    static constexpr size_t CAPACITY = 26 * 26 * 1000;
};

class Robot {
public:
    std::string getName() {
        if (name.empty()) {
            name = RobotFactory::generateUnique();
        }
        return name;
    }

    void reset() {
        if (!name.empty()) {
            RobotFactory::release(name);
            name.clear();
        }
    }

private:
    std::string name;
};

int main() {
    Robot r1, r2, r3;
    std::cout << "R1: " << r1.getName() << "\n";
    std::cout << "R2: " << r2.getName() << "\n";
    std::cout << "R3: " << r3.getName() << "\n";
    Robot r4;
    std::cout << "R4: " << r4.getName() << "\n";
    r2.reset();
    std::cout << "R2 after reset: " << r2.getName() << "\n";
    return 0;
}