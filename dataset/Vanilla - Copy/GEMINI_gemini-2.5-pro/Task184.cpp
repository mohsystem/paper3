#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <sstream>
#include <iomanip>

class Task184 {
private:
    std::string name;
    static std::unordered_set<std::string> used_names;
    
    static std::string generate_random_name() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> letter_dist('A', 'Z');
        std::uniform_int_distribution<> digit_dist(0, 999);

        std::stringstream ss;
        ss << (char)letter_dist(gen)
           << (char)letter_dist(gen)
           << std::setw(3) << std::setfill('0') << digit_dist(gen);
        return ss.str();
    }

public:
    Task184() {
        reset();
    }

    std::string get_name() const {
        return name;
    }

    void reset() {
        if (!name.empty()) {
            used_names.erase(name);
        }

        std::string new_name;
        do {
            new_name = generate_random_name();
        } while (used_names.count(new_name));
        
        used_names.insert(new_name);
        name = new_name;
    }
};

std::unordered_set<std::string> Task184::used_names;

int main() {
    std::cout << "--- Test Case 1: Create 5 robots ---" << std::endl;
    Task184 r1;
    Task184 r2;
    Task184 r3;
    Task184 r4;
    Task184 r5;

    std::cout << "Robot 1 name: " << r1.get_name() << std::endl;
    std::cout << "Robot 2 name: " << r2.get_name() << std::endl;
    std::cout << "Robot 3 name: " << r3.get_name() << std::endl;
    std::cout << "Robot 4 name: " << r4.get_name() << std::endl;
    std::cout << "Robot 5 name: " << r5.get_name() << std::endl;

    std::cout << "\n--- Test Case 2: Reset Robot 3 ---" << std::endl;
    std::string old_name_r3 = r3.get_name();
    r3.reset();
    std::cout << "Robot 3 old name: " << old_name_r3 << std::endl;
    std::cout << "Robot 3 new name: " << r3.get_name() << std::endl;
    
    std::cout << "\n--- Test Case 3: Reset Robot 1 ---" << std::endl;
    std::string old_name_r1 = r1.get_name();
    r1.reset();
    std::cout << "Robot 1 old name: " << old_name_r1 << std::endl;
    std::cout << "Robot 1 new name: " << r1.get_name() << std::endl;

    std::cout << "\n--- Test Case 4: Show current names ---" << std::endl;
    std::cout << "Robot 1 name: " << r1.get_name() << std::endl;
    std::cout << "Robot 2 name: " << r2.get_name() << std::endl;
    std::cout << "Robot 3 name: " << r3.get_name() << std::endl;
    std::cout << "Robot 4 name: " << r4.get_name() << std::endl;
    std::cout << "Robot 5 name: " << r5.get_name() << std::endl;
    
    std::cout << "\n--- Test Case 5: Create a new robot ---" << std::endl;
    Task184 r6;
    std::cout << "New Robot 6 name: " << r6.get_name() << std::endl;

    return 0;
}