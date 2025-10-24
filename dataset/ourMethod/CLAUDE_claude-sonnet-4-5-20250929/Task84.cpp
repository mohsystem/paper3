
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <openssl/rand.h>
#include <openssl/evp.h>

// Generate a cryptographically secure random session ID
// Uses OpenSSL's RAND_bytes for CSPRNG (CWE-338 mitigation)\n// Returns a hex-encoded string of 32 random bytes (64 hex characters)\nstd::string generateSessionID() {\n    const size_t SESSION_ID_BYTES = 32; // 256 bits of entropy\n    std::vector<unsigned char> randomBytes(SESSION_ID_BYTES);\n    \n    // Use OpenSSL's cryptographically secure random number generator
    // This prevents predictable session IDs (CWE-330, CWE-338)
    if (RAND_bytes(randomBytes.data(), SESSION_ID_BYTES) != 1) {
        throw std::runtime_error("Failed to generate secure random bytes");
    }
    
    // Convert to hex string for URL-safe representation
    std::ostringstream oss;
    for (unsigned char byte : randomBytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    
    return oss.str();
}

int main() {
    try {
        std::cout << "Generating 5 unique session IDs:\\n" << std::endl;
        
        // Test case 1: Generate first session ID
        std::string sessionID1 = generateSessionID();
        std::cout << "Session ID 1: " << sessionID1 << std::endl;
        
        // Test case 2: Generate second session ID
        std::string sessionID2 = generateSessionID();
        std::cout << "Session ID 2: " << sessionID2 << std::endl;
        
        // Test case 3: Generate third session ID
        std::string sessionID3 = generateSessionID();
        std::cout << "Session ID 3: " << sessionID3 << std::endl;
        
        // Test case 4: Generate fourth session ID
        std::string sessionID4 = generateSessionID();
        std::cout << "Session ID 4: " << sessionID4 << std::endl;
        
        // Test case 5: Generate fifth session ID
        std::string sessionID5 = generateSessionID();
        std::cout << "Session ID 5: " << sessionID5 << std::endl;
        
        // Verify uniqueness
        std::cout << "\\nVerifying uniqueness:" << std::endl;
        std::cout << "ID1 != ID2: " << (sessionID1 != sessionID2 ? "PASS" : "FAIL") << std::endl;
        std::cout << "ID2 != ID3: " << (sessionID2 != sessionID3 ? "PASS" : "FAIL") << std::endl;
        std::cout << "ID3 != ID4: " << (sessionID3 != sessionID4 ? "PASS" : "FAIL") << std::endl;
        std::cout << "ID4 != ID5: " << (sessionID4 != sessionID5 ? "PASS" : "FAIL") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
