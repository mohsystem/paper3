
#include <iostream>
#include <vector>
#include <string>

std::string oddOrEven(std::vector<int> array) {
    if (array.empty()) {
        array.push_back(0);
    }
    
    long long sum = 0;
    for (size_t i = 0; i < array.size(); i++) {
        sum += array[i];
    }
    
    return (sum % 2 == 0) ? "even" : "odd";
}

int main() {
    std::cout << oddOrEven({0}) << std::endl;
    std::cout << oddOrEven({0, 1, 4}) << std::endl;
    std::cout << oddOrEven({0, -1, -5}) << std::endl;
    std::cout << oddOrEven({}) << std::endl;
    std::cout << oddOrEven({1, 2, 3, 4, 5}) << std::endl;
    
    return 0;
}
