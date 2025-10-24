
#include <iostream>
#include <iomanip>
#include <string>

class Task199 {
private:
    int hours;
    int minutes;
    
public:
    Task199(int h, int m) {
        int totalMinutes = (h * 60 + m) % (24 * 60);
        if (totalMinutes < 0) {
            totalMinutes += 24 * 60;
        }
        hours = totalMinutes / 60;
        minutes = totalMinutes % 60;
    }
    
    Task199 addMinutes(int minutesToAdd) const {
        int totalMinutes = hours * 60 + minutes + minutesToAdd;
        return Task199(0, totalMinutes);
    }
    
    Task199 subtractMinutes(int minutesToSubtract) const {
        return addMinutes(-minutesToSubtract);
    }
    
    bool equals(const Task199& other) const {
        return hours == other.hours && minutes == other.minutes;
    }
    
    std::string toString() const {
        char buffer[6];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
        return std::string(buffer);
    }
};

int main() {
    // Test case 1: Create a clock and display it
    Task199 clock1(10, 30);
    std::cout << "Test 1 - Clock at 10:30: " << clock1.toString() << std::endl;
    
    // Test case 2: Add minutes
    Task199 clock2 = clock1.addMinutes(45);
    std::cout << "Test 2 - Add 45 minutes to 10:30: " << clock2.toString() << std::endl;
    
    // Test case 3: Subtract minutes
    Task199 clock3 = clock1.subtractMinutes(90);
    std::cout << "Test 3 - Subtract 90 minutes from 10:30: " << clock3.toString() << std::endl;
    
    // Test case 4: Wrap around midnight (add)
    Task199 clock4(23, 45);
    Task199 clock5 = clock4.addMinutes(30);
    std::cout << "Test 4 - Add 30 minutes to 23:45: " << clock5.toString() << std::endl;
    
    // Test case 5: Equality check
    Task199 clock6(11, 15);
    Task199 clock7(11, 15);
    Task199 clock8(11, 16);
    std::cout << "Test 5 - 11:15 equals 11:15: " << (clock6.equals(clock7) ? "true" : "false") << std::endl;
    std::cout << "Test 5 - 11:15 equals 11:16: " << (clock6.equals(clock8) ? "true" : "false") << std::endl;
    
    return 0;
}
