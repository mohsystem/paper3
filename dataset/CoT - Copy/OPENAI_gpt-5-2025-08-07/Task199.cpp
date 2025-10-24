#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

class Clock {
private:
    int minutes; // normalized [0,1439]

    static int normalize(long long total) {
        long long mod = total % 1440LL;
        if (mod < 0) mod += 1440LL;
        return static_cast<int>(mod);
    }

public:
    // Construct from hour and minute
    Clock(int hour, int minute) {
        long long total = static_cast<long long>(hour) * 60LL + static_cast<long long>(minute);
        minutes = normalize(total);
    }

    // Construct from total minutes
    static Clock fromTotalMinutes(long long totalMinutes) {
        return Clock(0, static_cast<int>(normalize(totalMinutes)));
    }

    // Add minutes and return new clock
    Clock plus(int minutesToAdd) const {
        long long total = static_cast<long long>(minutes) + static_cast<long long>(minutesToAdd);
        return fromTotalMinutes(total);
    }

    // Subtract minutes and return new clock
    Clock minus(int minutesToSub) const {
        long long total = static_cast<long long>(minutes) - static_cast<long long>(minutesToSub);
        return fromTotalMinutes(total);
    }

    // String "HH:MM"
    std::string toString() const {
        int h = minutes / 60;
        int m = minutes % 60;
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << h << ":" << std::setfill('0') << std::setw(2) << m;
        return oss.str();
    }

    bool operator==(const Clock& other) const {
        return minutes == other.minutes;
    }
    bool operator!=(const Clock& other) const {
        return !(*this == other);
    }
};

int main() {
    // Test 1: Basic creation and toString
    Clock c1(10, 37);
    std::cout << "Test1: " << c1.toString() << std::endl; // Expected 10:37

    // Test 2: Add minutes with hour rollover
    Clock c2 = Clock(10, 0).plus(3);
    std::cout << "Test2: " << c2.toString() << std::endl; // Expected 10:03

    // Test 3: Add minutes across midnight
    Clock c3 = Clock(23, 59).plus(2);
    std::cout << "Test3: " << c3.toString() << std::endl; // Expected 00:01

    // Test 4: Subtract minutes across midnight
    Clock c4 = Clock(0, 5).minus(10);
    std::cout << "Test4: " << c4.toString() << std::endl; // Expected 23:55

    // Test 5: Equality of clocks representing same time
    Clock c5a(24, 0);
    Clock c5b(48, 0);
    std::cout << "Test5: " << c5a.toString() << " == " << c5b.toString() << " -> " << (c5a == c5b ? "true" : "false") << std::endl; // Expected true

    return 0;
}