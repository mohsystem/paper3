#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include <cstdlib>
#include <cctype>
#include <algorithm>

int getValueFromArray(const std::vector<int>& arr, std::size_t size, int index) {
    if (size > arr.size()) {
        size = arr.size();
    }
    if (index < 0 || static_cast<std::size_t>(index) >= size) {
        throw std::out_of_range("index out of range");
    }
    return arr[static_cast<std::size_t>(index)];
}

static inline std::string trim(const std::string& s) {
    std::size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

void runTests() {
    std::vector<int> id_sequence {3, 14, 15, 92, 65, 35, 89, 79};
    std::size_t n = id_sequence.size();

    bool allOk = true;
    try {
        allOk &= (getValueFromArray(id_sequence, n, 0) == 3);
        allOk &= (getValueFromArray(id_sequence, n, 3) == 92);
        allOk &= (getValueFromArray(id_sequence, n, 7) == 79);
        try {
            (void)getValueFromArray(id_sequence, n, -1);
            allOk = false;
        } catch (const std::out_of_range&) {}
        try {
            (void)getValueFromArray(id_sequence, n, 100);
            allOk = false;
        } catch (const std::out_of_range&) {}
    } catch (...) {
        allOk = false;
    }

    if (!allOk) {
        std::cerr << "Tests failed\n";
    } else {
        std::cerr << "All tests passed\n";
    }
}

int main(int argc, char* argv[]) {
    std::vector<int> id_sequence {3, 14, 15, 92, 65, 35, 89, 79};
    std::size_t n = id_sequence.size();

    if (argc > 1 && std::string(argv[1]) == "test") {
        runTests();
        return 0;
    }

    std::string line;
    if (!std::getline(std::cin, line)) {
        return 1;
    }
    if (line.size() > 100) {
        return 1;
    }
    line = trim(line);
    if (line.empty()) {
        return 1;
    }

    try {
        std::size_t pos = 0;
        long long val = std::stoll(line, &pos, 10);
        if (pos != line.size()) {
            return 1;
        }
        if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
            return 1;
        }
        int idx = static_cast<int>(val);
        int result = getValueFromArray(id_sequence, n, idx);
        return result & 0xFF; // ensure portable exit code
    } catch (...) {
        return 1;
    }
}