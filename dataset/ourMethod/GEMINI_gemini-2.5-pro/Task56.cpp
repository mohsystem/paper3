#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <vector>

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>

// Custom deleter for OpenSSL BIO types using RAII
struct BIO_deleter {
    void operator()(BIO* bio) const { BIO_free_all(bio); }
};
using unique_bio_ptr = std::unique_ptr<BIO, BIO_deleter>;

// Encodes data to a URL-safe Base64 string
std::string base64_url_encode(const std::vector<unsigned char>& data) {
    unique_bio_ptr b64(BIO_new(BIO_f_base64()));
    if (!b64) {
        throw std::runtime_error("Failed to create Base64 filter");
    }
    // No newlines in the output
    BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);

    BIO* sink = BIO_new(BIO_s_mem());
    if (!sink) {
        throw std::runtime_error("Failed to create memory BIO");
    }
    
    BIO_push(b64.get(), sink);

    if (BIO_write(b64.get(), data.data(), data.size()) <= 0) {
        // Errors are pushed to the OpenSSL error stack and can be checked with ERR_get_error()
        // For this example, a generic error is sufficient.
        throw std::runtime_error("Failed to write to Base64 BIO");
    }
    BIO_flush(b64.get());

    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64.get(), &bptr);
    if (!bptr || !bptr->data || bptr->length == 0) {
        throw std::runtime_error("Failed to get Base64 output");
    }

    std::string encoded(bptr->data, bptr->length);
    
    // Make it URL-safe
    std::replace(encoded.begin(), encoded.end(), '+', '-');
    std::replace(encoded.begin(), encoded.end(), '/', '_');
    // Remove padding
    size_t pad_pos = encoded.find('=');
    if (pad_pos != std::string::npos) {
        encoded.erase(pad_pos);
    }

    return encoded;
}

// Generates an HMAC-SHA256 signature
std::vector<unsigned char> generate_hmac_sha256(const std::string& data, const std::string& key) {
    std::vector<unsigned char> signature(EVP_MAX_MD_SIZE);
    unsigned int signature_len = 0;

    if (HMAC(EVP_sha256(),
             key.c_str(),
             static_cast<int>(key.length()),
             reinterpret_cast<const unsigned char*>(data.c_str()),
             data.length(),
             signature.data(),
             &signature_len) == nullptr) {
        throw std::runtime_error("HMAC generation failed");
    }

    signature.resize(signature_len);
    return signature;
}

/**
 * @brief Generates a secure authentication token.
 *
 * The token format is: base64url(payload).base64url(signature)
 * Payload is: "<user_id>:<expiration_timestamp>"
 * Signature is: HMAC-SHA256(payload, secret_key)
 *
 * @param user_id The user identifier to include in the token.
 * @param secret_key The secret key for signing. MUST be kept confidential.
 * @param validity_seconds The duration for which the token is valid.
 * @return A secure authentication token as a string.
 * @throws std::runtime_error on failure, std::invalid_argument on bad input.
 */
std::string generate_token(const std::string& user_id, const std::string& secret_key, int validity_seconds) {
    if (user_id.empty() || secret_key.empty() || validity_seconds <= 0) {
        throw std::invalid_argument("Invalid arguments for token generation.");
    }

    // 1. Create payload
    auto now = std::chrono::system_clock::now();
    auto expires_at = now + std::chrono::seconds(validity_seconds);
    long long expiration_timestamp = std::chrono::duration_cast<std::chrono::seconds>(expires_at.time_since_epoch()).count();

    std::string payload = user_id + ":" + std::to_string(expiration_timestamp);

    // 2. Base64-URL-encode payload
    std::vector<unsigned char> payload_vec(payload.begin(), payload.end());
    std::string encoded_payload = base64_url_encode(payload_vec);

    // 3. Generate HMAC-SHA256 signature of the *unencoded* payload
    std::vector<unsigned char> signature = generate_hmac_sha256(payload, secret_key);
    
    // 4. Base64-URL-encode signature
    std::string encoded_signature = base64_url_encode(signature);

    // 5. Combine into final token
    return encoded_payload + "." + encoded_signature;
}

int main() {
    // WARNING: Do not hard-code secret keys in production code.
    // Load them from a secure source like environment variables or a secret management system.
    const std::string secret_key = "a_very_long_and_secure_secret_key_that_is_at_least_32_bytes";

    std::cout << "Generating Tokens with Secret Key: [REDACTED]" << std::endl;

    const char* test_users[] = {"user123", "admin", "guest-user", "another_user", "test@example.com"};
    const int test_validities[] = {3600, 86400, 60, 300, 1800};

    for (int i = 0; i < 5; ++i) {
        try {
            std::string user_id = test_users[i];
            int validity = test_validities[i];
            std::string token = generate_token(user_id, secret_key, validity);
            std::cout << "Test Case " << i + 1 << ":" << std::endl;
            std::cout << "  User ID: " << user_id << std::endl;
            std::cout << "  Validity: " << validity << "s" << std::endl;
            std::cout << "  Token: " << token << std::endl << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error generating token for user " << test_users[i] << ": " << e.what() << std::endl;
        }
    }

    return 0;
}