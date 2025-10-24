#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

class Clock {
private:
    static const int MINUTES_PER_HOUR = 60;
    static const int HOURS_PER_DAY = 24;
    static const int MINUTES_PER_DAY = HOURS_PER_DAY * MINUTES_PER_HOUR;

    int totalMinutes;

public:
    Clock(int hour, int minute) {
        int effectiveTotalMinutes = hour * MINUTES_PER_HOUR + minute;
        // The formula (a % n + n) % n ensures the result is always positive
        // in the range [0, n-1], which is necessary because C++'s %
        // operator can yield negative results.
        totalMinutes = (effectiveTotalMinutes % MINUTES_PER_DAY + MINUTES_PER_DAY) % MINUTES_PER_DAY;
    }

    Clock add(int minutes) const {
        return Clock(0, this->totalMinutes + minutes);
    }

    Clock subtract(int minutes) const {
        return Clock(0, this->totalMinutes - minutes);
    }
    
    std::string toString() const {
        int hour = totalMinutes / MINUTES_PER_HOUR;
        int minute = totalMinutes % MINUTES_PER_HOUR;
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << hour << ":"
            << std::setw(2) << std::setfill('0') << minute;
        return oss.str();
    }

    bool operator==(const Clock& other) const {
        return this->totalMinutes == other.totalMinutes;
    }

    bool operator!=(const Clock& other) const {
        return !(*this == other);
    }
};

// Overload the << operator for easy printing with std::cout
std::ostream& operator<<(std::ostream& os, const Clock& clock) {
    os << clock.toString();
    return os;
}

void run_tests() {
    // Test Case 1: Create a clock and check its string representation
    Clock clock1(10, 30);
    std::cout << "Test 1: Create 10:30 -> " << clock1 << std::endl;

    // Test Case 2: Add minutes
    Clock clock2 = clock1.add(70);
    std::cout << "Test 2: 10:30 + 70 min -> " << clock2 << std::endl;

    // Test Case 3: Subtract minutes, wrapping backwards
    Clock clock3 = Clock(0, 15).subtract(30);
    std::cout << "Test 3: 00:15 - 30 min -> " << clock3 << std::endl;

    // Test Case 4: Equality check with different but equivalent inputs
    Clock clock4a(14, 0);
    Clock clock4b(13, 60);
    std::cout << "Test 4: 14:00 equals 13:60 -> " << std::boolalpha << (clock4a == clock4b) << std::endl;

    // Test Case 5: Add minutes, wrapping forwards past midnight
    Clock clock5 = Clock(23, 50).add(20);
    std::cout << "Test 5: 23:50 + 20 min -> " << clock5 << std::endl;
}

int main() {
    run_tests();
    return 0;
}