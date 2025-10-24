#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

class Clock {
private:
    int totalMinutes;
    static const int MINUTES_IN_DAY = 24 * 60;

    static int normalize(int minutes) {
        // Handle negative results of % correctly for wraparound
        return (minutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
    }

public:
    Clock(int hours, int minutes) {
        totalMinutes = normalize(hours * 60 + minutes);
    }

    void add(int minutes) {
        totalMinutes = normalize(totalMinutes + minutes);
    }

    void subtract(int minutes) {
        add(-minutes);
    }

    std::string toString() const {
        std::ostringstream oss;
        int hours = totalMinutes / 60;
        int mins = totalMinutes % 60;
        oss << std::setw(2) << std::setfill('0') << hours << ":"
            << std::setw(2) << std::setfill('0') << mins;
        return oss.str();
    }

    bool operator==(const Clock& other) const {
        return this->totalMinutes == other.totalMinutes;
    }
};

int main() {
    std::cout << "CPP Test Cases:" << std::endl;
    std::cout << std::boolalpha; // Print booleans as true/false

    // Test 1: Creation and ToString
    Clock c1(8, 0);
    std::cout << "Test 1: Create 08:00 -> " << c1.toString() << std::endl;

    // Test 2: Equality
    Clock c2(8, 0);
    Clock c3(9, 0);
    std::cout << "Test 2: 08:00 == 08:00 -> " << (c1 == c2) << std::endl;
    std::cout << "Test 2: 08:00 == 09:00 -> " << (c1 == c3) << std::endl;

    // Test 3: Addition
    Clock c4(10, 0);
    c4.add(63);
    std::cout << "Test 3: 10:00 + 63 mins -> " << c4.toString() << std::endl;

    // Test 4: Subtraction with rollover
    Clock c5(0, 30);
    c5.subtract(60);
    std::cout << "Test 4: 00:30 - 60 mins -> " << c5.toString() << std::endl;

    // Test 5: Complex Addition with rollover
    Clock c6(23, 45);
    c6.add(180);
    std::cout << "Test 5: 23:45 + 180 mins -> " << c6.toString() << std::endl;

    return 0;
}