#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <stdexcept>

class Clock {
private:
    static constexpr int MINUTES_PER_DAY = 24 * 60;
    int minutes; // minutes since midnight, 0..1439

    static int normalize(long long total) {
        long long mod = total % MINUTES_PER_DAY;
        if (mod < 0) mod += MINUTES_PER_DAY;
        return static_cast<int>(mod);
    }

public:
    Clock(int hour, int minute = 0) {
        long long total = static_cast<long long>(hour) * 60LL + static_cast<long long>(minute);
        minutes = normalize(total);
    }

    int hour() const { return minutes / 60; }
    int minute() const { return minutes % 60; }

    Clock add(int minutesToAdd) const {
        long long total = static_cast<long long>(minutes) + static_cast<long long>(minutesToAdd);
        return Clock(0, normalize(total));
    }

    Clock subtract(int minutesToSubtract) const {
        return add(-minutesToSubtract);
    }

    bool operator==(const Clock& other) const {
        return minutes == other.minutes;
    }
    bool operator!=(const Clock& other) const {
        return !(*this == other);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hour() << ":"
            << std::setfill('0') << std::setw(2) << minute();
        return oss.str();
    }
};

// Example API functions that accept inputs and return outputs
Clock createClock(int hour, int minute) {
    return Clock(hour, minute);
}
Clock addMinutes(const Clock& c, int minutes) {
    return c.add(minutes);
}
Clock subtractMinutes(const Clock& c, int minutes) {
    return c.subtract(minutes);
}
bool clocksEqual(const Clock& a, const Clock& b) {
    return a == b;
}
std::string clockToString(const Clock& c) {
    return c.toString();
}

int main() {
    // 5 test cases
    Clock t1 = addMinutes(createClock(10, 0), 3);
    std::cout << "Test1: " << clockToString(t1) << "\n"; // 10:03

    Clock t2 = addMinutes(createClock(23, 59), 2);
    std::cout << "Test2: " << clockToString(t2) << "\n"; // 00:01

    Clock t3 = subtractMinutes(createClock(0, 3), 4);
    std::cout << "Test3: " << clockToString(t3) << "\n"; // 23:59

    Clock t4a = createClock(25, 0);
    Clock t4b = createClock(1, 0);
    std::cout << "Test4: equal=" << (clocksEqual(t4a, t4b) ? "true" : "false") << "\n"; // true

    Clock t5 = addMinutes(createClock(2, 30), -1500); // -25 hours
    std::cout << "Test5: " << clockToString(t5) << " equal 01:30? "
              << (clocksEqual(t5, createClock(1, 30)) ? "true" : "false") << "\n"; // 01:30 true

    return 0;
}