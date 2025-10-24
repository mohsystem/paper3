#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

// Note: This code requires the OpenSSL library.
// To compile, you might need to link against it, for example:
// g++ your_file.cpp -o your_program -lssl -lcrypto

/**
 * Computes the SHA-256 hash of a password combined with a salt.
 *
 * @param password The password to hash.
 * @param salt The salt to combine with the password.
 * @return The hexadecimal representation of the hash.
 */
std::string hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = salt + password;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, combined.c_str(), combined.length());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

int main() {
    // Test Case 1
    std::string pass1 = "password123";
    std::string salt1 = "randomsalt1";
    std::cout << "Password: " << pass1 << ", Salt: " << salt1 << std::endl;
    std::cout << "Hash: " << hashPassword(pass1, salt1) << std::endl << std::endl;

    // Test Case 2
    std::string pass2 = "MyP@ssw0rd";
    std::string salt2 = "salty-salty";
    std::cout << "Password: " << pass2 << ", Salt: " << salt2 << std::endl;
    std::cout << "Hash: " << hashPassword(pass2, salt2) << std::endl << std::endl;

    // Test Case 3
    std::string pass3 = "topsecret";
    std::string salt3 = "another-salt";
    std::cout << "Password: " << pass3 << ", Salt: " << salt3 << std::endl;
    std::cout << "Hash: " << hashPassword(pass3, salt3) << std::endl << std::endl;

    // Test Case 4 (Empty Password)
    std::string pass4 = "";
    std::string salt4 = "emptypass";
    std::cout << "Password: " << "''" << ", Salt: " << salt4 << std::endl;
    std::cout << "Hash: " << hashPassword(pass4, salt4) << std::endl << std::endl;

    // Test Case 5 (Empty Salt)
    std::string pass5 = "short";
    std::string salt5 = "";
    std::cout << "Password: " << pass5 << ", Salt: " << "''" << std::endl;
    std::cout << "Hash: " << hashPassword(pass5, salt5) << std::endl << std::endl;

    return 0;
}