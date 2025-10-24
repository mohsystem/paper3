
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

class Task81 {
public:
    static std::string bytesToHex(const unsigned char* bytes, size_t len) {
        std::stringstream ss;
        for (size_t i = 0; i < len; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
        }
        return ss.str();
    }
    
    static std::string normalizeHash(const std::string& hash) {
        std::string result = hash;
        result.erase(std::remove(result.begin(), result.end(), ':'), result.end());
        result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    static bool checkCertificateHash(const std::string& certificatePEM, 
                                     const std::string& knownHash, 
                                     const std::string& algorithm) {
        BIO* bio = BIO_new_mem_buf(certificatePEM.c_str(), -1);
        if (!bio) return false;
        
        X509* cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
        BIO_free(bio);
        
        if (!cert) return false;
        
        unsigned char* der = NULL;
        int derLen = i2d_X509(cert, &der);
        
        if (derLen < 0) {
            X509_free(cert);
            return false;
        }
        
        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLen = 0;
        
        const EVP_MD* md;
        if (algorithm == "SHA-256" || algorithm == "SHA256" || algorithm == "sha256") {
            md = EVP_sha256();
        } else if (algorithm == "SHA-1" || algorithm == "SHA1" || algorithm == "sha1") {
            md = EVP_sha1();
        } else if (algorithm == "MD5" || algorithm == "md5") {
            md = EVP_md5();
        } else {
            OPENSSL_free(der);
            X509_free(cert);
            return false;
        }
        
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, md, NULL);
        EVP_DigestUpdate(ctx, der, derLen);
        EVP_DigestFinal_ex(ctx, hash, &hashLen);
        EVP_MD_CTX_free(ctx);
        
        OPENSSL_free(der);
        X509_free(cert);
        
        std::string certHash = bytesToHex(hash, hashLen);
        std::string normalizedKnownHash = normalizeHash(knownHash);
        
        return certHash == normalizedKnownHash;
    }
};

int main() {
    std::string testCert = 
        "-----BEGIN CERTIFICATE-----\\n"
        "MIICLDCCAdKgAwIBAgIBADAKBggqhkjOPQQDAjB9MQswCQYDVQQGEwJ1czELMAkG\\n"
        "A1UECAwCQ0ExCzAJBgNVBAcMAlNGMQ8wDQYDVQQKDAZKb3lFbnQxEDAOBgNVBAsM\\n"
        "B05vZGUtY2ExFDASBgNVBAMMC2V4YW1wbGUuY29tMRswGQYJKoZIhvcNAQkBFgxh\\n"
        "QGV4YW1wbGUuY29tMB4XDTIxMDEwMTAwMDAwMFoXDTMxMDEwMTAwMDAwMFowfTEL\\n"
        "MAkGA1UEBhMCdXMxCzAJBgNVBAgMAkNBMQswCQYDVQQHDAJTRjEPMA0GA1UECgwG\\n"
        "Sm95RW50MRAwDgYDVQQLDAdOb2RlLWNhMRQwEgYDVQQDDAtleGFtcGxlLmNvbTEb\\n"
        "MBkGCSqGSIb3DQEJARYMYUBleGFtcGxlLmNvbTBZMBMGByqGSM49AgEGCCqGSM49\\n"
        "AwEHA0IABEg7eqR4yMGb2N8rTOi3OKrPtGLLzqVPnzqLjLwrPmLqLnVChdPMbZHL\\n"
        "LyZuMN0wPgwGNxQqPVLfvAp9fkqFfqajUDBOMB0GA1UdDgQWBBQ7WxJ8rYxNQvJm\\n"
        "lzWVhFWrfVWnlDAfBgNVHSMEGDAWgBQ7WxJ8rYxNQvJmlzWVhFWrfVWnlDAMBgNV\\n"
        "HRMEBTADAQH/MAoGCCqGSM49BAMCA0gAMEUCIQDLWlPGzNCPKqBvbLDPuLOdRVjJ\\n"
        "kIrGqTI6+xB3L2x8hAIgCHMzH8KwHgfTLxP9gFmqFYhpUkGJvVYMqfnCMZJmLrk=\\n"
        "-----END CERTIFICATE-----";
    
    // Test Case 1
    std::string hash1 = "8f43288ad272f3103b6fb1428485ea3014c0bcf547a2f1d4e8e1e2c6e5f7d8c9";
    std::cout << "Test 1 - Valid SHA-256: " << Task81::checkCertificateHash(testCert, hash1, "SHA-256") << std::endl;
    
    // Test Case 2
    std::string hash2 = "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0";
    std::cout << "Test 2 - Valid SHA-1: " << Task81::checkCertificateHash(testCert, hash2, "SHA-1") << std::endl;
    
    // Test Case 3
    std::string hash3 = "0000000000000000000000000000000000000000000000000000000000000000";
    std::cout << "Test 3 - Invalid hash: " << Task81::checkCertificateHash(testCert, hash3, "SHA-256") << std::endl;
    
    // Test Case 4
    std::string hash4 = "8f:43:28:8a:d2:72:f3:10:3b:6f:b1:42:84:85:ea:30:14:c0:bc:f5:47:a2:f1:d4:e8:e1:e2:c6:e5:f7:d8:c9";
    std::cout << "Test 4 - Hash with colons: " << Task81::checkCertificateHash(testCert, hash4, "SHA-256") << std::endl;
    
    // Test Case 5
    std::cout << "Test 5 - Invalid cert: " << Task81::checkCertificateHash("invalid", hash1, "SHA-256") << std::endl;
    
    return 0;
}
