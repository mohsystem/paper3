#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>

static const std::unordered_map<std::string, double> PRICE_MAP = {
    {"A1", 9.99},
    {"B2", 5.00},
    {"C3", 2.50},
    {"D4", 100.00},
    {"E5", 0.75}
};

double get_price_for_product_code(const std::string& code) {
    auto it = PRICE_MAP.find(code);
    if (it == PRICE_MAP.end()) return -1.0;
    return it->second;
}

static bool is_ascii_printable(const std::string& s) {
    for (unsigned char ch : s) {
        if (ch == '\n' || ch == '\r' || ch == '\t') continue;
        if (!(ch >= 32 && ch <= 126)) return false;
    }
    return true;
}

static std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) a++;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) b--;
    return s.substr(a, b - a);
}

static std::string strip_quotes(const std::string& s) {
    std::string t = trim(s);
    if (t.size() >= 2 && ((t.front() == '\'' && t.back() == '\'') || (t.front() == '"' && t.back() == '"'))) {
        return t.substr(1, t.size() - 2);
    }
    return t;
}

static bool is_valid_product_code(const std::string& s) {
    if (s.empty() || s.size() > 20) return false;
    for (unsigned char c : s) {
        if (std::isalnum(c) || c == '_' || c == '-') continue;
        return false;
    }
    return true;
}

static bool parse_positive_int(const std::string& s, long long minv, long long maxv, long long& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    for (unsigned char c : t) {
        if (!std::isdigit(c)) return false;
    }
    try {
        long long v = std::stoll(t);
        if (v < minv || v > maxv) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

std::string calculate_price(const std::string& yamlPayload) {
    if (yamlPayload.size() == 0 || yamlPayload.size() > 4096) {
        return "error: invalid_request";
    }
    if (!is_ascii_printable(yamlPayload)) {
        return "error: invalid_request";
    }
    std::string product_code;
    std::string quantity_str;

    // Split lines
    std::istringstream iss(yamlPayload);
    std::string line;
    size_t lines_count = 0;
    while (std::getline(iss, line)) {
        lines_count++;
        if (lines_count > 200) {
            return "error: invalid_request";
        }
        std::string l = trim(line);
        if (l.empty() || (!l.empty() && l[0] == '#')) continue;
        auto pos = l.find(':');
        if (pos == std::string::npos || pos == 0) continue;
        std::string key = trim(l.substr(0, pos));
        std::string val = strip_quotes(l.substr(pos + 1));
        if (key == "product_code") product_code = val;
        else if (key == "quantity") quantity_str = val;
    }

    if (product_code.empty() || quantity_str.empty()) {
        return "error: invalid_request";
    }
    if (!is_valid_product_code(product_code)) {
        return "error: invalid_request";
    }
    long long qty = 0;
    if (!parse_positive_int(quantity_str, 1, 1000000, qty)) {
        return "error: invalid_request";
    }

    double price = get_price_for_product_code(product_code);
    if (price < 0.0) {
        return "error: unknown_product_code";
    }

    double total = price * static_cast<double>(qty);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << "total_price: " << total;
    return oss.str();
}

int main() {
    std::vector<std::string> tests = {
        "product_code: A1\nquantity: 3\n",
        "product_code: B2\nquantity: 10\n",
        "product_code: Z9\nquantity: 1\n",
        "product_code: C3\nquantity: -2\n",
        " product_code : 'E5' \n quantity : 4 \n"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " -> " << calculate_price(tests[i]) << "\n";
    }
    return 0;
}