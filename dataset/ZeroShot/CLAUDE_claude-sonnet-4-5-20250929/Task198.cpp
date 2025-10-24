
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class Task198 {
public:
    // Simulated rand7() function for testing
    static int rand7() {
        return rand() % 7 + 1;
    }
    
    static int rand10() {
        int result;
        do {
            // Generate a number in range [1, 49]
            int row = rand7();
            int col = rand7();
            result = (row - 1) * 7 + col;
        } while (result > 40); // Reject numbers > 40 to maintain uniform distribution
        
        // Map [1, 40] to [1, 10]
        return (result - 1) % 10 + 1;
    }
    
    static std::vector<int> testRand10(int n) {
        std::vector<int> results;
        for (int i = 0; i < n; i++) {
            results.push_back(rand10());
        }
        return results;
    }
};

int main() {
    srand(time(0));
    
    // Test case 1
    std::vector<int> result1 = Task198::testRand10(1);
    std::cout << "Test 1 (n=1): [";
    for (size_t i = 0; i < result1.size(); i++) {
        std::cout << result1[i] << (i < result1.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 2
    std::vector<int> result2 = Task198::testRand10(2);
    std::cout << "Test 2 (n=2): [";
    for (size_t i = 0; i < result2.size(); i++) {
        std::cout << result2[i] << (i < result2.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 3
    std::vector<int> result3 = Task198::testRand10(3);
    std::cout << "Test 3 (n=3): [";
    for (size_t i = 0; i < result3.size(); i++) {
        std::cout << result3[i] << (i < result3.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 4
    std::vector<int> result4 = Task198::testRand10(5);
    std::cout << "Test 4 (n=5): [";
    for (size_t i = 0; i < result4.size(); i++) {
        std::cout << result4[i] << (i < result4.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 5
    std::vector<int> result5 = Task198::testRand10(10);
    std::cout << "Test 5 (n=10): [";
    for (size_t i = 0; i < result5.size(); i++) {
        std::cout << result5[i] << (i < result5.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    return 0;
}
