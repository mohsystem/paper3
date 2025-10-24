#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <limits>

static constexpr std::size_t MAX_PARTS = 1000;
static constexpr std::size_t MAX_PART_LEN = 100000;
static constexpr std::size_t MAX_TOTAL_LEN = 1000000;

std::string safe_concat(const std::vector<std::string>& parts,
                        std::size_t max_total_len = MAX_TOTAL_LEN,
                        std::size_t max_part_len = MAX_PART_LEN) {
    if (parts.size() > MAX_PARTS) {
        throw std::invalid_argument("Too many parts.");
    }
    std::size_t total = 0;
    for (const auto& s : parts) {
        if (s.size() > max_part_len) {
            throw std::invalid_argument("A part exceeded maximum allowed length.");
        }
        if (total > max_total_len - s.size()) {
            throw std::invalid_argument("Total length exceeded maximum allowed length.");
        }
        total += s.size();
    }
    std::string out;
    out.reserve(total);
    for (const auto& s : parts) {
        out.append(s);
    }
    return out;
}

static void run_tests() {
    std::cout << safe_concat({"Hello", " ", "World"}) << "\n";
    std::cout << safe_concat({"", "abc", "", "def"}) << "\n";
    std::cout << safe_concat({"multi", "-", "part", "-", "concat"}) << "\n";
    std::cout << safe_concat({}) << "\n";
    std::cout << safe_concat({"Safe ", "concat ", "テスト"}) << "\n";
}

static bool parse_int(const std::string& s, int& out) {
    std::istringstream iss(s);
    long val;
    iss >> val;
    if (!iss || !iss.eof()) return false;
    if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) return false;
    out = static_cast<int>(val);
    return true;
}

static void read_from_user() {
    std::cout << "Enter number of strings to concatenate (0.." << MAX_PARTS << "):\n";
    std::string line;
    if (!std::getline(std::cin, line)) return;
    int n = 0;
    if (!parse_int(line, n) || n < 0 || static_cast<std::size_t>(n) > MAX_PARTS) {
        std::cerr << "ERROR: Invalid count.\n";
        return;
    }
    std::vector<std::string> parts;
    parts.reserve(static_cast<std::size_t>(n));
    std::size_t total = 0;
    for (int i = 0; i < n; ++i) {
        std::string s;
        if (!std::getline(std::cin, s)) {
            std::cerr << "ERROR: Not enough lines provided.\n";
            return;
        }
        if (s.size() > MAX_PART_LEN) {
            std::cerr << "ERROR: A part exceeded maximum allowed length.\n";
            return;
        }
        if (total > MAX_TOTAL_LEN - s.size()) {
            std::cerr << "ERROR: Total length exceeded maximum allowed length.\n";
            return;
        }
        total += s.size();
        parts.push_back(std::move(s));
    }
    try {
        std::string result = safe_concat(parts);
        std::cout << "Concatenated result:\n" << result << "\n";
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}

int main() {
    run_tests();
    read_from_user();
    return 0;
}