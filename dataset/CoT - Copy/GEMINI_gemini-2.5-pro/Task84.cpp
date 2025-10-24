#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdint> // For uint32_t

class Task84 {
public:
    /**
     * Generates a 128-bit random session ID and returns it as a hex string.
     * It uses std::random_device, which is typically a non-deterministic
     * random number generator (often a hardware entropy source).
     *
     * @return A 32-character hex string representing the session ID.
     */
    static std::string generateSessionId() {
        // We need 128 bits of randomness (16 bytes)
        uint32_t random_data[4];

        // std::random_device is a source of non-deterministic random numbers
        // if a hardware entropy source is available.
        std::random_device rd;
        for (int i = 0; i < 4; ++i) {
            random_data[i] = rd();
        }

        // Convert the random bytes to a hex string
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        // Cast to unsigned char pointer to iterate through bytes
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(random_data);

        for (int i = 0; i < 16; ++i) {
            ss << std::setw(2) << static_cast<int>(bytes[i]);
        }

        return ss.str();
    }
};

int main() {
    std::cout << "--- Testing generateSessionId() ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string sessionId = Task84::generateSessionId();
        std::cout << "Test Case " << (i + 1) << ": " << sessionId << std::endl;
    }
    return 0;
}