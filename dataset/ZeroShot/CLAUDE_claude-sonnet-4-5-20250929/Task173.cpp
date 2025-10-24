
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Task173 {
public:
    static string longestDupSubstring(string s) {
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
    
private:
    static string search(const string& s, int len) {
        long long MOD = (1LL << 31) - 1;
        long long base = 26;
        long long hash = 0;
        long long power = 1;
        
        unordered_map<long long, vector<int>> seen;
        
        for (int i = 0; i < len; i++) {
            hash = (hash * base + (s[i] - 'a')) % MOD;
            if (i < len - 1) {
                power = (power * base) % MOD;
            }
        }
        
        seen[hash].push_back(0);
        
        for (int i = len; i < s.length(); i++) {
            hash = ((hash - (s[i - len] - 'a') * power % MOD + MOD) % MOD * base 
                    + (s[i] - 'a')) % MOD;
            
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
};

int main() {
    cout << "Test 1: " << Task173::longestDupSubstring("banana") << endl;
    cout << "Test 2: " << Task173::longestDupSubstring("abcd") << endl;
    cout << "Test 3: " << Task173::longestDupSubstring("aa") << endl;
    cout << "Test 4: " << Task173::longestDupSubstring("aaaaa") << endl;
    cout << "Test 5: " << Task173::longestDupSubstring("abcabcabc") << endl;
    return 0;
}
