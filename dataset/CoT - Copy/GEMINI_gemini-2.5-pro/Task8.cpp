#include <iostream>
#include <vector>

class Task8 {
public:
    static char findMissingLetter(const std::vector<char>& arr) {
        for (size_t i = 0; i < arr.size() - 1; ++i) {
            if (arr[i+1] != arr[i] + 1) {
                return static_cast<char>(arr[i] + 1);
            }
        }
        // This part should not be reachable given the problem constraints
        return ' ';
    }
};

void print_vector(const std::vector<char>& vec) {
    std::cout << "[";
    for(size_t i = 0; i < vec.size(); ++i) {
        std::cout << "'" << vec[i] << "'";
        if (i < vec.size() - 1) {
            std::cout << ",";
        }
    }
    std::cout << "]";
}

int main() {
    // Test Case 1
    std::vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
    std::cout << "Test Case 1: ";
    print_vector(test1);
    std::cout << " -> " << Task8::findMissingLetter(test1) << std::endl;

    // Test Case 2
    std::vector<char> test2 = {'O', 'Q', 'R', 'S'};
    std::cout << "Test Case 2: ";
    print_vector(test2);
    std::cout << " -> " << Task8::findMissingLetter(test2) << std::endl;

    // Test Case 3
    std::vector<char> test3 = {'x', 'z'};
    std::cout << "Test Case 3: ";
    print_vector(test3);
    std::cout << " -> " << Task8::findMissingLetter(test3) << std::endl;

    // Test Case 4
    std::vector<char> test4 = {'g', 'h', 'i', 'j', 'l'};
    std::cout << "Test Case 4: ";
    print_vector(test4);
    std::cout << " -> " << Task8::findMissingLetter(test4) << std::endl;

    // Test Case 5
    std::vector<char> test5 = {'A', 'B', 'C', 'E'};
    std::cout << "Test Case 5: ";
    print_vector(test5);
    std::cout << " -> " << Task8::findMissingLetter(test5) << std::endl;

    return 0;
}