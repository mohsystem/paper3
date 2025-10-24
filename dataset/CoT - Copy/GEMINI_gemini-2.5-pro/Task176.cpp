#include <iostream>
#include <vector>

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
    // Test Case 1: Example from prompt
    std::cout << "Test Case 1:" << std::endl;
    ProductOfNumbers p1;
    p1.add(3);
    p1.add(0);
    p1.add(2);
    p1.add(5);
    p1.add(4);
    std::cout << "getProduct(2): " << p1.getProduct(2) << std::endl;
    std::cout << "getProduct(3): " << p1.getProduct(3) << std::endl;
    std::cout << "getProduct(4): " << p1.getProduct(4) << std::endl;
    p1.add(8);
    std::cout << "getProduct(2): " << p1.getProduct(2) << std::endl;
    std::cout << std::endl;

    // Test Case 2: Multiple zeros
    std::cout << "Test Case 2:" << std::endl;
    ProductOfNumbers p2;
    p2.add(1);
    p2.add(2);
    p2.add(0);
    p2.add(4);
    p2.add(5);
    std::cout << "getProduct(2): " << p2.getProduct(2) << std::endl;
    p2.add(0);
    p2.add(10);
    std::cout << "getProduct(1): " << p2.getProduct(1) << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: k equals the number of non-zero elements
    std::cout << "Test Case 3:" << std::endl;
    ProductOfNumbers p3;
    p3.add(5);
    p3.add(6);
    p3.add(7);
    std::cout << "getProduct(3): " << p3.getProduct(3) << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: getProduct(1)
    std::cout << "Test Case 4:" << std::endl;
    ProductOfNumbers p4;
    p4.add(100);
    p4.add(50);
    std::cout << "getProduct(1): " << p4.getProduct(1) << std::endl;
    std::cout << "getProduct(2): " << p4.getProduct(2) << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: k spans across a zero
    std::cout << "Test Case 5:" << std::endl;
    ProductOfNumbers p5;
    p5.add(1);
    p5.add(2);
    p5.add(3);
    p5.add(0);
    p5.add(4);
    p5.add(5);
    p5.add(6);
    std::cout << "getProduct(3): " << p5.getProduct(3) << std::endl;
    std::cout << "getProduct(4): " << p5.getProduct(4) << std::endl;
    std::cout << std::endl;

    return 0;
}