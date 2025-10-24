#include <iostream>
#include <string>
#include <cctype>
using namespace std;

string accum(const string& s) {
    string res;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i > 0) res.push_back('-');
        res.push_back(static_cast<char>(toupper(static_cast<unsigned char>(s[i]))));
        for (size_t j = 0; j < i; ++j) {
            res.push_back(static_cast<char>(tolower(static_cast<unsigned char>(s[i]))));
        }
    }
    return res;
}

int main() {
    string tests[] = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "ZpglnRxqenU",
        "NyffsGeyylB"
    };
    for (const auto& t : tests) {
        cout << accum(t) << endl;
    }
    return 0;
}