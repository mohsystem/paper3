// Step 1: Problem understanding
// - Provide calculate_price function that parses a YAML-like payload string,
//   extracts product_code and quantity, calculates total price, and returns it as a string.
//
// Step 2: Security requirements
// - No external YAML parsers; implement minimal safe parsing.
// - Limit payload size, validate fields strictly, and avoid reflecting input in error messages.
// - Use integer cents to avoid floating-point precision issues.
//
// Step 3: Secure coding generation
// - Implement robust parsing, validation, and overflow checks.
//
// Step 4: Code review
// - Ensure no UB on string operations, check bounds, and sanitize inputs.
//
// Step 5: Secure code output
// - Return price as string with two decimals or "ERROR: ..." on failure.
// - Include 5 test cases in main.

#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <limits>

static const size_t MAX_PAYLOAD_LENGTH = 4096;

// Prices stored in cents to avoid floating point issues
static const std::unordered_map<std::string, long long> PRICE_MAP = {
    {"P001", 1999},
    {"P002", 550},
    {"P003", 9995},
    {"P100", 125},
    {"P900", 25000}
};

static inline std::string ltrim(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    return s.substr(i);
}
static inline std::string rtrim(const std::string& s) {
    if (s.empty()) return s;
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i-1]))) i--;
    return s.substr(0, i);
}
static inline std::string trim(const std::string& s) {
    return rtrim(ltrim(s));
}
static inline bool isQuoted(const std::string& s) {
    return s.size() >= 2 && ((s.front()=='"' && s.back()=='"') || (s.front()=='\'' && s.back()=='\''));
}
static inline std::string stripQuotes(const std::string& s) {
    if (isQuoted(s)) return s.substr(1, s.size()-2);
    return s;
}
static inline std::string toUpper(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (char c : s) out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    return out;
}
static inline bool isValidCode(const std::string& s) {
    if (s.empty() || s.size() > 20) return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='_' || c=='-')) return false;
    }
    return true;
}
static inline bool isDigits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

static std::unordered_map<std::string, std::string> parseSimpleYaml(const std::string& payload) {
    std::unordered_map<std::string, std::string> out;
    std::string normalized = payload;
    for (size_t i = 0; i + 1 < normalized.size(); ++i) {
        if (normalized[i] == '\r' && normalized[i+1] == '\n') {
            normalized[i] = '\n';
            normalized.erase(i+1, 1);
        }
    }
    for (size_t i = 0; i < normalized.size(); ++i) {
        if (normalized[i] == '\r') normalized[i] = '\n';
    }

    std::istringstream iss(normalized);
    std::string line;
    while (std::getline(iss, line)) {
        line = trim(line);
        if (line.empty() || (!line.empty() && line[0] == '#')) continue;
        size_t pos = line.find(':');
        if (pos == std::string::npos || pos == 0) continue;
        std::string key = trim(line.substr(0, pos));
        std::string val = trim(line.substr(pos + 1));
        val = stripQuotes(val);
        for (auto& ch : key) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        out[key] = val;
    }
    return out;
}

std::string calculate_price(const std::string& yamlPayload) {
    if (yamlPayload.empty()) return "ERROR: Missing payload";
    if (yamlPayload.size() > MAX_PAYLOAD_LENGTH) return "ERROR: Payload too large";

    auto map = parseSimpleYaml(yamlPayload);
    auto itCode = map.find("product_code");
    auto itQty = map.find("quantity");
    if (itCode == map.end() || itCode->second.empty()) return "ERROR: Missing product_code";
    if (itQty == map.end() || itQty->second.empty()) return "ERROR: Missing quantity";

    std::string code = toUpper(trim(itCode->second));
    if (!isValidCode(code)) return "ERROR: Invalid product_code format";

    std::string qtyStr = trim(itQty->second);
    qtyStr = stripQuotes(qtyStr);
    if (!isDigits(qtyStr)) return "ERROR: Invalid quantity";

    long long qty = 0;
    try {
        qty = std::stoll(qtyStr);
    } catch (...) {
        return "ERROR: Quantity out of range";
    }
    if (qty <= 0 || qty > 1000000LL) return "ERROR: Quantity out of allowed bounds";

    auto itPrice = PRICE_MAP.find(code);
    if (itPrice == PRICE_MAP.end()) return "ERROR: Unknown product_code";

    long long unit = itPrice->second; // cents
    // Check overflow: qty * unit
    if (qty > 0 && unit > 0 && qty > (std::numeric_limits<long long>::max() / unit)) {
        return "ERROR: Total exceeds limit";
    }
    long long totalCents = qty * unit;

    // Format cents to string with 2 decimals
    long long dollars = totalCents / 100;
    long long cents = std::llabs(totalCents % 100);
    std::ostringstream oss;
    oss << dollars << "." << (cents < 10 ? "0" : "") << cents;
    return oss.str();
}

int main() {
    const char* tests[] = {
        "product_code: P001\nquantity: 3",
        "product_code: P003\nquantity: 2",
        "product_code: 'P100'\nquantity: '10'",
        "product_code: X999\nquantity: 1",
        "product_code: P002\nquantity: five"
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << (i+1) << ": " << calculate_price(tests[i]) << "\n";
    }
    return 0;
}