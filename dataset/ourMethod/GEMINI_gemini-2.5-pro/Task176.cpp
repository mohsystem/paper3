#include <iostream>
#include <vector>
#include <numeric>

class ProductOfNumbers {
private:
    std::vector<int> prefixProducts;

public:
    ProductOfNumbers() {
        prefixProducts.push_back(1);
    }

    void add(int num) {
        if (num == 0) {
            prefixProducts.clear();
            prefixProducts.push_back(1);
        } else {
            prefixProducts.push_back(prefixProducts.back() * num);
        }
    }

    int getProduct(int k) {
        int n = prefixProducts.size();
        if (k >= n) {
            return 0;
        }
        return prefixProducts.back() / prefixProducts[n - 1 - k];
    }
};

int main() {
    // Test Case 1 (from example)
    std::cout << "--- Test Case 1 ---" << std::endl;
    ProductOfNumbers pon1;
    pon1.add(3);
    pon1.add(0);
    pon1.add(2);
    pon1.add(5);
    pon1.add(4);
    std::cout << "getProduct(2): " << pon1.getProduct(2) << std::endl; // Expected: 20
    std::cout << "getProduct(3): " << pon1.getProduct(3) << std::endl; // Expected: 40
    std::cout << "getProduct(4): " << pon1.getProduct(4) << std::endl; // Expected: 0
    pon1.add(8);
    std::cout << "getProduct(2): " << pon1.getProduct(2) << std::endl; // Expected: 32

    // Test Case 2
    std::cout << "\n--- Test Case 2 ---" << std::endl;
    ProductOfNumbers pon2;
    pon2.add(5);
    pon2.add(10);
    pon2.add(2);
    std::cout << "getProduct(3): " << pon2.getProduct(3) << std::endl; // Expected: 100

    // Test Case 3
    std::cout << "\n--- Test Case 3 ---" << std::endl;
    ProductOfNumbers pon3;
    pon3.add(1);
    pon3.add(1);
    pon3.add(0);
    pon3.add(1);
    pon3.add(1);
    std::cout << "getProduct(2): " << pon3.getProduct(2) << std::endl; // Expected: 1
    std::cout << "getProduct(3): " << pon3.getProduct(3) << std::endl; // Expected: 0

    // Test Case 4
    std::cout << "\n--- Test Case 4 ---" << std::endl;
    ProductOfNumbers pon4;
    pon4.add(7);
    std::cout << "getProduct(1): " << pon4.getProduct(1) << std::endl; // Expected: 7

    // Test Case 5
    std::cout << "\n--- Test Case 5 ---" << std::endl;
    ProductOfNumbers pon5;
    pon5.add(4);
    pon5.add(0);
    pon5.add(9);
    pon5.add(8);
    pon5.add(0);
    pon5.add(2);
    std::cout << "getProduct(1): " << pon5.getProduct(1) << std::endl; // Expected: 2
    std::cout << "getProduct(2): " << pon5.getProduct(2) << std::endl; // Expected: 0

    return 0;
}