#include <bits/stdc++.h>
using namespace std;

static const long long LL_MIN = LLONG_MIN;
static const long long LL_MAX = LLONG_MAX;

static inline string make_result(long long v) {
    return string("RESULT: ") + to_string(v);
}

static inline string make_error(const string& msg) {
    return string("ERROR: ") + msg;
}

static bool parse_int64(const string& s, long long& out) {
    string t = s;
    // Trim
    auto l = t.find_first_not_of(" \t\r\n");
    auto r = t.find_last_not_of(" \t\r\n");
    if (l == string::npos) return false;
    t = t.substr(l, r - l + 1);
    // Validate token format
    if (t.empty()) return false;
    size_t i = 0;
    if (t[0] == '+' || t[0] == '-') {
        if (t.size() == 1) return false;
        i = 1;
    }
    for (; i < t.size(); ++i) {
        if (!isdigit(static_cast<unsigned char>(t[i]))) return false;
    }
    errno = 0;
    char* endp = nullptr;
    long long val = strtoll(t.c_str(), &endp, 10);
    if (errno != 0 || endp == t.c_str() || *endp != '\0') return false;
    out = val;
    return true;
}

static bool add_checked(long long a, long long b, long long& res) {
    if ((b > 0 && a > LL_MAX - b) || (b < 0 && a < LL_MIN - b)) return false;
    res = a + b;
    return true;
}

static bool sub_checked(long long a, long long b, long long& res) {
    if ((b < 0 && a > LL_MAX + b) || (b > 0 && a < LL_MIN + b)) return false;
    res = a - b;
    return true;
}

static bool mul_checked(long long a, long long b, long long& res) {
    if (a == 0 || b == 0) { res = 0; return true; }
    if (a == -1 && b == LL_MIN) return false;
    if (b == -1 && a == LL_MIN) return false;
    // Check via division thresholds
    if (a > 0) {
        if (b > 0) { if (a > LL_MAX / b) return false; }
        else { if (b < LL_MIN / a) return false; }
    } else {
        if (b > 0) { if (a < LL_MIN / b) return false; }
        else {
            if (a != 0 && b < LL_MAX / a) return false;
        }
    }
    res = a * b;
    return true;
}

static bool div_checked(long long a, long long b, long long& res) {
    if (b == 0) return false;
    if (a == LL_MIN && b == -1) return false;
    res = a / b;
    return true;
}

static bool mod_checked(long long a, long long b, long long& res) {
    if (b == 0) return false;
    res = a % b;
    return true;
}

static bool pow_checked(long long base, long long exp, long long& res) {
    if (exp < 0) return false;
    if (base == 0 && exp == 0) return false;
    long long result = 1;
    long long b = base;
    long long e = exp;
    while (e > 0) {
        if (e & 1LL) {
            if (!mul_checked(result, b, result)) return false;
        }
        e >>= 1LL;
        if (e > 0) {
            if (!mul_checked(b, b, b)) return false;
        }
    }
    res = result;
    return true;
}

string processInput(const string& input) {
    // Trim
    string s = input;
    auto l = s.find_first_not_of(" \t\r\n");
    if (l == string::npos) return make_error("Empty input");
    auto r = s.find_last_not_of(" \t\r\n");
    s = s.substr(l, r - l + 1);

    // Tokenize
    istringstream iss(s);
    string op, aStr, bStr, extra;
    if (!(iss >> op >> aStr >> bStr) || (iss >> extra)) {
        return make_error("Expected format: OP A B");
    }
    for (auto& c : op) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));

    long long a, b;
    if (!parse_int64(aStr, a) || !parse_int64(bStr, b)) {
        return make_error("Invalid number");
    }

    long long res;
    if (op == "ADD") {
        if (!add_checked(a, b, res)) return make_error("Overflow");
        return make_result(res);
    } else if (op == "SUB") {
        if (!sub_checked(a, b, res)) return make_error("Overflow");
        return make_result(res);
    } else if (op == "MUL") {
        if (!mul_checked(a, b, res)) return make_error("Overflow");
        return make_result(res);
    } else if (op == "DIV") {
        if (!div_checked(a, b, res)) {
            if (b == 0) return make_error("Division by zero");
            return make_error("Overflow");
        }
        return make_result(res);
    } else if (op == "MOD") {
        if (!mod_checked(a, b, res)) {
            return make_error("Modulus by zero");
        }
        return make_result(res);
    } else if (op == "POW") {
        if (!pow_checked(a, b, res)) {
            if (b < 0) return make_error("Negative exponent");
            if (a == 0 && b == 0) return make_error("Undefined 0^0");
            return make_error("Overflow");
        }
        return make_result(res);
    } else {
        return make_error("Unknown operation");
    }
}

int main() {
    vector<string> tests = {
        "ADD 3 5",
        "DIV 10 0",
        "POW 2 10",
        "MUL 9223372036854775807 2",
        "SUB a 5"
    };
    for (const auto& t : tests) {
        cout << "Input: " << t << " -> " << processInput(t) << "\n";
    }
    return 0;
}