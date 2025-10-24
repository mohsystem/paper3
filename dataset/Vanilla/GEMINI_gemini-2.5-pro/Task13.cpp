#include <iostream>
#include <vector>

bool isValidWalk(const std::vector<char>& walk) {
    if (walk.size() != 10) {
        return false;
    }

    int x = 0;
    int y = 0;

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

int main() {
    std::cout << std::boolalpha;

    std::vector<char> walk1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    std::cout << "Test 1: " << isValidWalk(walk1) << std::endl;

    std::vector<char> walk2 = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};
    std::cout << "Test 2: " << isValidWalk(walk2) << std::endl;

    std::vector<char> walk3 = {'w'};
    std::cout << "Test 3: " << isValidWalk(walk3) << std::endl;

    std::vector<char> walk4 = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    std::cout << "Test 4: " << isValidWalk(walk4) << std::endl;

    std::vector<char> walk5 = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
    std::cout << "Test 5: " << isValidWalk(walk5) << std::endl;

    return 0;
}