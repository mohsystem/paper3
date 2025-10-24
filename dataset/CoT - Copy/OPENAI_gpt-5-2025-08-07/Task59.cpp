// Chain-of-Through Step 1: Problem understanding
// accum(s): build "A-Bb-Ccc-..." segments.

// Chain-of-Through Step 2: Security requirements
// - No undefined behavior: cast to unsigned char before std::toupper/tolower.
// - No unchecked indexing beyond string bounds.

// Chain-of-Through Step 3: Secure coding generation
#include <bits/stdc++.h>
using namespace std;

string accum(const string& s) {
    // reserve estimated capacity: n + n*(n-1)/2 + (n? n-1:0)
    const size_t n = s.size();
    unsigned long long letters = n + (n * (unsigned long long)(n - 1)) / 2ULL;
    unsigned long long hyphens = (n > 0 ? n - 1ULL : 0ULL);
    unsigned long long total = letters + hyphens;
    string out;
    if (total <= out.max_size()) out.reserve(static_cast<size_t>(total));

    for (size_t i = 0; i < n; ++i) {
        if (i > 0) out.push_back('-');
        unsigned char uc = static_cast<unsigned char>(s[i]);
        out.push_back(static_cast<char>(std::toupper(uc)));
        char lower = static_cast<char>(std::tolower(uc));
        for (size_t j = 0; j < i; ++j) out.push_back(lower);
    }
    return out;
}

// Chain-of-Through Step 4: Code review
// - Loops are bounded by sizes; safe casts applied.

// Chain-of-Through Step 5: Secure code output - tests
int main() {
    vector<string> tests = {
        "abcd",
        "RqaEzty",
        "cwAt",
        "",
        "ZpglnRxqenU"
    };
    for (const auto& t : tests) {
        cout << "accum(\"" << t << "\") -> \"" << accum(t) << "\"\n";
    }
    return 0;
}