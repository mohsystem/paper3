#include <bits/stdc++.h>
using namespace std;

static const size_t MAX_PAYLOAD_LEN = 4096;

static unordered_map<string, long long> PRICE_MAP = {
    {"A100", 1999},   // $19.99
    {"B200", 599},    // $5.99
    {"C300", 2500},   // $25.00
    {"D400", 12345},  // $123.45
    {"E500", 1}       // $0.01
};

long long get_price_for_product_code(const string& product_code, bool& found) {
    found = false;
    string code;
    code.reserve(product_code.size());
    for (char c : product_code) code.push_back((char)toupper((unsigned char)c));
    static const regex re("^[A-Z0-9_-]{1,32}$");
    if (!regex_match(code, re)) {
        return 0;
    }
    auto it = PRICE_MAP.find(code);
    if (it == PRICE_MAP.end()) {
        return 0;
    }
    found = true;
    return it->second;
}

static string strip_quotes(const string& v) {
    if (v.size() >= 2 && ((v.front()=='"' && v.back()=='"') || (v.front=='\'' && v.back()=='\''))) {
        return v.substr(1, v.size()-2);
    }
    return v;
}

static map<string, string> parse_simple_yaml(const string& s) {
    map<string, string> out;
    string line;
    stringstream ss(s);
    while (getline(ss, line)) {
        string t = line;
        // trim
        auto l = t.find_first_not_of(" \t\r\n");
        if (l == string::npos) continue;
        auto r = t.find_last_not_of(" \t\r\n");
        t = t.substr(l, r - l + 1);
        if (t.empty() || t[0] == '#') continue;
        auto pos = t.find(':');
        if (pos == string::npos || pos == 0) continue;
        string key = t.substr(0, pos);
        string val = t.substr(pos + 1);
        // trim key and val
        auto kl = key.find_first_not_of(" \t");
        auto kr = key.find_last_not_of(" \t");
        key = (kl == string::npos) ? "" : key.substr(kl, kr - kl + 1);
        auto vl = val.find_first_not_of(" \t");
        auto vr = val.find_last_not_of(" \t");
        val = (vl == string::npos) ? "" : val.substr(vl, vr - vl + 1);
        // lowercase key
        for (auto& c : key) c = (char)tolower((unsigned char)c);
        val = strip_quotes(val);
        out[key] = val;
    }
    return out;
}

static string escape_yaml(const string& s) {
    string r;
    r.reserve(s.size() * 2);
    for (char c : s) {
        if (c == '\\') r += "\\\\";
        else if (c == '"') r += "\\\"";
        else if (c == '\n') r += "\\n";
        else if (c == '\r') r += "\\r";
        else r += c;
    }
    return r;
}

static bool safe_parse_int64(const string& s, long long& out) {
    if (s.empty()) return false;
    errno = 0;
    char* endp = nullptr;
    long long val = strtoll(s.c_str(), &endp, 10);
    if (errno != 0) return false;
    if (endp == s.c_str() || *endp != '\0') return false;
    out = val;
    return true;
}

static string format_cents(long long cents) {
    long long dollars = cents / 100;
    long long rem = llabs(cents % 100);
    ostringstream oss;
    oss << dollars << "." << setw(2) << setfill('0') << rem;
    return oss.str();
}

string calculate_price(const string& yaml_payload) {
    try {
        if (yaml_payload.empty()) {
            return "error: \"missing payload\"\n";
        }
        if (yaml_payload.size() > MAX_PAYLOAD_LEN) {
            return "error: \"payload too large\"\n";
        }
        auto data = parse_simple_yaml(yaml_payload);
        auto itCode = data.find("product_code");
        auto itQty = data.find("quantity");
        if (itCode == data.end() || itCode->second.empty()) {
            return "error: \"missing product_code\"\n";
        }
        string code = itCode->second;
        for (auto& c : code) c = (char)toupper((unsigned char)c);
        static const regex re("^[A-Z0-9_-]{1,32}$");
        if (!regex_match(code, re)) {
            return "error: \"invalid product_code format\"\n";
        }
        bool found = false;
        long long price_cents = get_price_for_product_code(code, found);
        if (!found) {
            return "error: \"unknown product_code\"\n";
        }
        if (itQty == data.end() || itQty->second.empty()) {
            return "error: \"missing quantity\"\n";
        }
        string qtxt = itQty->second;
        if (!qtxt.empty() && qtxt[0] == '+') qtxt.erase(0, 1);
        long long qty = 0;
        if (!safe_parse_int64(qtxt, qty)) {
            return "error: \"quantity must be an integer\"\n";
        }
        if (qty <= 0) {
            return "error: \"quantity must be positive\"\n";
        }
        if (qty > 1000000LL) {
            return "error: \"quantity too large\"\n";
        }
        long long total = price_cents * qty; // fits easily under current constraints
        ostringstream out;
        out << "total_price: \"" << format_cents(total) << "\"\n";
        return out.str();
    } catch (const exception& e) {
        return string("error: \"") + escape_yaml(e.what()) + "\"\n";
    } catch (...) {
        return "error: \"internal error\"\n";
    }
}

int main() {
    vector<string> tests = {
        "product_code: A100\nquantity: 2",
        "product_code: \"B200\"\nquantity: 3",
        "product_code: Z999\nquantity: 1",
        "product_code: C300\nquantity: -5",
        "product_code: D400\nquantity: two"
    };
    for (auto& t : tests) {
        cout << calculate_price(t);
    }
    return 0;
}