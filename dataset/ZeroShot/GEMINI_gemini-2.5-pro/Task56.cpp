#include <iostream>
#include <string>
#include <vector>
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
 * Generates a secure random token encoded in hexadecimal.
 * This function uses OS-specific cryptographically secure random number generators:
 * - Windows: BCryptGenRandom
 * - Linux/macOS: /dev/urandom
 *
 * @param lengthInBytes The number of random bytes to generate.
 * @return A hex-encoded string representing the token, or an empty string on failure.
 */
std::string generateToken(int lengthInBytes) {
    if (lengthInBytes <= 0) {
        return "";
    }
    std::vector<unsigned char> buffer(lengthInBytes);

#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buffer.data(), buffer.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        std::cerr << "Error: BCryptGenRandom failed." << std::endl;
        return "";
    }
#else
    std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
    if (!urandom) {
        std::cerr << "Error: Cannot open /dev/urandom." << std::endl;
        return "";
    }
    urandom.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    if (!urandom) {
        std::cerr << "Error: Failed to read from /dev/urandom." << std::endl;
        urandom.close();
        return "";
    }
    urandom.close();
#endif

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned char byte : buffer) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

int main() {
    std::cout << "CPP: Generating 5 secure tokens (32 bytes of randomness each)" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string token = generateToken(32);
        if (!token.empty()) {
            std::cout << "Test Case " << (i + 1) << ": " << token << std::endl;
        }
    }
    return 0;
}