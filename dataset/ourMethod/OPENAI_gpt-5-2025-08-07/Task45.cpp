#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cctype>
#include <stdexcept>

namespace Safe {

static const size_t MAX_LINE_LEN = 1024;
static const size_t MAX_TEXT_LEN = 512;
static const size_t MAX_COUNT = 100;

static inline std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static inline std::string to_upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

static inline bool is_printable_ascii(char c) {
    return c >= 32 && c <= 126;
}

static inline std::string sanitize_text(const std::string& input, size_t maxLen) {
    std::string out;
    out.reserve(std::min(input.size(), maxLen));
    for (char c : input) {
        if (is_printable_ascii(c)) {
            out.push_back(c);
            if (out.size() >= maxLen) break;
        }
    }
    return out;
}

static inline bool safe_parse_ll(const std::string& s, long long& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    std::istringstream iss(t);
    long long v = 0;
    char extra = 0;
    iss >> v;
    if (!iss.fail()) {
        if (!(iss >> extra)) {
            out = v;
            return true;
        }
    }
    return false;
}

static inline bool safe_parse_double(const std::string& s, double& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    std::istringstream iss(t);
    double v = 0.0;
    char extra = 0;
    iss >> v;
    if (!iss.fail()) {
        if (!(iss >> extra)) {
            out = v;
            return true;
        }
    }
    return false;
}

static inline bool safe_add_ll(long long a, long long b, long long& res) {
    if ((b > 0 && a > (std::numeric_limits<long long>::max)() - b) ||
        (b < 0 && a < (std::numeric_limits<long long>::min)() - b)) {
        return false;
    }
    res = a + b;
    return true;
}

static inline bool safe_mul_ll(long long a, long long b, long long& res) {
    if (a == 0 || b == 0) { res = 0; return true; }
    if (a == -1) {
        if (b == (std::numeric_limits<long long>::min)()) return false;
        res = -b; return true;
    }
    if (b == -1) {
        if (a == (std::numeric_limits<long long>::min)()) return false;
        res = -a; return true;
    }
    long long maxv = (std::numeric_limits<long long>::max)();
    long long minv = (std::numeric_limits<long long>::min)();
    if (a > 0) {
        if (b > 0) {
            if (a > maxv / b) return false;
        } else {
            if (b < minv / a) return false;
        }
    } else {
        if (b > 0) {
            if (a < minv / b) return false;
        } else {
            if (a != 0 && b < maxv / a) return false;
        }
    }
    res = a * b;
    return true;
}

static inline std::string format_double(double v) {
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(6);
    oss << v;
    std::string s = oss.str();
    // Trim trailing zeros and optional dot
    size_t pos = s.find('.');
    if (pos != std::string::npos) {
        size_t end = s.size();
        while (end > pos && s[end - 1] == '0') --end;
        if (end > pos && s[end - 1] == '.') --end;
        s.erase(end);
    }
    if (s == "-0") s = "0";
    return s;
}

static inline std::vector<std::string> split_ws(const std::string& s) {
    std::vector<std::string> toks;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) toks.push_back(tok);
    return toks;
}

static inline std::string join_numbers(const std::vector<double>& nums) {
    std::ostringstream oss;
    for (size_t i = 0; i < nums.size(); ++i) {
        if (i) oss << ' ';
        oss << format_double(nums[i]);
    }
    return oss.str();
}

std::string process_line(const std::string& raw) {
    try {
        if (raw.size() > MAX_LINE_LEN) return "ERROR: line too long (max 1024)";
        std::string line = trim(raw);
        if (line.empty()) return "ERROR: empty input";
        // Extract command
        size_t sp = line.find_first_of(" \t");
        std::string cmd = to_upper(sp == std::string::npos ? line : line.substr(0, sp));
        std::string rest = sp == std::string::npos ? "" : trim(line.substr(sp + 1));
        if (cmd == "REVERSE") {
            std::string t = sanitize_text(rest, MAX_TEXT_LEN);
            std::reverse(t.begin(), t.end());
            return t;
        } else if (cmd == "UPPER") {
            std::string t = sanitize_text(rest, MAX_TEXT_LEN);
            for (char& c : t) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return t;
        } else if (cmd == "ADD") {
            auto toks = split_ws(rest);
            if (toks.size() != 2) return "ERROR: ADD expects 2 integers";
            long long a = 0, b = 0, r = 0;
            if (!safe_parse_ll(toks[0], a) || !safe_parse_ll(toks[1], b)) return "ERROR: invalid integer";
            if (!safe_add_ll(a, b, r)) return "ERROR: integer overflow on addition";
            return std::to_string(r);
        } else if (cmd == "MUL") {
            auto toks = split_ws(rest);
            if (toks.size() != 2) return "ERROR: MUL expects 2 integers";
            long long a = 0, b = 0, r = 0;
            if (!safe_parse_ll(toks[0], a) || !safe_parse_ll(toks[1], b)) return "ERROR: invalid integer";
            if (!safe_mul_ll(a, b, r)) return "ERROR: integer overflow on multiplication";
            return std::to_string(r);
        } else if (cmd == "AVG") {
            auto toks = split_ws(rest);
            if (toks.size() < 2) return "ERROR: AVG expects count followed by numbers";
            long long cntll = 0;
            if (!safe_parse_ll(toks[0], cntll) || cntll < 1 || cntll > static_cast<long long>(MAX_COUNT))
                return "ERROR: invalid count (1..100)";
            size_t cnt = static_cast<size_t>(cntll);
            if (toks.size() != cnt + 1) return "ERROR: count does not match number of values";
            long double sum = 0.0L;
            for (size_t i = 0; i < cnt; ++i) {
                double v = 0.0;
                if (!safe_parse_double(toks[i + 1], v)) return "ERROR: invalid number at position " + std::to_string(i + 1);
                sum += v;
            }
            double avg = static_cast<double>(sum / static_cast<long double>(cnt));
            return format_double(avg);
        } else if (cmd == "SORT") {
            auto toks = split_ws(rest);
            if (toks.size() < 2) return "ERROR: SORT expects count followed by numbers";
            long long cntll = 0;
            if (!safe_parse_ll(toks[0], cntll) || cntll < 1 || cntll > static_cast<long long>(MAX_COUNT))
                return "ERROR: invalid count (1..100)";
            size_t cnt = static_cast<size_t>(cntll);
            if (toks.size() != cnt + 1) return "ERROR: count does not match number of values";
            std::vector<double> nums;
            nums.reserve(cnt);
            for (size_t i = 0; i < cnt; ++i) {
                double v = 0.0;
                if (!safe_parse_double(toks[i + 1], v)) return "ERROR: invalid number at position " + std::to_string(i + 1);
                nums.push_back(v);
            }
            std::sort(nums.begin(), nums.end());
            return join_numbers(nums);
        } else {
            return "ERROR: unknown command";
        }
    } catch (const std::bad_alloc&) {
        return "ERROR: out of memory";
    } catch (...) {
        return "ERROR: unexpected error";
    }
}

} // namespace Safe

int main() {
    // 5 test cases
    std::vector<std::string> tests = {
        "ADD 10 20",
        "MUL 2000000000 3",
        "REVERSE Hello, World!",
        "AVG 5 1 2 3 4 5",
        "SORT 5 5 1 4 3 2"
    };
    std::cout << "Test cases:\n";
    for (const auto& t : tests) {
        std::cout << t << " => " << Safe::process_line(t) << "\n";
    }

    std::cout << "\nEnter commands (CTRL+D to end):\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.size() > Safe::MAX_LINE_LEN) {
            std::cout << "ERROR: line too long (max 1024)\n";
            continue;
        }
        std::cout << Safe::process_line(line) << "\n";
    }
    return 0;
}