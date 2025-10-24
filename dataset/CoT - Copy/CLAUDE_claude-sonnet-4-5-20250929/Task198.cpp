
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class Task198 {
public:
    // Simulated rand7() API
    static int rand7() {
        return rand() % 7 + 1;
    }
    
    // rand10() implementation using rejection sampling
    static int rand10() {
        int num;
        while (true) {
            // Generate number in range [0, 48]
            num = (rand7() - 1) * 7 + (rand7() - 1);
            
            // Only accept numbers in range [0, 39] for uniform distribution
            if (num < 40) {
                return (num % 10) + 1;
            }
            // Reject and retry if num >= 40
        }
    }
    
    // Test function
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
    std::vector<int> test1 = Task198::testRand10(1);
    std::cout << "Test 1: [";
    for (size_t i = 0; i < test1.size(); i++) {
        std::cout << test1[i] << (i < test1.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 2
    std::vector<int> test2 = Task198::testRand10(2);
    std::cout << "Test 2: [";
    for (size_t i = 0; i < test2.size(); i++) {
        std::cout << test2[i] << (i < test2.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 3
    std::vector<int> test3 = Task198::testRand10(3);
    std::cout << "Test 3: [";
    for (size_t i = 0; i < test3.size(); i++) {
        std::cout << test3[i] << (i < test3.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 4
    std::vector<int> test4 = Task198::testRand10(5);
    std::cout << "Test 4: [";
    for (size_t i = 0; i < test4.size(); i++) {
        std::cout << test4[i] << (i < test4.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    // Test case 5
    std::vector<int> test5 = Task198::testRand10(10);
    std::cout << "Test 5: [";
    for (size_t i = 0; i < test5.size(); i++) {
        std::cout << test5[i] << (i < test5.size() - 1 ? "," : "");
    }
    std::cout << "]" << std::endl;
    
    return 0;
}
