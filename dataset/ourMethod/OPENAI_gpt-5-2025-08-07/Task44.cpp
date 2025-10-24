#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>
#include <cerrno>
#include <cstdlib>
#include <iomanip>

struct StringAnalysis {
    size_t length;
    size_t letters;
    size_t digits;
    bool isPalindrome;
    std::string reversed;
};

static bool getLineLimited(std::istream& in, std::string& out, size_t maxLen) {
    std::string line;
    if (!std::getline(in, line)) {
        return false;
    }
    if (line.size() > maxLen) {
        return false;
    }
    out = line;
    return true;
}

static bool isPrintableASCII(char c) {
    unsigned char uc = static_cast<unsigned char>(c);
    return uc >= 32 && uc <= 126;
}

static bool sanitizeString(const std::string& input, std::string& sanitized, size_t maxLen) {
    sanitized.clear();
    if (input.size() > maxLen) {
        return false;
    }
    for (char c : input) {
        if (!isPrintableASCII(c)) {
            // Reject non-printable to avoid control injection
            return false;
        }
        sanitized.push_back(c);
    }
    // No further transformation; keep original printable content
    return true;
}

static StringAnalysis analyzeString(const std::string& s) {
    StringAnalysis sa{};
    sa.length = s.size();
    sa.letters = 0;
    sa.digits = 0;
    for (char c : s) {
        if (std::isalpha(static_cast<unsigned char>(c))) sa.letters++;
        if (std::isdigit(static_cast<unsigned char>(c))) sa.digits++;
    }
    sa.reversed = s;
    std::reverse(sa.reversed.begin(), sa.reversed.end());

    // Palindrome check: alnum-only, case-insensitive
    std::string norm;
    norm.reserve(s.size());
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalnum(uc)) {
            norm.push_back(static_cast<char>(std::tolower(uc)));
        }
    }
    std::string normRev = norm;
    std::reverse(normRev.begin(), normRev.end());
    sa.isPalindrome = (norm == normRev);
    return sa;
}

static bool parseIntToken(const char* begin, const char* end, long long& out) {
    // Parse [begin, end) as a base-10 integer with full consumption
    errno = 0;
    char* parseEnd = nullptr;
    // Create a temporary null-terminated buffer safely
    size_t len = static_cast<size_t>(end - begin);
    if (len == 0 || len > 64) return false; // token must have at least 1 char, limit length
    char buf[65];
    std::fill(std::begin(buf), std::end(buf), 0);
    std::memcpy(buf, begin, len);
    buf[len] = '\0';
    long long val = std::strtoll(buf, &parseEnd, 10);
    if (errno != 0) return false;
    if (parseEnd == buf) return false; // no digits parsed
    // Ensure full consumption
    while (*parseEnd != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*parseEnd))) return false;
        ++parseEnd;
    }
    out = val;
    return true;
}

static bool parseIntList(const std::string& line, std::vector<long long>& out, size_t maxCount, long long minV, long long maxV) {
    out.clear();
    if (line.size() > 2048) return false; // cap input processing
    const char* p = line.c_str();
    const char* e = p + line.size();
    while (p < e) {
        // Skip whitespace and commas
        while (p < e && (std::isspace(static_cast<unsigned char>(*p)) || *p == ',')) ++p;
        if (p >= e) break;
        const char* start = p;

        // Accept optional sign
        if (*p == '+' || *p == '-') ++p;
        bool hasDigit = false;
        while (p < e && std::isdigit(static_cast<unsigned char>(*p))) {
            hasDigit = true;
            ++p;
        }
        const char* endTok = p;

        // Validate token has digits
        if (!hasDigit) return false;

        // Next char must be delimiter or end
        if (p < e && !(std::isspace(static_cast<unsigned char>(*p)) || *p == ',')) {
            return false;
        }

        long long val = 0;
        if (!parseIntToken(start, endTok, val)) return false;
        if (val < minV || val > maxV) return false;

        out.push_back(val);
        if (out.size() > maxCount) return false;
    }
    return !out.empty();
}

struct NumberStats {
    size_t count;
    long long min;
    long long max;
    long long sum;
    double average;
};

static NumberStats analyzeNumbers(const std::vector<long long>& nums) {
    NumberStats ns{};
    if (nums.empty()) {
        return ns;
    }
    ns.count = nums.size();
    ns.min = nums[0];
    ns.max = nums[0];
    long long sum = 0;
    for (long long v : nums) {
        if (v < ns.min) ns.min = v;
        if (v > ns.max) ns.max = v;
        sum += v;
    }
    ns.sum = sum;
    ns.average = static_cast<double>(sum) / static_cast<double>(ns.count);
    return ns;
}

// --------- Tests ----------
static void runStringTests() {
    const size_t MAXLEN = 256;
    std::vector<std::string> tests = {
        "Hello, World!",
        "RaceCar",
        "123321",
        "abc123",
        ""
    };
    std::cout << "String Tests:\n";
    for (size_t i = 0; i < tests.size(); ++i) {
        std::string sanitized;
        bool ok = sanitizeString(tests[i], sanitized, MAXLEN);
        std::cout << "Test " << (i + 1) << ": ";
        if (!ok) {
            std::cout << "invalid input\n";
            continue;
        }
        StringAnalysis sa = analyzeString(sanitized);
        std::cout << "len=" << sa.length
                  << " letters=" << sa.letters
                  << " digits=" << sa.digits
                  << " palindrome=" << (sa.isPalindrome ? "yes" : "no")
                  << " reversed=\"" << sa.reversed << "\"\n";
    }
}

static void runNumberTests() {
    const long long MINV = -1000000;
    const long long MAXV =  1000000;
    const size_t MAXC = 100;
    std::vector<std::string> tests = {
        "1 2 3 4 5",
        "-5, 10, -15, 20",
        "1000000 -1000000 0",
        "42",
        "1, 2, x" // invalid
    };
    std::cout << "Number Tests:\n";
    for (size_t i = 0; i < tests.size(); ++i) {
        std::vector<long long> nums;
        bool ok = parseIntList(tests[i], nums, MAXC, MINV, MAXV);
        std::cout << "Test " << (i + 1) << ": ";
        if (!ok) {
            std::cout << "invalid input\n";
            continue;
        }
        NumberStats ns = analyzeNumbers(nums);
        std::cout << "count=" << ns.count
                  << " min=" << ns.min
                  << " max=" << ns.max
                  << " sum=" << ns.sum
                  << " avg=" << std::fixed << std::setprecision(2) << ns.average << "\n";
    }
}

// --------- Interactive ----------
static void interactive() {
    const size_t MAX_INPUT = 1024;
    std::cout << "\nInteractive mode:\n";
    std::cout << "Choose operation:\n";
    std::cout << "1) Analyze a string\n";
    std::cout << "2) Analyze a list of integers\n";
    std::cout << "Enter choice (1 or 2): ";
    std::string choice;
    if (!getLineLimited(std::cin, choice, 8)) {
        std::cout << "Input error.\n";
        return;
    }
    if (choice != "1" && choice != "2") {
        std::cout << "Invalid choice.\n";
        return;
    }
    if (choice == "1") {
        std::cout << "Enter a printable ASCII string (max " << MAX_INPUT << " chars): ";
        std::string line;
        if (!getLineLimited(std::cin, line, MAX_INPUT)) {
            std::cout << "Input too long or error.\n";
            return;
        }
        std::string sanitized;
        if (!sanitizeString(line, sanitized, MAX_INPUT)) {
            std::cout << "Invalid characters detected.\n";
            return;
        }
        StringAnalysis sa = analyzeString(sanitized);
        std::cout << "Length: " << sa.length << "\n";
        std::cout << "Letters: " << sa.letters << "\n";
        std::cout << "Digits: " << sa.digits << "\n";
        std::cout << "Palindrome (alnum-only, case-insensitive): " << (sa.isPalindrome ? "yes" : "no") << "\n";
        std::cout << "Reversed: " << sa.reversed << "\n";
    } else {
        std::cout << "Enter integers separated by spaces or commas (range -1000000..1000000, max 100 numbers): ";
        std::string line;
        if (!getLineLimited(std::cin, line, MAX_INPUT)) {
            std::cout << "Input too long or error.\n";
            return;
        }
        std::vector<long long> nums;
        if (!parseIntList(line, nums, 100, -1000000, 1000000)) {
            std::cout << "Invalid number list.\n";
            return;
        }
        NumberStats ns = analyzeNumbers(nums);
        std::cout << "Count: " << ns.count << "\n";
        std::cout << "Min: " << ns.min << "\n";
        std::cout << "Max: " << ns.max << "\n";
        std::cout << "Sum: " << ns.sum << "\n";
        std::cout << "Average: " << std::fixed << std::setprecision(2) << ns.average << "\n";
    }
}

int main() {
    // 5 test cases each category
    runStringTests();
    runNumberTests();

    // Interactive demonstration
    interactive();
    return 0;
}