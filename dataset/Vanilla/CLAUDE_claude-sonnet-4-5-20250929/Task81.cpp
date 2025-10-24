
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

class Task81 {
public:
    static std::string base64_decode(const std::string& input) {
        BIO* bio = BIO_new_mem_buf(input.data(), input.length());
        BIO* b64 = BIO_new(BIO_f_base64());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        
        std::string output;
        char buffer[1024];
        int decoded_length;
        
        while ((decoded_length = BIO_read(bio, buffer, sizeof(buffer))) > 0) {
            output.append(buffer, decoded_length);
        }
        
        BIO_free_all(bio);
        return output;
    }
    
    static std::string bytes_to_hex(const unsigned char* data, size_t len) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < len; i++) {
            ss << std::setw(2) << static_cast<int>(data[i]);
        }
        return ss.str();
    }
    
    static std::string normalize_hash(std::string hash) {
        hash.erase(std::remove(hash.begin(), hash.end(), ':'), hash.end());
        hash.erase(std::remove(hash.begin(), hash.end(), ' '), hash.end());
        std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
        return hash;
    }
    
    static bool checkCertificateHash(const std::string& certificate_pem, 
                                     const std::string& known_hash, 
                                     const std::string& algorithm) {
        try {
            // Remove PEM headers
            std::string cert_content = certificate_pem;
            size_t start = cert_content.find("-----BEGIN CERTIFICATE-----");
            size_t end = cert_content.find("-----END CERTIFICATE-----");
            
            if (start == std::string::npos || end == std::string::npos) {
                return false;
            }
            
            start += 27; // Length of "-----BEGIN CERTIFICATE-----"
            cert_content = cert_content.substr(start, end - start);
            
            // Remove whitespace
            cert_content.erase(std::remove_if(cert_content.begin(), cert_content.end(), ::isspace), 
                              cert_content.end());
            
            // Decode base64
            std::string cert_bytes = base64_decode(cert_content);
            
            // Calculate hash
            std::string calculated_hash;
            
            if (algorithm == "SHA-256" || algorithm == "sha256") {
                unsigned char hash[SHA256_DIGEST_LENGTH];
                SHA256(reinterpret_cast<const unsigned char*>(cert_bytes.c_str()), 
                       cert_bytes.length(), hash);
                calculated_hash = bytes_to_hex(hash, SHA256_DIGEST_LENGTH);
            } else if (algorithm == "SHA-1" || algorithm == "sha1") {
                unsigned char hash[SHA_DIGEST_LENGTH];
                SHA1(reinterpret_cast<const unsigned char*>(cert_bytes.c_str()), 
                     cert_bytes.length(), hash);
                calculated_hash = bytes_to_hex(hash, SHA_DIGEST_LENGTH);
            } else if (algorithm == "MD5" || algorithm == "md5") {
                unsigned char hash[MD5_DIGEST_LENGTH];
                MD5(reinterpret_cast<const unsigned char*>(cert_bytes.c_str()), 
                    cert_bytes.length(), hash);
                calculated_hash = bytes_to_hex(hash, MD5_DIGEST_LENGTH);
            } else {
                return false;
            }
            
            // Compare hashes
            return calculated_hash == normalize_hash(known_hash);
            
        } catch (...) {
            return false;
        }
    }
};

int main() {
    std::string test_cert = 
        "-----BEGIN CERTIFICATE-----\\n"
        "MIICWzCCAcSgAwIBAgIJAL7xG3YqkqWGMA0GCSqGSIb3DQEBCwUAMEUxCzAJBgNV\\n"
        "BAYTAkFVMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRlcm5ldCBX\\n"
        "aWRnaXRzIFB0eSBMdGQwHhcNMjEwMTAxMDAwMDAwWhcNMjIwMTAxMDAwMDAwWjBF\\n"
        "MQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UECgwYSW50\\n"
        "ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKB\\n"
        "gQDHc0BcF8y6tN3jKBvXvL7VdqOYWlYwXMdGpLvXgNF5h6m6R7X9J4qH8o4F3k9Q\\n"
        "y6z8K7dF9e3pQ6mL4x2X7fG5j8K9L3m4x5N6p7qQ8R3j4K9L2m5x6N7q8R3j5K9L\\n"
        "3m5x7N8q9R4j6K0L4m6x8N0q0R5j7K1L5m7x9N1q1R6j8K2L6m8xQIDAQABo1Aw\\n"
        "TjAdBgNVHQ4EFgQU8h3j9K0L4m6x8N0q0R5j7K1L5m8wHwYDVR0jBBgwFoAU8h3j\\n"
        "9K0L4m6x8N0q0R5j7K1L5m8wDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQsFAAOB\\n"
        "gQBZ3j9K1L5m7x9N1q1R6j8K2L6m8x0N2q2R7j9K3L7m9xN3q3R8jK4L8mxN4q4R\\n"
        "9jK5L9mx0N5q5R0jK6LmxN6q6R1jK7LmxN7q7R2jK8LmxN8q8R3jK9Lmx\\n"
        "-----END CERTIFICATE-----";
    
    std::cout << "Test 1 - Valid SHA-256: " 
              << Task81::checkCertificateHash(test_cert, "a1b2c3d4e5f6", "SHA-256") << std::endl;
    
    std::cout << "Test 2 - Invalid hash: " 
              << Task81::checkCertificateHash(test_cert, "0000000000000000", "SHA-256") << std::endl;
    
    std::cout << "Test 3 - SHA-1: " 
              << Task81::checkCertificateHash(test_cert, "abcdef123456", "SHA-1") << std::endl;
    
    std::cout << "Test 4 - MD5: " 
              << Task81::checkCertificateHash(test_cert, "fedcba654321", "MD5") << std::endl;
    
    std::cout << "Test 5 - Invalid cert: " 
              << Task81::checkCertificateHash("-----BEGIN CERTIFICATE-----\\nINVALID\\n-----END CERTIFICATE-----",
                                             "a1b2c3d4e5f6", "SHA-256") << std::endl;
    
    return 0;
}
