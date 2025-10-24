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
            // If a zero is added, reset the prefix products.
            prefixProducts.clear();
            prefixProducts.push_back(1);
        } else {
            // Append the new prefix product.
            prefixProducts.push_back(prefixProducts.back() * num);
        }
    }

    int getProduct(int k) {
        int n = prefixProducts.size();
        // If k is greater than or equal to the size of our list, a zero must be in the range.
        if (k >= n) {
            return 0;
        }
        // Calculate product using division of prefix products.
        return prefixProducts.back() / prefixProducts[n - 1 - k];
    }
};

class Task176 {
public:
    static void runTests() {
        // Test Case 1 (from example)
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

        // Test Case 2: No zeros
        std::cout << "Test Case 2:" << std::endl;
        ProductOfNumbers p2;
        p2.add(1);
        p2.add(2);
        p2.add(3);
        p2.add(4);
        std::cout << "getProduct(1): " << p2.getProduct(1) << std::endl;
        std::cout << "getProduct(4): " << p2.getProduct(4) << std::endl;
        std::cout << std::endl;

        // Test Case 3: Starts with zero
        std::cout << "Test Case 3:" << std::endl;
        ProductOfNumbers p3;
        p3.add(0);
        p3.add(5);
        p3.add(10);
        std::cout << "getProduct(1): " << p3.getProduct(1) << std::endl;
        std::cout << "getProduct(2): " << p3.getProduct(2) << std::endl;
        std::cout << "getProduct(3): " << p3.getProduct(3) << std::endl;
        std::cout << std::endl;

        // Test Case 4: Multiple zeros
        std::cout << "Test Case 4:" << std::endl;
        ProductOfNumbers p4;
        p4.add(5);
        p4.add(6);
        p4.add(0);
        p4.add(7);
        p4.add(0);
        p4.add(8);
        p4.add(9);
        std::cout << "getProduct(1): " << p4.getProduct(1) << std::endl;
        std::cout << "getProduct(2): " << p4.getProduct(2) << std::endl;
        std::cout << "getProduct(3): " << p4.getProduct(3) << std::endl;
        std::cout << std::endl;

        // Test Case 5: k equals the number of elements
        std::cout << "Test Case 5:" << std::endl;
        ProductOfNumbers p5;
        p5.add(10);
        p5.add(20);
        p5.add(30);
        std::cout << "getProduct(3): " << p5.getProduct(3) << std::endl;
        std::cout << std::endl;
    }
};

int main() {
    Task176::runTests();
    return 0;
}