#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

/**
 * Generates a cryptographically secure, random and unique session ID.
 * Uses std::random_device, which is typically a non-deterministic random number generator
 * on modern platforms (e.g., using /dev/urandom on Linux or cryptographic APIs on Windows).
 *
 * @return A random 32-character hexadecimal session ID string, or an empty string on failure.
 */
std::string generateSessionId() {
    const size_t num_bytes = 16; // 128 bits
    std::vector<unsigned char> buffer(num_bytes);

    try {
        std::random_device rd;
        // Check if the random_device is truly non-deterministic. If not, the IDs may be predictable.
        if (rd.entropy() == 0) {
            std::cerr << "Warning: std::random_device is deterministic. Session IDs may not be secure." << std::endl;
        }

        // Generate random bytes
        for (size_t i = 0; i < num_bytes; ++i) {
            // The result of rd() is an unsigned int, so we cast it to unsigned char
            buffer[i] = static_cast<unsigned char>(rd());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to generate random data using std::random_device: " << e.what() << std::endl;
        return ""; // Return empty string on failure
    }

    // Convert bytes to a hexadecimal string
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : buffer) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    
    return ss.str();
}

int main() {
    std::cout << "CPP: Generating 5 unique session IDs" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string sessionId = generateSessionId();
        if (!sessionId.empty()) {
            std::cout << "Session ID " << (i + 1) << ": " << sessionId << std::endl;
        }
    }
    return 0;
}