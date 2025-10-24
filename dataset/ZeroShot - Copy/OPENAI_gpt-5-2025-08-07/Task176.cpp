#include <iostream>
#include <vector>
#include <stdexcept>

class ProductOfNumbers {
public:
    ProductOfNumbers() {
        prefix.reserve(16);
        prefix.push_back(1LL); // sentinel
    }

    void add(int num) {
        if (num == 0) {
            prefix.clear();
            prefix.push_back(1LL);
            return;
        }
        long long next = prefix.back() * static_cast<long long>(num);
        prefix.push_back(next);
    }

    int getProduct(int k) const {
        if (k < 0) {
            throw std::invalid_argument("k must be non-negative");
        }
        if (static_cast<size_t>(k) >= prefix.size()) {
            return 0;
        }
        size_t sz = prefix.size();
        long long num = prefix[sz - 1];
        long long den = prefix[sz - 1 - static_cast<size_t>(k)];
        if (den == 0) {
            return 0; // defensive
        }
        long long res = num / den;
        return static_cast<int>(res);
    }

private:
    std::vector<long long> prefix;
};

int main() {
    // Test case 1: Example from prompt
    ProductOfNumbers p1;
    p1.add(3);
    p1.add(0);
    p1.add(2);
    p1.add(5);
    p1.add(4);
    std::cout << p1.getProduct(2) << "\n"; // 20
    std::cout << p1.getProduct(3) << "\n"; // 40
    std::cout << p1.getProduct(4) << "\n"; // 0
    p1.add(8);
    std::cout << p1.getProduct(2) << "\n"; // 32

    // Test case 2: No zeros
    ProductOfNumbers p2;
    p2.add(1);
    p2.add(2);
    p2.add(3);
    p2.add(4);
    std::cout << p2.getProduct(4) << "\n"; // 24

    // Test case 3: Multiple zeros with short queries
    ProductOfNumbers p3;
    p3.add(0);
    p3.add(7);
    p3.add(0);
    p3.add(10);
    std::cout << p3.getProduct(1) << "\n"; // 10
    std::cout << p3.getProduct(2) << "\n"; // 0

    // Test case 4: Larger k within segment
    ProductOfNumbers p4;
    p4.add(2);
    p4.add(2);
    p4.add(2);
    p4.add(2);
    p4.add(2);
    std::cout << p4.getProduct(5) << "\n"; // 32

    // Test case 5: Zero boundary checks
    ProductOfNumbers p5;
    p5.add(9);
    p5.add(9);
    p5.add(0);
    p5.add(9);
    p5.add(9);
    p5.add(9);
    std::cout << p5.getProduct(3) << "\n"; // 729
    std::cout << p5.getProduct(4) << "\n"; // 0

    return 0;
}