
#include <vector>
#include <iostream>

class Task176 {
private:
    std::vector<int> prefixProducts;
    
public:
    Task176() {
        prefixProducts.push_back(1); // Initial prefix product
    }
    
    void add(int num) {
        if (num == 0) {
            // Reset the prefix products when 0 is encountered
            prefixProducts.clear();
            prefixProducts.push_back(1);
        } else {
            // Add the product of last prefix and current number
            int lastProduct = prefixProducts.back();
            prefixProducts.push_back(lastProduct * num);
        }
    }
    
    int getProduct(int k) {
        int n = prefixProducts.size();
        // If k is greater than or equal to the size, it means we crossed a zero
        if (k >= n) {
            return 0;
        }
        // Product of last k numbers = prefixProducts[n-1] / prefixProducts[n-1-k]
        return prefixProducts[n - 1] / prefixProducts[n - 1 - k];
    }
};

int main() {
    // Test case 1: Example from prompt
    Task176 test1;
    test1.add(3);
    test1.add(0);
    test1.add(2);
    test1.add(5);
    test1.add(4);
    std::cout << "Test 1 - getProduct(2): " << test1.getProduct(2) << std::endl; // Expected: 20
    std::cout << "Test 1 - getProduct(3): " << test1.getProduct(3) << std::endl; // Expected: 40
    std::cout << "Test 1 - getProduct(4): " << test1.getProduct(4) << std::endl; // Expected: 0
    test1.add(8);
    std::cout << "Test 1 - getProduct(2): " << test1.getProduct(2) << std::endl; // Expected: 32
    
    // Test case 2: No zeros
    Task176 test2;
    test2.add(2);
    test2.add(3);
    test2.add(4);
    std::cout << "\\nTest 2 - getProduct(2): " << test2.getProduct(2) << std::endl; // Expected: 12
    std::cout << "Test 2 - getProduct(3): " << test2.getProduct(3) << std::endl; // Expected: 24
    
    // Test case 3: Multiple zeros
    Task176 test3;
    test3.add(1);
    test3.add(0);
    test3.add(5);
    test3.add(0);
    test3.add(3);
    std::cout << "\\nTest 3 - getProduct(1): " << test3.getProduct(1) << std::endl; // Expected: 3
    
    // Test case 4: All ones
    Task176 test4;
    test4.add(1);
    test4.add(1);
    test4.add(1);
    std::cout << "\\nTest 4 - getProduct(3): " << test4.getProduct(3) << std::endl; // Expected: 1
    
    // Test case 5: Large numbers
    Task176 test5;
    test5.add(10);
    test5.add(10);
    test5.add(10);
    std::cout << "\\nTest 5 - getProduct(2): " << test5.getProduct(2) << std::endl; // Expected: 100
    std::cout << "Test 5 - getProduct(3): " << test5.getProduct(3) << std::endl; // Expected: 1000
    
    return 0;
}
