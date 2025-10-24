// Chain-of-Through process in code generation:
// 1) Problem understanding: Determine if a walk of directions is exactly 10 steps and returns to origin.
// 2) Security requirements: Validate inputs and handle unexpected characters.
// 3) Secure coding generation: Use const refs, no raw pointers, check sizes.
// 4) Code review: Ensure complete switch coverage and safe defaults.
// 5) Secure code output: Provide final, safe code with test cases.

#include <iostream>
#include <vector>

bool isValidWalk(const std::vector<char>& walk) {
    if (walk.size() != 10) return false;
    int x = 0, y = 0;
    for (char c : walk) {
        switch (c) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            default: return false; // invalid direction
        }
    }
    return x == 0 && y == 0;
}

int main() {
    // 5 test cases
    std::vector<std::vector<char>> tests = {
        {'n','s','n','s','n','s','n','s','n','s'}, // true
        {'w','e','w','e','w'},                     // false (length != 10)
        {'n','n','n','s','n','s','n','s','n','s'}, // false (doesn't return)
        {'e','w','e','w','e','w','e','w','e','w'}, // true
        {'n','n','n','n','n','s','s','s','s','e'}  // false (net east)
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        bool result = isValidWalk(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << (result ? "true" : "false") << std::endl;
    }
    return 0;
}