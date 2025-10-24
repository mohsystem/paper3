#include <iostream>
#include <vector>
#include <string>

class Task13 {
public:
    static bool isValidWalk(const std::vector<char>& walk) {
        // 1. The walk must take exactly 10 minutes.
        if (walk.size() != 10) {
            return false;
        }

        // 2. The walk must return you to your starting point.
        int x = 0; // Represents East-West axis
        int y = 0; // Represents North-South axis

        for (char direction : walk) {
            switch (direction) {
                case 'n': y++; break;
                case 's': y--; break;
                case 'e': x++; break;
                case 'w': x--; break;
            }
        }

        return x == 0 && y == 0;
    }
};

void runTest(const std::string& testName, const std::vector<char>& walk) {
    std::cout << testName << ": {";
    for (size_t i = 0; i < walk.size(); ++i) {
        std::cout << "'" << walk[i] << "'" << (i == walk.size() - 1 ? "" : ", ");
    }
    std::cout << "}" << std::endl;
    std::cout << "Result: " << (Task13::isValidWalk(walk) ? "true" : "false") << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test Cases
    runTest("Test Case 1", {'n','s','n','s','n','s','n','s','n','s'});
    runTest("Test Case 2", {'w','e','w','e','w','e','w','e','w','e'});
    runTest("Test Case 3", {'w'});
    runTest("Test Case 4", {'n','n','n','s','n','s','n','s','n','s'});
    runTest("Test Case 5", {'e','e','e','e','w','w','w','w','w','w'});
    
    return 0;
}