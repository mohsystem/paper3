
#include <iostream>
#include <limits>

class Lse15 {
private:
    static const int id_sequence[10];
    static const int ARRAY_SIZE = 10;
    
public:
    static int getValueAtIndex(int index, bool& valid) {
        valid = false;
        if (index < 0 || index >= ARRAY_SIZE) {
            return -1;
        }
        valid = true;
        return id_sequence[index];
    }
    
    static int getValueWithPrompt(bool& valid) {
        std::cout << "Enter an index value: ";
        int index;
        if (!(std::cin >> index)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
            valid = false;
            return -1;
        }
        return getValueAtIndex(index, valid);
    }
};

const int Lse15::id_sequence[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    bool valid;
    
    // Test cases
    std::cout << "Test Case 1 - Index 0: " << Lse15::getValueAtIndex(0, valid) << " (Valid: " << valid << ")" << std::endl;
    std::cout << "Test Case 2 - Index 5: " << Lse15::getValueAtIndex(5, valid) << " (Valid: " << valid << ")" << std::endl;
    std::cout << "Test Case 3 - Index 9: " << Lse15::getValueAtIndex(9, valid) << " (Valid: " << valid << ")" << std::endl;
    std::cout << "Test Case 4 - Index -1 (invalid): " << Lse15::getValueAtIndex(-1, valid) << " (Valid: " << valid << ")" << std::endl;
    std::cout << "Test Case 5 - Index 10 (out of bounds): " << Lse15::getValueAtIndex(10, valid) << " (Valid: " << valid << ")" << std::endl;
    
    return 0;
}
