
#include <vector>
#include <iostream>

class Task176 {
private:
    std::vector<int> prefixProducts;
    
public:
    Task176() {
        prefixProducts.push_back(1);
    }
    
    void add(int num) {
        if (num == 0) {
            prefixProducts.clear();
            prefixProducts.push_back(1);
        } else {
            int lastProduct = prefixProducts.back();
            prefixProducts.push_back(lastProduct * num);
        }
    }
    
    int getProduct(int k) {
        int n = prefixProducts.size();
        if (k >= n) {
            return 0;
        }
        return prefixProducts[n - 1] / prefixProducts[n - 1 - k];
    }
};

int main() {
    // Test case 1: Basic example from problem
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
    
    // Test case 2: All non-zero numbers
    Task176 test2;
    test2.add(2);
    test2.add(3);
    test2.add(4);
    std::cout << "\\nTest 2 - getProduct(2): " << test2.getProduct(2) << std::endl; // Expected: 12
    std::cout << "Test 2 - getProduct(3): " << test2.getProduct(3) << std::endl; // Expected: 24
    
    // Test case 3: Single element
    Task176 test3;
    test3.add(5);
    std::cout << "\\nTest 3 - getProduct(1): " << test3.getProduct(1) << std::endl; // Expected: 5
    
    // Test case 4: Multiple zeros
    Task176 test4;
    test4.add(1);
    test4.add(0);
    test4.add(2);
    test4.add(0);
    test4.add(3);
    std::cout << "\\nTest 4 - getProduct(1): " << test4.getProduct(1) << std::endl; // Expected: 3
    std::cout << "Test 4 - getProduct(2): " << test4.getProduct(2) << std::endl; // Expected: 0
    
    // Test case 5: Large numbers
    Task176 test5;
    test5.add(10);
    test5.add(20);
    test5.add(30);
    std::cout << "\\nTest 5 - getProduct(2): " << test5.getProduct(2) << std::endl; // Expected: 600
    std::cout << "Test 5 - getProduct(3): " << test5.getProduct(3) << std::endl; // Expected: 6000
    
    return 0;
}
