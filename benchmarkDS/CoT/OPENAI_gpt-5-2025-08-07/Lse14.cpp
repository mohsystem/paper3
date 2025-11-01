// Chain-of-Through process in code generation:
// 1) Problem understanding: Provide a function to return id_sequence[index], plus a prompt-based wrapper.
// 2) Security requirements: Validate indices and user input; avoid undefined behavior.
// 3) Secure coding generation: Bounds checks, safe parsing using getline and stoi with try/catch.
// 4) Code review: No out-of-bounds access; handle parsing and I/O failures.
// 5) Secure code output: Return -1 on invalid input or errors.

#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <limits>

static inline std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

int getValueAtIndex(const std::vector<int>& id_sequence, int index) {
    if (index < 0 || static_cast<size_t>(index) >= id_sequence.size()) {
        return -1;
    }
    return id_sequence[static_cast<size_t>(index)];
}

int promptAndGetValue(const std::vector<int>& id_sequence) {
    std::cout << "Enter index: ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        return -1;
    }
    line = trim(line);
    if (line.empty()) {
        return -1;
    }
    try {
        size_t pos = 0;
        long long val = std::stoll(line, &pos, 10);
        if (pos != line.size()) {
            return -1;
        }
        if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) {
            return -1;
        }
        return getValueAtIndex(id_sequence, static_cast<int>(val));
    } catch (...) {
        return -1;
    }
}

int main() {
    std::vector<int> id_sequence{10, 20, 30, 40, 50};

    int testIndices[5] = {0, 2, 4, 1, 3};
    for (int idx : testIndices) {
        int result = getValueAtIndex(id_sequence, idx);
        std::cout << "Index " << idx << " -> " << result << "\n";
    }

    // Uncomment to test interactive prompt:
    // int userResult = promptAndGetValue(id_sequence);
    // std::cout << "User-selected value -> " << userResult << "\n";

    return 0;
}