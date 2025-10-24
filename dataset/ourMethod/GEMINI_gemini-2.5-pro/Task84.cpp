#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>

// For OpenSSL, ensure you have the development libraries installed.
// On Debian/Ubuntu: sudo apt-get install libssl-dev
// On RedHat/CentOS: sudo yum install openssl-devel
// Compile with: g++ this_file.cpp -o your_app -lssl -lcrypto
#include <openssl/rand.h>
#include <openssl/err.h>

// 32 bytes (256 bits) of entropy is a strong choice for session IDs.
const int SESSION_ID_BYTES = 32;

/**
 * Generates a cryptographically secure, random session ID using OpenSSL.
 * The ID is returned as a hex-encoded string.
 *
 * @return A string containing the hex-encoded session ID.
 * @throws std::runtime_error if the random byte generation fails.
 */
std::string generateSessionId() {
    std::vector<unsigned char> buffer(SESSION_ID_BYTES);

    if (RAND_bytes(buffer.data(), buffer.size()) != 1) {
        // OpenSSL's error queue can provide more details.
        unsigned long err_code = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err_code, err_buf, sizeof(err_buf));
        throw std::runtime_error("Failed to generate random bytes: " + std::string(err_buf));
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (const auto& byte : buffer) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

void run_test_cases() {
    std::cout << "Generating 5 random and unique session IDs:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        try {
            std::string sessionId = generateSessionId();
            std::cout << "Session ID " << (i + 1) << ": " << sessionId << std::endl;
            if (sessionId.empty() || sessionId.length() != SESSION_ID_BYTES * 2) {
                 std::cerr << "Test case " << (i + 1) << " failed: Invalid session ID generated." << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Error during test case " << (i + 1) << ": " << e.what() << std::endl;
        }
    }
}

int main() {
    // Load the error strings for libcrypto for better error reporting.
    ERR_load_crypto_strings();
    
    run_test_cases();

    // Free the error strings.
    ERR_free_strings();
    return 0;
}