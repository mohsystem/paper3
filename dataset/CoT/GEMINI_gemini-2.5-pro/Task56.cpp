#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fstream>
#endif

/**
 * Converts a vector of bytes to a hexadecimal string.
 *
 * @param bytes The vector of bytes to convert.
 * @return The resulting hexadecimal string.
 */
static std::string bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

/**
 * Generates a cryptographically secure token as a hex string.
 *
 * @param byteLength The number of random bytes to generate. 32 is recommended.
 * @return A hexadecimal string representation of the random bytes.
 */
std::string generateToken(size_t byteLength) {
    if (byteLength == 0) {
        return "";
    }
    std::vector<unsigned char> buffer(byteLength);

#ifdef _WIN32
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RNG_ALGORITHM, NULL, 0) != 0) {
        throw std::runtime_error("Failed to open algorithm provider.");
    }
    if (BCryptGenRandom(hAlg, buffer.data(), (ULONG)buffer.size(), 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg, 0);
        throw std::runtime_error("Failed to generate random data.");
    }
    BCryptCloseAlgorithmProvider(hAlg, 0);
#else
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    if (!urandom) {
        throw std::runtime_error("Failed to open /dev/urandom.");
    }
    urandom.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    if (!urandom) {
        urandom.close();
        throw std::runtime_error("Failed to read from /dev/urandom.");
    }
    urandom.close();
#endif

    return bytesToHex(buffer);
}

int main() {
    std::cout << "C++ Token Generation Test Cases:" << std::endl;
    try {
        // Test Case 1: 16 bytes (128 bits) -> 32 hex chars
        std::cout << "1. 16-byte token: " << generateToken(16) << std::endl;

        // Test Case 2: 32 bytes (256 bits) -> 64 hex chars - Recommended
        std::cout << "2. 32-byte token: " << generateToken(32) << std::endl;

        // Test Case 3: 48 bytes (384 bits) -> 96 hex chars
        std::cout << "3. 48-byte token: " << generateToken(48) << std::endl;
        
        // Test Case 4: Another 32-byte token to show it's different
        std::cout << "4. 32-byte token: " << generateToken(32) << std::endl;
        
        // Test Case 5: 64 bytes (512 bits) -> 128 hex chars
        std::cout << "5. 64-byte token: " << generateToken(64) << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}