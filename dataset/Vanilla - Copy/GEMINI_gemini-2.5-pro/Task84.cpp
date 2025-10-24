#include <iostream>
#include <string>
#include <random>
#include <sstream>

/**
 * Generates a random and unique session ID as a hexadecimal string.
 * @return A 32-character hexadecimal string.
 */
std::string generateSessionId() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;

    std::stringstream ss;
    ss << std::hex << dis(gen);
    ss << std::hex << dis(gen);
    
    // Pad with leading zeros if necessary to ensure a consistent length
    std::string result = ss.str();
    result.insert(0, 32 - result.length(), '0');
    return result.substr(0, 32);
}

int main() {
    std::cout << "CPP Session IDs:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ": " << generateSessionId() << std::endl;
    }
    return 0;
}