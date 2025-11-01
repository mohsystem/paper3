#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cstdlib>

static std::vector<std::string> createElements() {
    return {"alpha", "bravo", "charlie", "delta", "echo", "foxtrot"};
}

// Returns true on success and writes the element to out. Returns false if invalid.
static bool getElementAt(const std::vector<std::string>& arr, long long idx, std::string& out) {
    if (idx < 0 || static_cast<size_t>(idx) >= arr.size()) {
        return false;
    }
    out = arr[static_cast<size_t>(idx)];
    return true;
}

static bool safeParseIndex(const char* s, long long& out) {
    if (s == nullptr) return false;
    std::string str(s);
    // Trim spaces
    auto l = str.find_first_not_of(" \t\r\n");
    auto r = str.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) return false;
    str = str.substr(l, r - l + 1);
    if (str.empty() || str.size() > 32) return false;

    char* endptr = nullptr;
    errno = 0;
    long long val = std::strtoll(str.c_str(), &endptr, 10);
    if (errno != 0 || endptr == str.c_str() || *endptr != '\0') {
        return false;
    }
    out = val;
    return true;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> elements = createElements();

    if (argc >= 2) {
        long long idx = 0;
        if (!safeParseIndex(argv[1], idx)) {
            std::cout << "Invalid input" << std::endl;
            return 0;
        }
        std::string value;
        if (getElementAt(elements, idx, value)) {
            std::cout << value << std::endl;
        } else {
            std::cout << "Invalid index" << std::endl;
        }
        return 0;
    }

    // 5 test cases
    std::vector<long long> tests = {0, 2, 5, -1, 6};
    for (long long t : tests) {
        std::string value;
        if (getElementAt(elements, t, value)) {
            std::cout << "Index " << t << " -> " << value << std::endl;
        } else {
            std::cout << "Index " << t << " -> Invalid index" << std::endl;
        }
    }
    return 0;
}