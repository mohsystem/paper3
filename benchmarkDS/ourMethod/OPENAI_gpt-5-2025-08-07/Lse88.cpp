#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <cctype>
#include <limits>

static const size_t MAX_INPUT_LEN = 4096;
static const size_t MAX_KEY_VAL_LEN = 64;

static std::unordered_map<std::string, long long> PRICE_CENTS = {
    {"P001", 999},   // 9.99
    {"P002", 550},   // 5.50
    {"P003", 2000},  // 20.00
};

static inline void rtrim_inplace(std::string &s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}
static inline void ltrim_inplace(std::string &s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    if (i > 0) s.erase(0, i);
}
static inline void trim_inplace(std::string &s) {
    rtrim_inplace(s);
    ltrim_inplace(s);
}
static inline std::string strip_quotes(const std::string &s) {
    if (s.size() >= 2 && ((s.front() == s.back()) && (s.front() == '"' || s.front() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}
static inline bool is_valid_product_code(const std::string &s) {
    if (s.empty() || s.size() > 32) return false;
    for (unsigned char c : s) {
        if (!(std::isalnum(c) || c == '_' || c == '-')) return false;
    }
    return true;
}
static inline bool is_valid_int_str(const std::string &s) {
    if (s.empty() || s.size() > 9) return false;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s.size() == 1) return false;
        i = 1;
    }
    for (; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}
static inline std::string format_cents(long long cents) {
    long long dollars = cents / 100;
    long long rem = cents % 100;
    if (rem < 0) rem = -rem;
    std::ostringstream oss;
    oss << dollars << "." << (rem < 10 ? "0" : "") << rem;
    return oss.str();
}

std::string calculate_price(const std::string &yaml_payload) {
    try {
        if (yaml_payload.empty() || yaml_payload.size() > MAX_INPUT_LEN) {
            return "error: invalid input";
        }
        if (yaml_payload.find('\0') != std::string::npos) {
            return "error: invalid input";
        }

        std::string product_code;
        std::string quantity_str;

        // Normalize line endings
        std::string data = yaml_payload;
        for (size_t pos = 0; (pos = data.find('\r', pos)) != std::string::npos; ) {
            data.erase(pos, 1);
        }

        std::istringstream iss(data);
        std::string raw;
        while (std::getline(iss, raw)) {
            std::string line = raw;
            trim_inplace(line);
            if (line.empty() || (!line.empty() && line[0] == '#')) continue;
            size_t colon = line.find(':');
            if (colon == std::string::npos) continue;

            std::string k = line.substr(0, colon);
            std::string v = line.substr(colon + 1);
            trim_inplace(k);
            trim_inplace(v);

            if (k.empty() || k.size() > MAX_KEY_VAL_LEN || v.size() > MAX_KEY_VAL_LEN) {
                return "error: invalid input";
            }

            v = strip_quotes(v);

            if (k == "product_code") {
                product_code = v;
            } else if (k == "quantity") {
                quantity_str = v;
            }
        }

        if (product_code.empty() || quantity_str.empty()) {
            return "error: missing product_code or quantity";
        }
        if (!is_valid_product_code(product_code)) {
            return "error: invalid product_code";
        }
        if (!is_valid_int_str(quantity_str)) {
            return "error: invalid quantity";
        }

        long long quantity = 0;
        try {
            quantity = std::stoll(quantity_str);
        } catch (...) {
            return "error: invalid quantity";
        }
        if (quantity <= 0 || quantity > 1000000) {
            return "error: invalid quantity";
        }

        auto it = PRICE_CENTS.find(product_code);
        if (it == PRICE_CENTS.end()) {
            return "error: unknown product_code";
        }
        long long price = it->second;

        if (quantity > std::numeric_limits<long long>::max() / price) {
            return "error: overflow";
        }
        long long total = price * quantity;
        return format_cents(total);
    } catch (...) {
        return "error: processing failed";
    }
}

int main() {
    std::string tests[5] = {
        "product_code: P001\nquantity: 3\n",
        "quantity: 10\nproduct_code: P002\n",
        "product_code: UNKNOWN\nquantity: 1\n",
        "product_code: P001\nquantity: -5\n",
        "product_code: P003\n"
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test " << (i + 1) << ": " << calculate_price(tests[i]) << std::endl;
    }
    return 0;
}