// Chain-of-Through process:
// 1) Problem understanding: Parse "OP A B", execute 64-bit safe arithmetic, return "Result: <value>" or "Error: <reason>".
// 2) Security requirements: strict input validation, whitelist operations, overflow checks, division guards.
// 3) Secure coding generation: Helper functions for safe add/sub/mul/div without UB, validate tokens.
// 4) Code review: Checked for integer overflows, input bounds, error handling robustness.
// 5) Secure code output: Final, safe C++ implementation below.

#include <bits/stdc++.h>
using namespace std;

static bool safeAdd(long long a, long long b, long long &out) {
    if (b > 0 && a > LLONG_MAX - b) return false;
    if (b < 0 && a < LLONG_MIN - b) return false;
    out = a + b;
    return true;
}

static bool safeSub(long long a, long long b, long long &out) {
    if (b > 0 && a < LLONG_MIN + b) return false;
    if (b < 0 && a > LLONG_MAX + b) return false;
    out = a - b;
    return true;
}

static bool safeMul(long long a, long long b, long long &out) {
    if (a == 0 || b == 0) { out = 0; return true; }
    if (a == -1 && b == LLONG_MIN) return false;
    if (b == -1 && a == LLONG_MIN) return false;
    if (a > 0) {
        if (b > 0) { if (a > LLONG_MAX / b) return false; }
        else { if (b < LLONG_MIN / a) return false; }
    } else { // a < 0
        if (b > 0) { if (a < LLONG_MIN / b) return false; }
        else { if (a != 0 && b < LLONG_MAX / a) return false; }
    }
    out = a * b;
    return true;
}

static bool safeDiv(long long a, long long b, long long &out) {
    if (b == 0) return false;
    if (a == LLONG_MIN && b == -1) return false;
    out = a / b;
    return true;
}

static bool parseInt64(const string &s, long long &val) {
    if (s.empty()) return false;
    // Validate characters
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (!(c == '+' || c == '-' || (c >= '0' && c <= '9'))) return false;
    }
    try {
        size_t idx = 0;
        long long v = stoll(s, &idx, 10);
        if (idx != s.size()) return false;
        val = v;
        return true;
    } catch (...) {
        return false;
    }
}

string processInput(const string &input) {
    if (input.size() == 0) return "Error: input is empty";
    if (input.size() > 1000) return "Error: input too long";

    // Tokenize
    istringstream iss(input);
    string op, aStr, bStr, extra;
    if (!(iss >> op >> aStr >> bStr) || (iss >> extra)) {
        return "Error: expected format 'OP A B' with exactly three tokens";
    }

    for (auto &ch : op) ch = (char)toupper((unsigned char)ch);

    long long a, b;
    if (!parseInt64(aStr, a)) return "Error: invalid number for A";
    if (!parseInt64(bStr, b)) return "Error: invalid number for B";

    long long res;
    if (op == "ADD") {
        if (!safeAdd(a, b, res)) return "Error: overflow in addition";
        return string("Result: ") + to_string(res);
    } else if (op == "SUB") {
        if (!safeSub(a, b, res)) return "Error: overflow in subtraction";
        return string("Result: ") + to_string(res);
    } else if (op == "MUL") {
        if (!safeMul(a, b, res)) return "Error: overflow in multiplication";
        return string("Result: ") + to_string(res);
    } else if (op == "DIV") {
        if (!safeDiv(a, b, res)) return "Error: division error (division by zero or overflow)";
        return string("Result: ") + to_string(res);
    } else {
        return "Error: unsupported operation";
    }
}

int main() {
    vector<string> tests = {
        "ADD 5 7",
        "DIV 10 0",
        "MUL 9223372036854775807 2",
        "SUB -5 -10",
        "FOO 1 2"
    };
    for (const auto &t : tests) {
        cout << "Input: \"" << t << "\" -> " << processInput(t) << "\n";
    }
    return 0;
}