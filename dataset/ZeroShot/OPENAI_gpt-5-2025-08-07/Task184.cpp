#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <mutex>

class Robot {
public:
    std::string getName() {
        if (name.empty()) {
            name = newUniqueName();
        }
        return name;
    }

    void reset() {
        name.clear();
    }

private:
    std::string name;

    static std::string newUniqueName() {
        // 26*26*1000 possible names
        for (int attempts = 0; attempts < 700000; ++attempts) {
            std::string candidate = randomName();
            std::lock_guard<std::mutex> lock(getMutex());
            if (getAllocated().insert(candidate).second) {
                return candidate;
            }
        }
        throw std::runtime_error("Exhausted all possible unique names");
    }

    static std::string randomName() {
        static thread_local std::random_device rd;
        static thread_local std::mt19937_64 gen(rd());
        std::uniform_int_distribution<int> distA(0, 25);
        std::uniform_int_distribution<int> distD(0, 9);

        std::string s;
        s.reserve(5);
        s.push_back(static_cast<char>('A' + distA(gen)));
        s.push_back(static_cast<char>('A' + distA(gen)));
        s.push_back(static_cast<char>('0' + distD(gen)));
        s.push_back(static_cast<char>('0' + distD(gen)));
        s.push_back(static_cast<char>('0' + distD(gen)));
        return s;
    }

    static std::unordered_set<std::string>& getAllocated() {
        static std::unordered_set<std::string> allocated;
        return allocated;
    }

    static std::mutex& getMutex() {
        static std::mutex mtx;
        return mtx;
    }
};

int main() {
    Robot r1, r2, r3;

    std::string n1 = r1.getName();
    std::string n2 = r2.getName();
    std::string n3 = r3.getName();

    std::cout << n1 << "\n";
    std::cout << n2 << "\n";
    std::cout << n3 << "\n";

    std::string old2 = n2;
    r2.reset();
    std::string n2b = r2.getName();
    std::cout << n2b << "\n";

    Robot r4, r5;
    std::string n4 = r4.getName();
    std::string n5 = r5.getName();
    std::cout << n4 << "\n";
    std::cout << n5 << "\n";

    // Verify uniqueness of the five names after reset
    std::unordered_set<std::string> uniq;
    uniq.insert(n1);
    uniq.insert(n2b);
    uniq.insert(n3);
    uniq.insert(n4);
    uniq.insert(n5);
    if (uniq.size() != 5) {
        std::cerr << "Names are not unique\n";
        return 1;
    }
    if (old2 == n2b) {
        std::cerr << "Reset did not change the name\n";
        return 1;
    }
    return 0;
}