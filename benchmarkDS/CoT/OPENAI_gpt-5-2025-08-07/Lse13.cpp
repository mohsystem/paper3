#include <iostream>
#include <vector>
#include <stdexcept>

static const std::vector<int>& ID_SEQUENCE() {
    static const std::vector<int> seq = {101, 205, 309, 412, 518, 623, 734, 845, 956, 1067};
    return seq;
}

class Lse13 {
public:
    static int getValueAtIndex(int index) {
        if (index < 0 || index >= static_cast<int>(ID_SEQUENCE().size())) {
            throw std::out_of_range("Index out of range.");
        }
        return ID_SEQUENCE()[index];
    }
};

int main() {
    std::cout << "Enter an index to retrieve from id_sequence:" << std::endl;

    bool input_processed = false;
    std::streamsize avail = std::cin.rdbuf()->in_avail();
    if (avail > 0) {
        int idx;
        if (std::cin >> idx) {
            try {
                int val = Lse13::getValueAtIndex(idx);
                std::cout << "Value at index " << idx << " is: " << val << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "No valid integer input provided." << std::endl;
        }
        input_processed = true;
    }
    if (!input_processed) {
        std::cout << "No input detected. Skipping user query." << std::endl;
    }

    std::cout << "Running 5 test cases:" << std::endl;
    int tests[5] = {0, 3, 9, -1, 100};
    for (int t : tests) {
        try {
            int val = Lse13::getValueAtIndex(t);
            std::cout << "Index " << t << " => " << val << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Index " << t << " => Error: " << e.what() << std::endl;
        }
    }

    return 0;
}