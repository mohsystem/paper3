
#include <iostream>
#include <stdexcept>
#include <iomanip>

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
        if (hours < 0 || minutes < 0) {
            throw std::invalid_argument("Hours and minutes cannot be negative");
        }
        int totalMinutes = hours * 60 + minutes;
        normalize(totalMinutes);
    }
    
    Task199 addMinutes(int minutesToAdd) const {
        int totalMinutes = this->hours * 60 + this->minutes + minutesToAdd;
        return Task199(0, totalMinutes);
    }
    
    Task199 subtractMinutes(int minutesToSubtract) const {
        return addMinutes(-minutesToSubtract);
    }
    
    bool operator==(const Task199& other) const {
        return this->hours == other.hours && this->minutes == other.minutes;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Task199& clock) {
        os << std::setfill('0') << std::setw(2) << clock.hours << ":" 
           << std::setfill('0') << std::setw(2) << clock.minutes;
        return os;
    }
};

int main() {
    // Test case 1: Basic clock creation
    Task199 clock1(10, 30);
    std::cout << "Test 1 - Clock1: " << clock1 << std::endl;
    
    // Test case 2: Adding minutes
    Task199 clock2 = clock1.addMinutes(45);
    std::cout << "Test 2 - Clock1 + 45 minutes: " << clock2 << std::endl;
    
    // Test case 3: Subtracting minutes
    Task199 clock3 = clock2.subtractMinutes(90);
    std::cout << "Test 3 - Clock2 - 90 minutes: " << clock3 << std::endl;
    
    // Test case 4: Equality check
    Task199 clock4(10, 30);
    std::cout << "Test 4 - Clock1 equals Clock4: " << (clock1 == clock4 ? "true" : "false") << std::endl;
    
    // Test case 5: Wrap around midnight
    Task199 clock5(23, 45);
    Task199 clock6 = clock5.addMinutes(30);
    std::cout << "Test 5 - 23:45 + 30 minutes: " << clock6 << std::endl;
    
    return 0;
}
