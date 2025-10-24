
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Task173 {
private:
    static const long long MOD = 1e9 + 7;
    static const long long BASE = 26;
    
    string search(const string& s, int len) {
        if (len <= 0 || len > s.length()) {
            return "";
        }
        
        long long hash = 0;
        long long pow = 1;
        
        for (int i = 0; i < len; i++) {
            hash = (hash * BASE + (s[i] - 'a')) % MOD;
            if (i < len - 1) {
                pow = (pow * BASE) % MOD;
            }
        }
        
        unordered_map<long long, vector<int>> seen;
        seen[hash].push_back(0);
        
        for (int i = len; i < s.length(); i++) {
            hash = ((hash - (s[i - len] - 'a') * pow % MOD + MOD) % MOD * BASE + (s[i] - 'a')) % MOD;
            
            if (seen.find(hash) != seen.end()) {
                string current = s.substr(i - len + 1, len);
                for (int idx : seen[hash]) {
                    if (s.substr(idx, len) == current) {
                        return current;
                    }
                }
            }
            seen[hash].push_back(i - len + 1);
        }
        
        return "";
    }
    
public:
    string longestDupSubstring(string s) {
        if (s.empty() || s.length() < 2) {
            return "";
        }
        
        int n = s.length();
        int left = 1, right = n;
        string result = "";
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            string dup = search(s, mid);
            if (!dup.empty()) {
                result = dup;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return result;
    }
};

int main() {
    Task173 solution;
    
    cout << "Test 1: " << solution.longestDupSubstring("banana") << endl;
    cout << "Test 2: " << solution.longestDupSubstring("abcd") << endl;
    cout << "Test 3: " << solution.longestDupSubstring("aa") << endl;
    cout << "Test 4: " << solution.longestDupSubstring("aaaaa") << endl;
    cout << "Test 5: " << solution.longestDupSubstring("abcabcabc") << endl;
    
    return 0;
}
