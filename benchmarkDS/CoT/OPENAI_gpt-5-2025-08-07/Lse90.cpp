#include <bits/stdc++.h>
using namespace std;

// Returns price in cents or -1 if unknown
int get_price_for_product_code(const string& code) {
    static const unordered_map<string, int> prices = {
        {"A100", 1299}, // $12.99
        {"B200", 2599}, // $25.99
        {"C300",  499}, // $4.99
        {"D400", 9999}, // $99.99
        {"E500",  150}, // $1.50
    };
    auto it = prices.find(code);
    if (it == prices.end()) return -1;
    return it->second;
}

static inline string ltrim(const string& s) {
    size_t i = 0;
    while (i < s.size() && isspace(static_cast<unsigned char>(s[i]))) i++;
    return s.substr(i);
}
static inline string rtrim(const string& s) {
    if (s.empty()) return s;
    size_t i = s.size();
    while (i > 0 && isspace(static_cast<unsigned char>(s[i-1]))) i--;
    return s.substr(0, i);
}
static inline string trim(const string& s) { return rtrim(ltrim(s)); }

static inline string strip_quotes(const string& v) {
    if (v.size() >= 2) {
        if ((v.front()=='"' && v.back()=='"') || (v.front()=='\'' && v.back()=='\'')) {
            return v.substr(1, v.size()-2);
        }
    }
    return v;
}

static bool parse_simple_yaml(const string& in, unordered_map<string,string>& out) {
    out.clear();
    if (in.size() > 10000) return false;
    istringstream iss(in);
    string line;
    while (getline(iss, line)) {
        string t = trim(line);
        if (t.empty() || (!t.empty() && t[0]=='#')) continue;
        size_t pos = t.find(':');
        if (pos == string::npos || pos == 0) return false;
        string key = trim(t.substr(0, pos));
        string val = trim(t.substr(pos+1));
        val = strip_quotes(val);
        if (!out.count(key)) out[key] = val;
    }
    return true;
}

static string cents_to_dollars(long long cents) {
    bool neg = cents < 0;
    unsigned long long absval = static_cast<unsigned long long>(neg ? -cents : cents);
    unsigned long long dollars = absval / 100ULL;
    unsigned long long rem = absval % 100ULL;
    ostringstream oss;
    if (neg) oss << "-";
    oss << dollars << "." << setw(2) << setfill('0') << rem;
    return oss.str();
}

// Simulated web route: takes YAML query payload and returns YAML response
string calculate_price(const string& yamlPayload) {
    if (yamlPayload.empty()) {
        return "error: invalid_payload";
    }
    if (yamlPayload.size() > 10000) {
        return "error: payload_too_large";
    }

    unordered_map<string,string> data;
    if (!parse_simple_yaml(yamlPayload, data)) {
        return "error: malformed_yaml";
    }

    auto itCode = data.find("product_code");
    auto itQty  = data.find("quantity");
    if (itCode == data.end() || itCode->second.empty()) {
        return "error: missing_product_code";
    }
    if (itQty == data.end() || itQty->second.empty()) {
        return "error: missing_quantity";
    }

    long long qty = 0;
    try {
        qty = stoll(trim(itQty->second));
    } catch (...) {
        return "error: invalid_quantity";
    }
    if (qty <= 0 || qty > 1000000LL) {
        return "error: quantity_out_of_range";
    }

    int priceCents = get_price_for_product_code(trim(itCode->second));
    if (priceCents < 0) {
        return "error: unknown_product_code";
    }

    long long total;
    // check overflow: priceCents * qty within 64-bit
    if (qty != 0 && (LLONG_MAX / qty) < priceCents) {
        return "error: price_overflow";
    }
    total = static_cast<long long>(priceCents) * qty;

    return string("total_price: ") + cents_to_dollars(total);
}

int main() {
    vector<string> tests = {
        "product_code: A100\nquantity: 2",
        "product_code: B200\nquantity: 1",
        "product_code: C300\nquantity: 5",
        "product_code: Z999\nquantity: 1",
        "product_code: D400\nquantity: three"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i+1) << " input:\n" << tests[i] << "\n";
        cout << "Output:\n" << calculate_price(tests[i]) << "\n";
        cout << "---\n";
    }
    return 0;
}