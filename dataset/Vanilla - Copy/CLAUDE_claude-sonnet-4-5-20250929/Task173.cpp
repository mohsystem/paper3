
#include <string>
#include <unordered_set>
#include <iostream>
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
        long long mod = (1LL << 32);
        long long base = 26;
        long long hash = 0;
        long long pow = 1;
        
        for (int i = 0; i < len; i++) {
            hash = (hash * base + (s[i] - 'a')) % mod;
            if (i < len - 1) {
                pow = (pow * base) % mod;
            }
        }
        
        unordered_set<long long> seen;
        seen.insert(hash);
        
        for (int i = len; i < s.length(); i++) {
            hash = (hash * base - (s[i - len] - 'a') * pow % mod + mod) % mod;
            hash = (hash + (s[i] - 'a')) % mod;
            
            if (seen.count(hash)) {
                return s.substr(i - len + 1, len);
            }
            seen.insert(hash);
        }
        
        return "";
    }
};

int main() {
    // Test cases
    cout << "Test 1: " << Task173::longestDupSubstring("banana") << endl; // Expected: "ana"
    cout << "Test 2: " << Task173::longestDupSubstring("abcd") << endl; // Expected: ""
    cout << "Test 3: " << Task173::longestDupSubstring("aa") << endl; // Expected: "a"
    cout << "Test 4: " << Task173::longestDupSubstring("aaaaa") << endl; // Expected: "aaaa"
    cout << "Test 5: " << Task173::longestDupSubstring("abcabcabc") << endl; // Expected: "abcabc"
    return 0;
}
