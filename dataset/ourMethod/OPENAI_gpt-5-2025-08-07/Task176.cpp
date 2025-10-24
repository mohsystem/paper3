#include <iostream>
#include <vector>
#include <stdexcept>

class ProductOfNumbers {
private:
    std::vector<int> pref; // pref[0] = 1; products since last zero
public:
    ProductOfNumbers() {
        pref.clear();
        pref.push_back(1);
    }

    void add(int num) {
        if (num < 0 || num > 100) {
            throw std::invalid_argument("num out of allowed range [0,100]");
        }
        if (num == 0) {
            pref.clear();
            pref.push_back(1);
        } else {
            long long next = static_cast<long long>(pref.back()) * static_cast<long long>(num);
            // per constraints, result fits 32-bit int
            pref.push_back(static_cast<int>(next));
        }
    }

    int getProduct(int k) const {
        if (k <= 0) throw std::invalid_argument("k must be positive");
        if (k >= static_cast<int>(pref.size())) return 0;
        int n = static_cast<int>(pref.size());
        int num = pref[n - 1];
        int den = pref[n - 1 - k];
        return num / den;
    }
};

static void header(const char* title) {
    std::cout << "---- " << title << " ----" << std::endl;
}

int main() {
    // Test Case 1: Example
    header("Test Case 1: Example");
    ProductOfNumbers p1;
    p1.add(3);
    p1.add(0);
    p1.add(2);
    p1.add(5);
    p1.add(4);
    std::cout << p1.getProduct(2) << std::endl; // 20
    std::cout << p1.getProduct(3) << std::endl; // 40
    std::cout << p1.getProduct(4) << std::endl; // 0
    p1.add(8);
    std::cout << p1.getProduct(2) << std::endl; // 32

    // Test Case 2: No zeros
    header("Test Case 2: No zeros");
    ProductOfNumbers p2;
    p2.add(1);
    p2.add(2);
    p2.add(3);
    p2.add(4);
    std::cout << p2.getProduct(1) << std::endl; // 4
    std::cout << p2.getProduct(2) << std::endl; // 12
    std::cout << p2.getProduct(4) << std::endl; // 24

    // Test Case 3: Multiple zeros and resets
    header("Test Case 3: Multiple zeros");
    ProductOfNumbers p3;
    p3.add(0);
    p3.add(9);
    p3.add(9);
    std::cout << p3.getProduct(2) << std::endl; // 81
    p3.add(0);
    std::cout << p3.getProduct(1) << std::endl; // 0
    p3.add(7);
    std::cout << p3.getProduct(2) << std::endl; // 0
    std::cout << p3.getProduct(1) << std::endl; // 7

    // Test Case 4: Crossing zero
    header("Test Case 4: Crossing zero");
    ProductOfNumbers p4;
    p4.add(2);
    p4.add(5);
    p4.add(0);
    p4.add(3);
    p4.add(4);
    std::cout << p4.getProduct(2) << std::endl; // 12
    std::cout << p4.getProduct(3) << std::endl; // 0
    std::cout << p4.getProduct(5) << std::endl; // 0

    // Test Case 5: Mixed segments
    header("Test Case 5: Mixed segments");
    ProductOfNumbers p5;
    p5.add(5);
    p5.add(2);
    p5.add(0);
    p5.add(1);
    p5.add(1);
    p5.add(1);
    p5.add(10);
    p5.add(0);
    p5.add(2);
    p5.add(2);
    p5.add(2);
    std::cout << p5.getProduct(1) << std::endl; // 2
    std::cout << p5.getProduct(2) << std::endl; // 4
    std::cout << p5.getProduct(3) << std::endl; // 8
    std::cout << p5.getProduct(4) << std::endl; // 0

    return 0;
}