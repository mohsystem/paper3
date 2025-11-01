#include <bits/stdc++.h>
using namespace std;

static const size_t MAX_PAYLOAD_SIZE = 10000;

static const unordered_map<string, long long> PRICE_MAP_CENTS = {
    {"P001", 999},
    {"P002", 1999},
    {"P003", 2500},
    {"P004", 12345},
    {"P005", 50}
};

static inline string trim(const string &s) {
    size_t start = 0, end = s.size();
    while (start < end && isspace(static_cast<unsigned char>(s[start]))) start++;
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static inline string trim_quotes(const string &s) {
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

static bool is_valid_product_code(const string &code) {
    if (code.empty() || code.size() > 32) return false;
    for (char c : code) {
        if (!(isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) return false;
    }
    return true;
}

string calculate_price(const string &yaml_payload) {
    if (yaml_payload.size() > MAX_PAYLOAD_SIZE) {
        return "error: payload too large";
    }

    string product_code;
    string quantity_str;

    size_t start = 0;
    while (start <= yaml_payload.size()) {
        size_t pos = yaml_payload.find('\n', start);
        string line = (pos == string::npos) ? yaml_payload.substr(start) : yaml_payload.substr(start, pos - start);
        start = (pos == string::npos) ? yaml_payload.size() + 1 : pos + 1;

        line = trim(line);
        if (line.empty() || (!line.empty() && line[0] == '#')) continue;
        size_t colon = line.find(':');
        if (colon == string::npos || colon == 0) continue;

        string key = trim(line.substr(0, colon));
        string value = trim(line.substr(colon + 1));
        value = trim_quotes(value);

        if (key == "product_code") product_code = value;
        else if (key == "quantity") quantity_str = value;
    }

    if (product_code.empty()) return "error: missing product_code";
    if (!is_valid_product_code(product_code)) return "error: invalid product_code";

    if (quantity_str.empty()) return "error: missing quantity";
    if (!all_of(quantity_str.begin(), quantity_str.end(), [](char c){ return isdigit(static_cast<unsigned char>(c)); })) {
        return "error: invalid quantity";
    }

    long long quantity = 0;
    try {
        quantity = stoll(quantity_str);
    } catch (...) {
        return "error: invalid quantity";
    }
    if (quantity <= 0 || quantity > 1000000) return "error: quantity out of range";

    auto it = PRICE_MAP_CENTS.find(product_code);
    if (it == PRICE_MAP_CENTS.end()) return "error: unknown product_code";

    long long total_cents = it->second * quantity;
    long long dollars = total_cents / 100;
    long long cents = total_cents % 100;

    ostringstream oss;
    oss << dollars << "." << setw(2) << setfill('0') << cents;
    return oss.str();
}

int main() {
    vector<string> tests = {
        "product_code: P001\nquantity: 3",
        "# Sample order\nproduct_code: \"P002\"\nquantity: 1",
        "product_code: P999\nquantity: 2",
        "product_code: P001\nquantity: -5",
        "product_code: P004\nquantity: 1000000"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << ": " << calculate_price(tests[i]) << "\n";
    }
    return 0;
}