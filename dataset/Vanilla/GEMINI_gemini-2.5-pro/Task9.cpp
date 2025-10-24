#include <iostream>
#include <vector>
#include <string>
#include <numeric> // For std::accumulate

class Task9 {
public:
    /**
     * @brief Given a vector of integers, determines if the sum of its elements is odd or even.
     * 
     * @param arr The input vector of integers. An empty vector is treated as {0}.
     * @return std::string "even" or "odd".
     */
    static std::string oddOrEven(const std::vector<int>& arr) {
        // std::accumulate with an initial value of 0 handles the empty vector case correctly.
        long long sum = std::accumulate(arr.begin(), arr.end(), 0LL);

        if (sum % 2 == 0) {
            return "even";
        } else {
            return "odd";
        }
    }
};

// Helper function to print a vector and run the test
void runTest(const std::vector<int>& arr) {
    std::cout << "Input: [";
    for(size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "], Output: " << Task9::oddOrEven(arr) << std::endl;
}

int main() {
    // Test Case 1
    runTest({0});

    // Test Case 2
    runTest({0, 1, 4});

    // Test Case 3
    runTest({0, -1, -5});

    // Test Case 4: Empty vector
    runTest({});

    // Test Case 5
    runTest({1, 2, 3, 4, 5});
    
    return 0;
}