#include <bits/stdc++.h>
using namespace std;

class Task198 {
private:
    static unsigned long long seed;

public:
    static void resetSeed(unsigned long long s) {
        seed = s;
    }

    static int nextRand() {
        seed = (seed * 1103515245ULL + 12345ULL) & 0x7fffffffULL;
        return static_cast<int>(seed);
    }

    // Given API
    static int rand7() {
        return (nextRand() % 7) + 1;
    }

    // Use only rand7()
    static int rand10() {
        while (true) {
            int a = rand7();
            int b = rand7();
            int num = (a - 1) * 7 + b; // 1..49
            if (num <= 40) return ((num - 1) % 10) + 1;
        }
    }

    static vector<int> generate(int n) {
        vector<int> out;
        out.reserve(n);
        for (int i = 0; i < n; ++i) out.push_back(rand10());
        return out;
    }

    static void printArray(const vector<int>& arr) {
        cout << "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i) cout << ",";
            cout << arr[i];
        }
        cout << "]\n";
    }
};

unsigned long long Task198::seed = 123456789ULL;

int main() {
    vector<int> tests = {1, 2, 3, 10, 20};
    for (int n : tests) {
        Task198::resetSeed(123456789ULL);
        auto res = Task198::generate(n);
        Task198::printArray(res);
    }
    return 0;
}