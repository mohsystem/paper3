#include <bits/stdc++.h>
using namespace std;

bool isValidWalk(const vector<char>& walk) {
    if (walk.size() != 10) return false;
    int x = 0, y = 0;
    for (char c : walk) {
        switch (c) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            default: return false;
        }
    }
    return x == 0 && y == 0;
}

int main() {
    vector<char> w1 = {'n','s','n','s','e','w','e','w','n','s'}; // true
    vector<char> w2 = {'n','s','e'}; // false
    vector<char> w3 = {'n','n','n','n','n','s','s','s','e','w'}; // false
    vector<char> w4 = {'e','w','e','w','n','s','n','s','n','s'}; // true
    vector<char> w5 = {'n','n','n','n','n','n','n','n','n','n'}; // false

    cout << (isValidWalk(w1) ? "true" : "false") << '\n';
    cout << (isValidWalk(w2) ? "true" : "false") << '\n';
    cout << (isValidWalk(w3) ? "true" : "false") << '\n';
    cout << (isValidWalk(w4) ? "true" : "false") << '\n';
    cout << (isValidWalk(w5) ? "true" : "false") << '\n';
    return 0;
}