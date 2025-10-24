
#include <iostream>
#include <string>
#include <cctype>

bool XO(const std::string& str) {
    int countX = 0;
    int countO = 0;
    
    for (size_t i = 0; i < str.length(); i++) {
        char c = std::tolower(static_cast<unsigned char>(str[i]));
        if (c == 'x') {
            countX++;
        } else if (c == 'o') {
            countO++;
        }
    }
    
    return countX == countO;
}

int main() {
    std::cout << "Test 1: XO(\\"ooxx\\") = " << (XO("ooxx") ? "true" : "false") << std::endl;
    std::cout << "Test 2: XO(\\"xooxx\\") = " << (XO("xooxx") ? "true" : "false") << std::endl;
    std::cout << "Test 3: XO(\\"ooxXm\\") = " << (XO("ooxXm") ? "true" : "false") << std::endl;
    std::cout << "Test 4: XO(\\"zpzpzpp\\") = " << (XO("zpzpzpp") ? "true" : "false") << std::endl;
    std::cout << "Test 5: XO(\\"zzoo\\") = " << (XO("zzoo") ? "true" : "false") << std::endl;
    
    return 0;
}
