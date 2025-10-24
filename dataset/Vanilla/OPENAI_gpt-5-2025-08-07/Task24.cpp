#include <bits/stdc++.h>
using namespace std;

bool validatePin(const string& pin) {
    if (pin.size() != 4 && pin.size() != 6) return false;
    for (char c : pin) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

int main() {
    vector<string> tests = {"1234", "12345", "a234", "0000", "098765"};
    for (const auto& t : tests) {
        cout << t << " -> " << (validatePin(t) ? "true" : "false") << "\n";
    }
    return 0;
}