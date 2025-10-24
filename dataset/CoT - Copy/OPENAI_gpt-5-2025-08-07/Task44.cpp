#include <bits/stdc++.h>
using namespace std;

static bool isAllowedChar(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) || c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '+' || c == '-';
}

static bool addWillOverflow(long long a, long long b) {
    if (b > 0 && a > LLONG_MAX - b) return true;
    if (b < 0 && a < LLONG_MIN - b) return true;
    return false;
}

string processInput(const string& input) {
    if (input.size() == 0) return "ERROR: empty input";
    if (input.size() > 1000) return "ERROR: input too long";
    for (char c : input) {
        if (!isAllowedChar(c)) {
            return "ERROR: invalid characters present";
        }
    }

    // Replace delimiters with spaces and split
    string norm = input;
    for (char& c : norm) {
        if (c == ',' || c == '\n' || c == '\t') c = ' ';
    }
    stringstream ss(norm);
    string tok;
    vector<string> tokens;
    while (ss >> tok) tokens.push_back(tok);
    if (tokens.empty()) return "ERROR: empty input";

    long long count = 0;
    long long mn = LLONG_MAX, mx = LLONG_MIN;
    long long sum = 0;

    for (const string& t : tokens) {
        if (++count > 100) return "ERROR: too many numbers (limit 100)";

        // Validate numeric token
        // Allow optional leading sign and digits 1..19 (range will be checked by stoll)
        if (!std::regex_match(t, std::regex("^[+-]?\\d{1,19}$"))) {
            // Attempt to parse anyway; if it fails, return error
            try {
                size_t idx = 0;
                (void)stoll(t, &idx, 10);
                if (idx != t.size()) return "ERROR: invalid number: " + t;
            } catch (...) {
                return "ERROR: invalid number: " + t;
            }
        }

        long long val;
        try {
            size_t idx = 0;
            val = stoll(t, &idx, 10);
            if (idx != t.size()) return "ERROR: invalid number: " + t;
        } catch (const out_of_range&) {
            return "ERROR: number out of range: " + t;
        } catch (...) {
            return "ERROR: invalid number: " + t;
        }

        if (val < mn) mn = val;
        if (val > mx) mx = val;

        if (addWillOverflow(sum, val)) return "ERROR: sum overflow";
        sum += val;
    }

    if (count == 0) return "ERROR: no numbers found";

    long double avg = static_cast<long double>(sum) / static_cast<long double>(count);
    ostringstream out;
    out.setf(std::ios::fixed);
    out << setprecision(10);
    out << "OK: count=" << count
        << ", sum=" << sum
        << ", min=" << mn
        << ", max=" << mx
        << ", average=";

    // Trim trailing zeros
    ostringstream tmp;
    tmp.setf(std::ios::fixed);
    tmp << setprecision(10) << avg;
    string avgStr = tmp.str();
    // remove trailing zeros and possibly trailing dot
    while (!avgStr.empty() && avgStr.back() == '0') avgStr.pop_back();
    if (!avgStr.empty() && avgStr.back() == '.') avgStr.pop_back();
    if (avgStr.empty()) avgStr = "0";
    out << avgStr;

    return out.str();
}

int main() {
    vector<string> tests = {
        "1, 2, 3, 4, 5",
        " 10 20 30 ",
        "-5, -10, 15",
        "abc",
        "9223372036854775807, 1"
    };
    for (const auto& t : tests) {
        cout << "Input: \"" << t << "\" -> " << processInput(t) << "\n";
    }
    return 0;
}