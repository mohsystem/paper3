
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
    // Test case 1: Example from problem
    Task176 test1;
    test1.add(3);
    test1.add(0);
    test1.add(2);
    test1.add(5);
    test1.add(4);
    std::cout << test1.getProduct(2) << std::endl; // Expected: 20
    std::cout << test1.getProduct(3) << std::endl; // Expected: 40
    std::cout << test1.getProduct(4) << std::endl; // Expected: 0
    test1.add(8);
    std::cout << test1.getProduct(2) << std::endl; // Expected: 32
    
    // Test case 2: All non-zero
    Task176 test2;
    test2.add(2);
    test2.add(3);
    test2.add(4);
    std::cout << test2.getProduct(2) << std::endl; // Expected: 12
    std::cout << test2.getProduct(3) << std::endl; // Expected: 24
    
    // Test case 3: Multiple zeros
    Task176 test3;
    test3.add(5);
    test3.add(0);
    test3.add(0);
    test3.add(6);
    std::cout << test3.getProduct(1) << std::endl; // Expected: 6
    
    // Test case 4: Single element
    Task176 test4;
    test4.add(7);
    std::cout << test4.getProduct(1) << std::endl; // Expected: 7
    
    // Test case 5: Zero then non-zero
    Task176 test5;
    test5.add(1);
    test5.add(2);
    test5.add(0);
    test5.add(3);
    test5.add(4);
    std::cout << test5.getProduct(2) << std::endl; // Expected: 12
    std::cout << test5.getProduct(3) << std::endl; // Expected: 0
    
    return 0;
}
