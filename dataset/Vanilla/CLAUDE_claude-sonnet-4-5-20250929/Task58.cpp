
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

bool XO(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    
    int xCount = 0;
    int oCount = 0;
    
    for (char c : str) {
        if (c == 'x') {
            xCount++;
        } else if (c == 'o') {
            oCount++;
        }
    }
    
    return xCount == oCount;
}

int main() {
    // Test case 1
    std::cout << "XO(\\"ooxx\\") = " << (XO("ooxx") ? "true" : "false") << std::endl;
    
    // Test case 2
    std::cout << "XO(\\"xooxx\\") = " << (XO("xooxx") ? "true" : "false") << std::endl;
    
    // Test case 3
    std::cout << "XO(\\"ooxXm\\") = " << (XO("ooxXm") ? "true" : "false") << std::endl;
    
    // Test case 4
    std::cout << "XO(\\"zpzpzpp\\") = " << (XO("zpzpzpp") ? "true" : "false") << std::endl;
    
    // Test case 5
    std::cout << "XO(\\"zzoo\\") = " << (XO("zzoo") ? "true" : "false") << std::endl;
    
    return 0;
}
