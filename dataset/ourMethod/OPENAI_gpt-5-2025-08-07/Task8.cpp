#include <iostream>
#include <vector>

char find_missing_letter(const std::vector<char>& arr) {
    if (arr.size() < 2) return '\0';
    for (size_t i = 1; i < arr.size(); ++i) {
        int diff = static_cast<int>(arr[i]) - static_cast<int>(arr[i - 1]);
        if (diff <= 0) return '\0'; // not strictly increasing -> invalid
        if (diff > 1) {
            char candidate = static_cast<char>(arr[i - 1] + 1);
            if ((candidate >= 'a' && candidate <= 'z') || (candidate >= 'A' && candidate <= 'Z')) {
                return candidate;
            }
            return '\0';
        }
    }
    return '\0'; // should not happen if exactly one letter is missing
}

int main() {
    // Test case 1
    std::vector<char> t1 = {'a','b','c','d','f'};
    std::cout << "Test 1 expected e, got " << (find_missing_letter(t1) ? find_missing_letter(t1) : '?') << "\n";

    // Test case 2
    std::vector<char> t2 = {'O','Q','R','S'};
    std::cout << "Test 2 expected P, got " << (find_missing_letter(t2) ? find_missing_letter(t2) : '?') << "\n";

    // Test case 3
    std::vector<char> t3 = {'x','z'};
    std::cout << "Test 3 expected y, got " << (find_missing_letter(t3) ? find_missing_letter(t3) : '?') << "\n";

    // Test case 4
    std::vector<char> t4 = {'b','c','d','e','f','g','i'};
    std::cout << "Test 4 expected h, got " << (find_missing_letter(t4) ? find_missing_letter(t4) : '?') << "\n";

    // Test case 5
    std::vector<char> t5 = {'M','N','P'};
    std::cout << "Test 5 expected O, got " << (find_missing_letter(t5) ? find_missing_letter(t5) : '?') << "\n";

    return 0;
}