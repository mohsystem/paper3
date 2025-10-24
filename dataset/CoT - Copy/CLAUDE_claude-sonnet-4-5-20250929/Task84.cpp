
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>

class Task84 {
public:
    static std::string generateSessionID() {
        // Use random_device for cryptographically secure random numbers
        std::random_device rd;
        
        // Generate 32 bytes (256 bits) of random data
        const size_t numBytes = 32;
        unsigned char randomBytes[numBytes];
        
        for (size_t i = 0; i < numBytes; ++i) {
            randomBytes[i] = static_cast<unsigned char>(rd());
        }
        
        // Convert to hexadecimal string
        std::ostringstream oss;
        for (size_t i = 0; i < numBytes; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0') 
                << static_cast<int>(randomBytes[i]);
        }
        
        return oss.str();
    }
};

int main() {
    std::cout << "Generating 5 unique session IDs:\\n" << std::endl;
    
    for (int i = 1; i <= 5; ++i) {
        std::string sessionId = Task84::generateSessionID();
        std::cout << "Session ID " << i << ": " << sessionId << std::endl;
        std::cout << "Length: " << sessionId.length() << " characters\\n" << std::endl;
    }
    
    return 0;
}
