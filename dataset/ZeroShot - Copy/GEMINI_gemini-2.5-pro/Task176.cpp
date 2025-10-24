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
    // Test Case 1
    ProductOfNumbers pon1;
    pon1.add(3);
    pon1.add(0);
    pon1.add(2);
    pon1.add(5);
    pon1.add(4);
    std::cout << pon1.getProduct(2) << std::endl; // 20
    std::cout << pon1.getProduct(3) << std::endl; // 40
    std::cout << pon1.getProduct(4) << std::endl; // 0
    pon1.add(8);
    std::cout << pon1.getProduct(2) << std::endl; // 32
    std::cout << std::endl;

    // Test Case 2
    ProductOfNumbers pon2;
    pon2.add(1);
    pon2.add(2);
    pon2.add(3);
    pon2.add(4);
    pon2.add(5);
    std::cout << pon2.getProduct(1) << std::endl; // 5
    std::cout << pon2.getProduct(2) << std::endl; // 20
    std::cout << pon2.getProduct(5) << std::endl; // 120
    std::cout << std::endl;
    
    // Test Case 3
    ProductOfNumbers pon3;
    pon3.add(0);
    pon3.add(10);
    pon3.add(20);
    std::cout << pon3.getProduct(1) << std::endl; // 20
    std::cout << pon3.getProduct(2) << std::endl; // 200
    std::cout << pon3.getProduct(3) << std::endl; // 0
    std::cout << std::endl;

    // Test Case 4
    ProductOfNumbers pon4;
    pon4.add(5);
    pon4.add(6);
    pon4.add(0);
    pon4.add(7);
    pon4.add(0);
    pon4.add(8);
    pon4.add(9);
    std::cout << pon4.getProduct(1) << std::endl; // 9
    std::cout << pon4.getProduct(2) << std::endl; // 72
    std::cout << pon4.getProduct(3) << std::endl; // 0
    std::cout << std::endl;
    
    // Test Case 5
    ProductOfNumbers pon5;
    pon5.add(42);
    std::cout << pon5.getProduct(1) << std::endl; // 42
    pon5.add(0);
    std::cout << pon5.getProduct(1) << std::endl; // 0
    pon5.add(1);
    std::cout << pon5.getProduct(1) << std::endl; // 1
    std::cout << pon5.getProduct(2) << std::endl; // 0
    std::cout << std::endl;
    
    return 0;
}