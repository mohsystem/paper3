#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

struct Clock {
    int hour;
    int minute;

    static Clock make(int hours, int minutes) {
        long long total = static_cast<long long>(hours) * 60LL + static_cast<long long>(minutes);
        long long mod = total % 1440LL;
        if (mod < 0) mod += 1440LL;
        Clock c;
        c.hour = static_cast<int>(mod / 60LL);
        c.minute = static_cast<int>(mod % 60LL);
        return c;
    }

    Clock addMinutes(int delta) const {
        return Clock::make(hour, minute + delta);
    }

    Clock subtractMinutes(int delta) const {
        return Clock::make(hour, minute - delta);
    }

    bool operator==(const Clock& other) const {
        return hour == other.hour && minute == other.minute;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0') << std::setw(2) << minute;
        return oss.str();
    }
};

int main() {
    // Test case 1: 10:00 + 3 minutes = 10:03
    Clock t1 = Clock::make(10, 0).addMinutes(3);
    std::cout << "Test1 " << t1.toString() << "\n";

    // Test case 2: 10:00 + 61 minutes = 11:01
    Clock t2 = Clock::make(10, 0).addMinutes(61);
    std::cout << "Test2 " << t2.toString() << "\n";

    // Test case 3: 23:59 + 2 minutes = 00:01
    Clock t3 = Clock::make(23, 59).addMinutes(2);
    std::cout << "Test3 " << t3.toString() << "\n";

    // Test case 4: 00:30 - 90 minutes = 23:00
    Clock t4 = Clock::make(0, 30).subtractMinutes(90);
    std::cout << "Test4 " << t4.toString() << "\n";

    // Test case 5: equality: 25:00 == 01:00
    Clock a = Clock::make(25, 0);
    Clock b = Clock::make(1, 0);
    std::cout << "Test5 " << a.toString() << " == " << b.toString() << " -> " << (a == b ? "true" : "false") << "\n";

    return 0;
}