#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>

using namespace std;

class Task173 {
private:
    long long modulus = (1LL << 61) - 1;
    long long base = 26;

    long long power(long long b, long long exp) {
        long long res = 1;
        b %= modulus;
        while (exp > 0) {
            if (exp % 2 == 1) res = (__int128)res * b % modulus;
            b = (__int128)b * b % modulus;
            exp /= 2;
        }
        return res;
    }

    int search(int L, int n, const string& s) {
        if (L == 0) return 0;

        long long h = power(base, L - 1);

        long long current_hash = 0;
        for (int i = 0; i < L; ++i) {
            current_hash = ((__int128)current_hash * base + (s[i] - 'a')) % modulus;
        }

        unordered_map<long long, vector<int>> seen;
        seen[current_hash].push_back(0);

        for (int i = 1; i <= n - L; ++i) {
            long long term_to_remove = ((__int128)(s[i - 1] - 'a') * h) % modulus;
            current_hash = (current_hash - term_to_remove + modulus) % modulus;
            current_hash = ((__int128)current_hash * base) % modulus;
            current_hash = (current_hash + (s[i + L - 1] - 'a')) % modulus;

            if (seen.count(current_hash)) {
                for (int start_idx : seen[current_hash]) {
                    if (s.compare(start_idx, L, s, i, L) == 0) {
                        return i;
                    }
                }
            }
            seen[current_hash].push_back(i);
        }
        return -1;
    }

public:
    string longestDupSubstring(string s) {
        int n = s.length();
        int low = 1, high = n;
        int start = -1;
        int len = 0;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (mid == 0) {
                low = mid + 1;
                continue;
            }
            int found_start = search(mid, n, s);
            if (found_start != -1) {
                len = mid;
                start = found_start;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return start != -1 ? s.substr(start, len) : "";
    }
};

int main() {
    Task173 solver;
    vector<string> testCases = {
        "banana",
        "abcd",
        "ababab",
        "aaaaa",
        "aabaaabaaaba"
    };
    for (const string& s : testCases) {
        cout << "Input: \"" << s << "\"" << endl;
        cout << "Output: \"" << solver.longestDupSubstring(s) << "\"" << endl;
        cout << endl;
    }
    return 0;
}