#include <iostream>
#include <string>
#include <unordered_set>
#include <random>
#include <vector>

// Chain-of-Through: 1) Problem understanding + 2) Security requirements
// - Unique active names with unpredictable generation.
// - Use std::random_device for non-deterministic numbers.
// - Maintain a registry to ensure uniqueness and release on reset.

class NameRegistry {
public:
    static std::string acquireUniqueName() {
        // Attempt loop to avoid collisions
        for (int attempts = 0; attempts < MAX_ATTEMPTS; ++attempts) {
            std::string candidate = randomName();
            if (active().find(candidate) == active().end()) {
                active().insert(candidate);
                return candidate;
            }
        }
        throw std::runtime_error("Unable to generate a unique robot name after many attempts.");
    }

    static void releaseName(const std::string& name) {
        if (!name.empty()) {
            active().erase(name);
        }
    }

    static bool isActive(const std::string& name) {
        return active().find(name) != active().end();
    }

private:
    static std::unordered_set<std::string>& active() {
        static std::unordered_set<std::string> s;
        return s;
    }

    static std::string randomName() {
        static thread_local std::random_device rd;
        static thread_local std::uniform_int_distribution<int> letterDist(0, 25);
        static thread_local std::uniform_int_distribution<int> digitDist(0, 9);

        std::string s;
        s.reserve(5);
        s.push_back(static_cast<char>('A' + letterDist(rd)));
        s.push_back(static_cast<char>('A' + letterDist(rd)));
        s.push_back(static_cast<char>('0' + digitDist(rd)));
        s.push_back(static_cast<char>('0' + digitDist(rd)));
        s.push_back(static_cast<char>('0' + digitDist(rd)));
        return s;
    }

    static constexpr int MAX_ATTEMPTS = 1000000;
};

class Robot {
public:
    Robot() = default;

    std::string getName() {
        if (name_.empty()) {
            name_ = NameRegistry::acquireUniqueName();
        }
        return name_;
    }

    void reset() {
        if (!name_.empty()) {
            NameRegistry::releaseName(name_);
            name_.clear();
        }
    }

private:
    std::string name_;
};

// Chain-of-Through: 3) Secure coding + 4) Review: deterministic bugs avoided with rd-based randomness; registry enforces uniqueness.

int main() {
    // Test 1
    Robot r1;
    std::string n1 = r1.getName();
    std::cout << "Test1 r1 name: " << n1 << " active=" << (NameRegistry::isActive(n1) ? "true" : "false") << "\n";

    // Test 2
    Robot r2;
    std::string n2 = r2.getName();
    std::cout << "Test2 r2 name: " << n2 << " unique_vs_r1=" << (n1 != n2 ? "true" : "false") << "\n";

    // Test 3
    std::string n2_again = r2.getName();
    std::cout << "Test3 r2 stable: " << n2 << " == " << n2_again << " -> " << (n2 == n2_again ? "true" : "false") << "\n";

    // Test 4
    r2.reset();
    std::string n2_new = r2.getName();
    std::cout << "Test4 r2 reset new name: " << n2_new << " old_different=" << (n2 != n2_new ? "true" : "false") << "\n";

    // Test 5
    int count = 1000;
    std::vector<Robot> robots(count);
    std::unordered_set<std::string> seen;
    bool unique = true;
    for (int i = 0; i < count; ++i) {
        std::string nm = robots[i].getName();
        if (!seen.insert(nm).second) {
            unique = false;
            break;
        }
    }
    std::cout << "Test5 1000 robots unique=" << (unique ? "true" : "false") << " size=" << seen.size() << "\n";
    return 0;
}