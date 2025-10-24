
#include <iostream>
#include <iomanip>
#include <sstream>

class Task199 {
private:
    int hours;
    int minutes;
    
    void normalize(int totalMinutes) {
        totalMinutes = totalMinutes % (24 * 60);
        if (totalMinutes < 0) {
            totalMinutes += 24 * 60;
        }
        this->hours = totalMinutes / 60;
        this->minutes = totalMinutes % 60;
    }
    
public:
    Task199(int hours, int minutes) {
        int totalMinutes = hours * 60 + minutes;
        normalize(totalMinutes);
    }
    
    void addMinutes(int minutesToAdd) {
        int totalMinutes = this->hours * 60 + this->minutes + minutesToAdd;
        normalize(totalMinutes);
    }
    
    void subtractMinutes(int minutesToSubtract) {
        addMinutes(-minutesToSubtract);
    }
    
    bool equals(const Task199& other) const {
        return this->hours == other.hours && this->minutes == other.minutes;
    }
    
    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hours << ":"
            << std::setfill('0') << std::setw(2) << minutes;
        return oss.str();
    }
};

int main() {
    // Test case 1: Basic clock creation
    Task199 clock1(10, 30);
    std::cout << "Test 1 - Clock creation: " << clock1.toString() << std::endl;
    
    // Test case 2: Add minutes
    Task199 clock2(10, 30);
    clock2.addMinutes(90);
    std::cout << "Test 2 - Add 90 minutes to 10:30: " << clock2.toString() << std::endl;
    
    // Test case 3: Subtract minutes
    Task199 clock3(10, 30);
    clock3.subtractMinutes(45);
    std::cout << "Test 3 - Subtract 45 minutes from 10:30: " << clock3.toString() << std::endl;
    
    // Test case 4: Clock equality
    Task199 clock4a(15, 45);
    Task199 clock4b(15, 45);
    std::cout << "Test 4 - Clock equality (15:45 == 15:45): " << (clock4a.equals(clock4b) ? "true" : "false") << std::endl;
    
    // Test case 5: Wrap around midnight
    Task199 clock5(23, 30);
    clock5.addMinutes(45);
    std::cout << "Test 5 - Add 45 minutes to 23:30 (wrap around): " << clock5.toString() << std::endl;
    
    return 0;
}
