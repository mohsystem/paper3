#include <bits/stdc++.h>
using namespace std;

class Clock {
    int minutes; // minutes since 00:00 in [0,1439]
    static int normalize(long long totalMinutes) {
        long long m = totalMinutes % (24LL * 60LL);
        if (m < 0) m += 24LL * 60LL;
        return static_cast<int>(m);
    }
public:
    explicit Clock(long long totalMinutes = 0) : minutes(normalize(totalMinutes)) {}

    static Clock at(int hour, int minute) {
        return Clock(1LL * hour * 60 + minute);
    }

    Clock add(long long mins) const {
        return Clock(1LL * minutes + mins);
    }

    Clock subtract(long long mins) const {
        return Clock(1LL * minutes - mins);
    }

    int hour() const { return minutes / 60; }
    int minute() const { return minutes % 60; }

    string toString() const {
        ostringstream oss;
        oss << setw(2) << setfill('0') << hour() << ":"
            << setw(2) << setfill('0') << minute();
        return oss.str();
    }

    bool operator==(const Clock& other) const {
        return minutes == other.minutes;
    }
    bool operator!=(const Clock& other) const {
        return !(*this == other);
    }
};

// Example helper function that takes parameters and returns string
string clockAfterAdding(int h, int m, long long delta) {
    return Clock::at(h, m).add(delta).toString();
}

int main() {
    // 5 test cases
    Clock c1 = Clock::at(10, 0).add(3);
    cout << "Test1 " << c1.toString() << "\n"; // 10:03

    Clock c2 = Clock::at(23, 30).add(90);
    cout << "Test2 " << c2.toString() << "\n"; // 01:00

    Clock c3 = Clock::at(0, 10).subtract(20);
    cout << "Test3 " << c3.toString() << "\n"; // 23:50

    Clock c4a = Clock::at(15, 37);
    Clock c4b = Clock::at(14, 97); // same as 15:37
    cout << "Test4 equals " << (c4a == c4b ? "true" : "false") << "\n"; // true

    Clock c5a = Clock::at(5, 32);
    Clock c5b = c5a.add(24 * 60); // wrap full day
    cout << "Test5 equals " << (c5a == c5b ? "true" : "false") << " " << c5b.toString() << "\n"; // true 05:32

    return 0;
}