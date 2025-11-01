#include <iostream>
#include <vector>
#include <string>

int getElement(const std::vector<int>& arr, int index) {
    return arr[index];
}

int main(int argc, char* argv[]) {
    std::vector<int> arr{5, 10, 15, 20, 25, 30, 35, 40, 45, 50};

    if (argc > 1) {
        try {
            int idx = std::stoi(std::string(argv[1]));
            if (idx >= 0 && idx < static_cast<int>(arr.size())) {
                int val = getElement(arr, idx);
                std::cout << val << std::endl;
            } else {
                std::cout << "Index out of bounds" << std::endl;
            }
        } catch (...) {
            std::cout << "Invalid integer argument" << std::endl;
        }
        return 0;
    }

    int tests[] = {0, 1, 4, 7, 9};
    for (int idx : tests) {
        std::cout << getElement(arr, idx) << std::endl;
    }

    return 0;
}