#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <cstdint>
#include <limits>
#include <regex>
#include <algorithm>
#include <cassert>
#include <random>

class NameRegistry {
public:
    static NameRegistry& instance() {
        static NameRegistry inst;
        return inst;
    }

    std::string acquireUniqueName() {
        std::lock_guard<std::mutex> lock(mu_);
        if (names_.size() >= MAX_NAMES) {
            throw std::runtime_error("Name space exhausted: cannot allocate more unique names");
        }
        for (size_t attempts = 0; attempts < MAX_NAMES * 2ull; ++attempts) {
            std::string candidate = generateCandidateName();
            if (!validateNameFormat(candidate)) continue;
            auto res = names_.insert(candidate);
            if (res.second) {
                return candidate;
            }
        }
        throw std::runtime_error("Unable to acquire a unique name after many attempts");
    }

    bool releaseName(const std::string& name) {
        std::lock_guard<std::mutex> lock(mu_);
        auto it = names_.find(name);
        if (it != names_.end()) {
            names_.erase(it);
            return true;
        }
        return false;
    }

private:
    NameRegistry() = default;

    static constexpr size_t MAX_NAMES = 26u * 26u * 1000u;

    // Use std::random_device as a source of non-deterministic randomness where available.
    std::random_device rd_;
    std::unordered_set<std::string> names_;
    std::mutex mu_;

    uint32_t rand32() {
        // Combine two calls to improve entropy width where rd() may be 16-bit on some systems.
        uint32_t a = static_cast<uint32_t>(rd_());
        uint32_t b = static_cast<uint32_t>(rd_());
        return (a << 16) ^ b;
    }

    uint32_t uniformBelow(uint32_t upperExclusive) {
        // Rejection sampling to avoid modulo bias
        if (upperExclusive == 0) return 0;
        uint32_t limit = std::numeric_limits<uint32_t>::max() - (std::numeric_limits<uint32_t>::max() % upperExclusive);
        uint32_t r;
        do {
            r = rand32();
        } while (r > limit);
        return r % upperExclusive;
    }

    std::string generateCandidateName() {
        std::string s(5, 'A');
        s[0] = static_cast<char>('A' + uniformBelow(26));
        s[1] = static_cast<char>('A' + uniformBelow(26));
        s[2] = static_cast<char>('0' + uniformBelow(10));
        s[3] = static_cast<char>('0' + uniformBelow(10));
        s[4] = static_cast<char>('0' + uniformBelow(10));
        return s;
    }

    bool validateNameFormat(const std::string& name) const {
        if (name.size() != 5) return false;
        for (int i = 0; i < 2; ++i) {
            if (name[i] < 'A' || name[i] > 'Z') return false;
        }
        for (int i = 2; i < 5; ++i) {
            if (name[i] < '0' || name[i] > '9') return false;
        }
        return true;
    }
};

class Robot {
public:
    Robot() : name_("") {}

    std::string getName() {
        if (name_.empty()) {
            name_ = NameRegistry::instance().acquireUniqueName();
        }
        return name_;
    }

    bool reset() {
        if (name_.empty()) return false;
        bool removed = NameRegistry::instance().releaseName(name_);
        name_.clear();
        return removed;
    }

private:
    std::string name_;
};

// Test helpers
static bool isUnique(const std::vector<std::string>& names) {
    std::unordered_set<std::string> s(names.begin(), names.end());
    return s.size() == names.size();
}

// Tests
static bool test1_singleRobotNamePattern() {
    Robot r;
    std::string n = r.getName();
    if (n.size() != 5) return false;
    for (int i = 0; i < 2; ++i) if (n[i] < 'A' || n[i] > 'Z') return false;
    for (int i = 2; i < 5; ++i) if (n[i] < '0' || n[i] > '9') return false;
    return true;
}

static bool test2_multipleRobotsUnique() {
    std::vector<Robot> robots(10);
    std::vector<std::string> names;
    names.reserve(robots.size());
    for (auto& r : robots) names.push_back(r.getName());
    return isUnique(names);
}

static bool test3_resetChangesAndUnique() {
    Robot r1, r2;
    std::string n1 = r1.getName();
    std::string n2 = r2.getName();
    if (n1 == n2) return false;
    r1.reset();
    std::string n1b = r1.getName();
    if (n1 == n1b) return false;
    if (n1b == r2.getName()) return false;
    return true;
}

static bool test4_repeatedResetsYieldDifferentNames() {
    Robot r;
    std::unordered_set<std::string> seen;
    for (int i = 0; i < 6; ++i) {
        std::string n = r.getName();
        if (seen.find(n) != seen.end()) return false;
        seen.insert(n);
        r.reset();
    }
    return true;
}

static bool test5_stressUniqueness() {
    const int N = 500;
    std::vector<Robot> robots(N);
    std::vector<std::string> names;
    names.reserve(N);
    for (auto& r : robots) names.push_back(r.getName());
    return isUnique(names);
}

int main() {
    std::cout << "Test1 pattern: " << (test1_singleRobotNamePattern() ? "true" : "false") << "\n";
    std::cout << "Test2 uniqueness among 10: " << (test2_multipleRobotsUnique() ? "true" : "false") << "\n";
    std::cout << "Test3 reset changes and unique: " << (test3_resetChangesAndUnique() ? "true" : "false") << "\n";
    std::cout << "Test4 repeated resets yield different names: " << (test4_repeatedResetsYieldDifferentNames() ? "true" : "false") << "\n";
    std::cout << "Test5 stress uniqueness among 500: " << (test5_stressUniqueness() ? "true" : "false") << "\n";
    return 0;
}