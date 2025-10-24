#include <bits/stdc++.h>
using namespace std;

class ProductOfNumbers {
private:
    vector<int> prefix; // prefix[0] = 1
public:
    ProductOfNumbers() {
        prefix.clear();
        prefix.push_back(1);
    }

    void add(int num) {
        if (num == 0) {
            prefix.clear();
            prefix.push_back(1);
        } else {
            long long prod = 1LL * prefix.back() * num;
            prefix.push_back(static_cast<int>(prod)); // safe per problem constraints
        }
    }

    int getProduct(int k) {
        int n = static_cast<int>(prefix.size());
        if (k >= n) return 0;
        return prefix[n - 1] / prefix[n - 1 - k];
    }
};

static void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    // Test 1: Example provided
    {
        ProductOfNumbers pon;
        pon.add(3);
        pon.add(0);
        pon.add(2);
        pon.add(5);
        pon.add(4);
        vector<int> out;
        out.push_back(pon.getProduct(2)); // 20
        out.push_back(pon.getProduct(3)); // 40
        out.push_back(pon.getProduct(4)); // 0
        pon.add(8);
        out.push_back(pon.getProduct(2)); // 32
        printVec(out);
    }

    // Test 2: No zeros
    {
        ProductOfNumbers pon;
        pon.add(1);
        pon.add(2);
        pon.add(3);
        pon.add(4);
        vector<int> out;
        out.push_back(pon.getProduct(1)); // 4
        out.push_back(pon.getProduct(3)); // 24
        out.push_back(pon.getProduct(4)); // 24
        printVec(out);
    }

    // Test 3: Zeros resetting
    {
        ProductOfNumbers pon;
        pon.add(0);
        pon.add(0);
        pon.add(7);
        pon.add(3);
        vector<int> out;
        out.push_back(pon.getProduct(1)); // 3
        out.push_back(pon.getProduct(2)); // 21
        printVec(out);
    }

    // Test 4: Long k equals length
    {
        ProductOfNumbers pon;
        pon.add(2);
        pon.add(2);
        pon.add(2);
        pon.add(2);
        pon.add(2);
        vector<int> out;
        out.push_back(pon.getProduct(5)); // 32
        printVec(out);
    }

    // Test 5: Mix with ones and zero
    {
        ProductOfNumbers pon;
        pon.add(1);
        pon.add(1);
        pon.add(1);
        pon.add(1);
        pon.add(0);
        pon.add(10);
        vector<int> out;
        out.push_back(pon.getProduct(1)); // 10
        out.push_back(pon.getProduct(2)); // 0
        pon.add(2);
        out.push_back(pon.getProduct(2)); // 20
        printVec(out);
    }

    return 0;
}