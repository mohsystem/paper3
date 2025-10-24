#include <bits/stdc++.h>
using namespace std;

using u128 = unsigned __int128;

static bool is_all_digits(const string& s) {
    for (unsigned char c : s) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

static u128 recompute_product(const string& s, int start, int span) {
    u128 p = 1;
    for (int i = start; i < start + span; ++i) {
        p *= (unsigned)(s[i] - '0');
    }
    return p;
}

u128 largest_product(const string& input, int span) {
    if (span < 0) throw invalid_argument("Span cannot be negative.");
    if (input.size() == 0 && span == 0) return (u128)1;
    if ((int)input.size() < span) throw invalid_argument("Span cannot exceed input length.");
    if (!is_all_digits(input)) throw invalid_argument("Input must contain only digits.");
    if (span == 0) return (u128)1;

    int n = (int)input.size();
    vector<int> digits(n);
    for (int i = 0; i < n; ++i) digits[i] = input[i] - '0';

    int zeros = 0;
    u128 product = 1;
    bool productValid = true;

    for (int i = 0; i < span; ++i) {
        int d = digits[i];
        if (d == 0) { zeros++; productValid = false; }
        else product *= (unsigned)d;
    }

    u128 mx = (zeros == 0) ? product : (u128)0;

    for (int i = span; i < n; ++i) {
        int outgoing = digits[i - span];
        int incoming = digits[i];

        if (outgoing == 0) {
            zeros--;
            productValid = false;
        } else if (zeros == 0 && productValid) {
            product /= (unsigned)outgoing;
        }

        if (incoming == 0) {
            zeros++;
            productValid = false;
        } else if (zeros == 0 && productValid) {
            product *= (unsigned)incoming;
        }

        if (zeros == 0 && !productValid) {
            product = recompute_product(input, i - span + 1, span);
            productValid = true;
        }

        if (zeros == 0 && product > mx) mx = product;
    }

    return mx;
}

static string u128_to_string(u128 x) {
    if (x == 0) return "0";
    string s;
    while (x > 0) {
        unsigned digit = (unsigned)(x % 10);
        s.push_back(char('0' + digit));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    return s;
}

int main() {
    vector<pair<string,int>> tests = {
        {"63915", 3},
        {"123456789", 2},
        {"1020304050", 2},
        {"000", 2},
        {"", 0}
    };
    for (auto& [s, sp] : tests) {
        try {
            u128 res = largest_product(s, sp);
            cout << "Input: " << s << ", Span: " << sp << " -> " << u128_to_string(res) << "\n";
        } catch (const exception& e) {
            cout << "Input: " << s << ", Span: " << sp << " -> Error: " << e.what() << "\n";
        }
    }
    return 0;
}