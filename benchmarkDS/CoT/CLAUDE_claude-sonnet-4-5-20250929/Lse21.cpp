
#include <iostream>
#include <string>
#include <climits>
#include <cerrno>
#include <cstdlib>

class Lse21 {
public:
    static int processValue(const std::string& input) {
        // Validate input is not empty
        if (input.empty()) {
            std::cerr << "Error: Input cannot be empty" << std::endl;
            return -1;
        }
        
        try {
            // Check for valid numeric format
            size_t pos;
            long long value = std::stoll(input, &pos);
            
            // Verify entire string was converted
            if (pos != input.length()) {
                std::cerr << "Error: Invalid number format" << std::endl;
                return -1;
            }
            
            // Check for overflow
            if (value > LLONG_MAX - 1000) {
                std::cerr << "Error: Integer overflow detected" << std::endl;
                return -1;
            }
            
            if (value < LLONG_MIN + 1000) {
                std::cerr << "Error: Integer underflow detected" << std::endl;
                return -1;
            }
            
            long long result = value + 1000;
            std::cout << result << std::endl;
            return 0;
            
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid number format" << std::endl;
            return -1;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Number out of range" << std::endl;
            return -1;
        }
    }
};

int main(int argc, char* argv[]) {
    // Test cases
    std::string testCases[] = {"100", "0", "-500", "999999999", "abc"};
    
    std::cout << "Running test cases:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "\\nTest case " << (i + 1) << ": input = " << testCases[i] << std::endl;
        Lse21::processValue(testCases[i]);
    }
    
    // Process command line argument if provided
    if (argc > 1) {
        std::cout << "\\nProcessing command line argument:" << std::endl;
        return Lse21::processValue(argv[1]);
    }
    
    return 0;
}
