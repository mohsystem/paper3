#include <iostream>
#include <string>
#include <cctype> // For std::tolower

class Task58 {
public:
    static bool XO(const std::string& str) {
        int xCount = 0;
        int oCount = 0;

        for (char c : str) {
            char lower_c = std::tolower(static_cast<unsigned char>(c));
            if (lower_c == 'x') {
                xCount++;
            } else if (lower_c == 'o') {
                oCount++;
            }
        }
        return xCount == oCount;
    }
};

int main() {
    // Test cases
    std::cout << std::boolalpha; // To print "true" or "false" instead of 1 or 0
    std::cout << "ooxx -> " << Task58::XO("ooxx") << std::endl;          // Expected: true
    std::cout << "xooxx -> " << Task58::XO("xooxx") << std::endl;        // Expected: false
    std::cout << "ooxXm -> " << Task58::XO("ooxXm") << std::endl;        // Expected: true
    std::cout << "zpzpzpp -> " << Task58::XO("zpzpzpp") << std::endl;    // Expected: true
    std::cout << "zzoo -> " << Task58::XO("zzoo") << std::endl;          // Expected: false
    return 0;
}