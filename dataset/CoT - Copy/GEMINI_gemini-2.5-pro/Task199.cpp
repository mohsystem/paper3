#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

class Clock {
private:
    int hours;
    int minutes;
    static const int MINUTES_IN_DAY = 24 * 60;

public:
    Clock(int h, int m) {
        int totalMinutes = h * 60 + m;
        // The formula (a % n + n) % n handles negative results correctly.
        totalMinutes = (totalMinutes % MINUTES_IN_DAY + MINUTES_IN_DAY) % MINUTES_IN_DAY;
        this->hours = totalMinutes / 60;
        this->minutes = totalMinutes % 60;
    }

    Clock add(int minutesToAdd) const {
        return Clock(this->hours, this->minutes + minutesToAdd);
    }

    Clock subtract(int minutesToSubtract) const {
        return Clock(this->hours, this->minutes - minutesToSubtract);
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << this->hours << ":"
            << std::setw(2) << std::setfill('0') << this->minutes;
        return oss.str();
    }

    bool operator==(const Clock& other) const {
        return this->hours == other.hours && this->minutes == other.minutes;
    }
};

class Task199 {
public:
    static void run_tests() {
        // Test Case 1: Simple creation and toString
        Clock clock1(8, 30);
        std::cout << "Test Case 1: Simple creation" << std::endl;
        std::cout << "Clock(8, 30) -> " << clock1.toString() << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 2: Adding minutes without hour rollover
        Clock clock2 = Clock(10, 0).add(25);
        std::cout << "Test Case 2: Add minutes (no rollover)" << std::endl;
        std::cout << "Clock(10, 0).add(25) -> " << clock2.toString() << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 3: Adding minutes with hour and day rollover
        Clock clock3 = Clock(23, 45).add(30);
        std::cout << "Test Case 3: Add minutes (with rollover)" << std::endl;
        std::cout << "Clock(23, 45).add(30) -> " << clock3.toString() << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 4: Subtracting minutes with hour and day rollover
        Clock clock4 = Clock(0, 10).subtract(30);
        std::cout << "Test Case 4: Subtract minutes (with rollover)" << std::endl;
        std::cout << "Clock(0, 10).subtract(30) -> " << clock4.toString() << std::endl;
        std::cout << "--------------------" << std::endl;

        // Test Case 5: Equality and normalization
        Clock clock5a(15, 30);
        Clock clock5b(14, 90);
        Clock clock5c(16, -30);
        Clock clock5d(39, 30);
        Clock clock5e(15, 31);
        std::cout << "Test Case 5: Equality and normalization" << std::endl;
        std::cout << std::boolalpha;
        std::cout << "Clock(15, 30) equals Clock(14, 90)? " << (clock5a == clock5b) << std::endl;
        std::cout << "Clock(15, 30) equals Clock(16, -30)? " << (clock5a == clock5c) << std::endl;
        std::cout << "Clock(15, 30) equals Clock(39, 30)? " << (clock5a == clock5d) << std::endl;
        std::cout << "Clock(15, 30) equals Clock(15, 31)? " << (clock5a == clock5e) << std::endl;
        std::cout << "--------------------" << std::endl;
    }
};

int main() {
    Task199::run_tests();
    return 0;
}