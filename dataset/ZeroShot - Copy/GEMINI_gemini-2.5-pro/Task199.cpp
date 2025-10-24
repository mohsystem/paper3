#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

class Task199 {
private:
    static const int MINUTES_PER_HOUR = 60;
    static const int HOURS_PER_DAY = 24;
    static const int MINUTES_PER_DAY = MINUTES_PER_HOUR * HOURS_PER_DAY;

    int totalMinutes;

    static int normalize(int minutes) {
        // C++'s % operator can yield negative results for negative inputs
        return (minutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
    }

public:
    Task199(int hour, int minute) {
        this->totalMinutes = normalize(hour * MINUTES_PER_HOUR + minute);
    }

    void add(int minutes) {
        this->totalMinutes = normalize(this->totalMinutes + minutes);
    }

    void subtract(int minutes) {
        add(-minutes);
    }

    std::string toString() const {
        int hour = totalMinutes / MINUTES_PER_HOUR;
        int minute = totalMinutes % MINUTES_PER_HOUR;
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << hour << ":"
            << std::setw(2) << std::setfill('0') << minute;
        return oss.str();
    }

    bool operator==(const Task199& other) const {
        return this->totalMinutes == other.totalMinutes;
    }
};

int main() {
    // Test Case 1: Create a clock and test toString
    Task199 clock1(10, 30);
    std::cout << "Test Case 1: Create clock (10:30) -> " << clock1.toString() << std::endl;

    // Test Case 2: Add minutes without day rollover
    clock1.add(20);
    std::cout << "Test Case 2: Add 20 mins to 10:30 -> " << clock1.toString() << std::endl;

    // Test Case 3: Add minutes with day rollover
    Task199 clock2(23, 50);
    clock2.add(20);
    std::cout << "Test Case 3: Add 20 mins to 23:50 -> " << clock2.toString() << std::endl;

    // Test Case 4: Subtract minutes with day rollover
    Task199 clock3(0, 10);
    clock3.subtract(20);
    std::cout << "Test Case 4: Subtract 20 mins from 00:10 -> " << clock3.toString() << std::endl;
    
    // Test Case 5: Equality
    Task199 clockA(14, 30);
    Task199 clockB(14, 30);
    Task199 clockC(10, 0);
    Task199 clockD(38, 30); // equivalent to 14:30
    std::cout << "Test Case 5: Equality Check" << std::endl;
    std::cout << "  14:30 == 14:30 -> " << std::boolalpha << (clockA == clockB) << std::endl;
    std::cout << "  14:30 == 10:00 -> " << std::boolalpha << (clockA == clockC) << std::endl;
    std::cout << "  14:30 == normalized 38:30 -> " << std::boolalpha << (clockA == clockD) << std::endl;

    return 0;
}