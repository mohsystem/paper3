
#include <iostream>
#include <stdexcept>

class Lse16 {
private:
    static const int arraySize = 10;
    static const int array[arraySize];
    
public:
    static int getValueAtIndex(int index) {
        if (index < 0 || index >= arraySize) {
            throw std::out_of_range("Index out of bounds");
        }
        return array[index];
    }
};

const int Lse16::array[Lse16::arraySize] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    // Test case 1
    std::cout << "Index 0: " << Lse16::getValueAtIndex(0) << std::endl;
    
    // Test case 2
    std::cout << "Index 5: " << Lse16::getValueAtIndex(5) << std::endl;
    
    // Test case 3
    std::cout << "Index 9: " << Lse16::getValueAtIndex(9) << std::endl;
    
    // Test case 4
    std::cout << "Index 3: " << Lse16::getValueAtIndex(3) << std::endl;
    
    // Test case 5
    std::cout << "Index 7: " << Lse16::getValueAtIndex(7) << std::endl;
    
    return 0;
}
