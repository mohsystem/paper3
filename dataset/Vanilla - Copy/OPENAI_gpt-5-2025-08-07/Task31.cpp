#include <bits/stdc++.h>
using namespace std;

string dna_strand(const string& dna) {
    string out;
    out.reserve(dna.size());
    for (char c : dna) {
        switch (c) {
            case 'A': out.push_back('T'); break;
            case 'T': out.push_back('A'); break;
            case 'C': out.push_back('G'); break;
            case 'G': out.push_back('C'); break;
            default: out.push_back(c); // In case of unexpected characters
        }
    }
    return out;
}

int main() {
    vector<string> tests = {"ATTGC", "GTAT", "AAAA", "CGCGTTAA", "A"};
    for (const auto& s : tests) {
        cout << dna_strand(s) << '\n';
    }
    return 0;
}