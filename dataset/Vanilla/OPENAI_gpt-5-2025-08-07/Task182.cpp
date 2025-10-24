#include <bits/stdc++.h>
using namespace std;

string cleanNumber(const string& input) {
    string digits;
    digits.reserve(input.size());
    for (unsigned char ch : input) {
        if (isdigit(ch)) digits.push_back(static_cast<char>(ch));
    }
    if (digits.size() == 11 && digits[0] == '1') {
        digits.erase(digits.begin());
    }
    if (digits.size() != 10) return "";
    if (digits[0] < '2' || digits[0] > '9') return "";
    if (digits[3] < '2' || digits[3] > '9') return "";
    return digits;
}

int main() {
    vector<string> tests = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "212-055-1234"
    };
    for (const auto& t : tests) {
        cout << cleanNumber(t) << "\n";
    }
    return 0;
}