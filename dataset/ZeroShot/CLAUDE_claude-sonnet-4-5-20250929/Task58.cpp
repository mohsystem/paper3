
#include <iostream>
#include <string>
#include <cctype>

bool XO(const std::string& str) {
    int countX = 0;
    int countO = 0;
    
    for (char ch : str) {
        char lowerCh = std::tolower(ch);
        if (lowerCh == 'x') {
            countX++;
        } else if (lowerCh == 'o') {
            countO++;
        }
    }
    
    return countX == countO;
}

int main() {
    std::cout << "Test 1: XO(\\"ooxx\\") = " << (XO("ooxx") ? "true" : "false") << " (expected: true)" << std::endl;
    std::cout << "Test 2: XO(\\"xooxx\\") = " << (XO("xooxx") ? "true" : "false") << " (expected: false)" << std::endl;
    std::cout << "Test 3: XO(\\"ooxXm\\") = " << (XO("ooxXm") ? "true" : "false") << " (expected: true)" << std::endl;
    std::cout << "Test 4: XO(\\"zpzpzpp\\") = " << (XO("zpzpzpp") ? "true" : "false") << " (expected: true)" << std::endl;
    std::cout << "Test 5: XO(\\"zzoo\\") = " << (XO("zzoo") ? "true" : "false") << " (expected: false)" << std::endl;
    
    return 0;
}
